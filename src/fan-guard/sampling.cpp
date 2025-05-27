#include <ina_mpu_classification_inferencing.h>

#include "projdefs.h"
#include "sampling.h"


xyzFloat motion_samples[NUM_SAMPLES] = {{0,0,0}};
float ina_samples[NUM_SAMPLES] = {0};

float features[DATA_BUFFER_SIZE * 4] = {0};
int feature_idx = 0;

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = features[offset + i];
  }
  return 0;
}

bool classify_anomaly(xyzFloat *motion_samples,float *ina_samples){
  Serial.printf("\n classify anomaly\n");
  return true;
}

void send_anomaly_mqtt(bool anomaly_class,xyzFloat *motion_samples,float *ina_samples){
  Serial.printf("*** Anomaly mpu sent ***");
}

void task_avg_acc(void *args){
  for(int i =0; i<DATA_BUFFER_SIZE;i++){
    motion_samples[i] = get_rms_reading_mpu(my_mpu6500,NUM_AVG_ACC);
    //Serial.printf("x_rms:%.2f , y_rms:%.2f , z_rms:%.2f \n",motion_samples[i].x,motion_samples[i].z,motion_samples[i].y);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}

void task_flt_ina(void *args){
  for(int i =0; i<DATA_BUFFER_SIZE;i++){
    ina_samples[i] = read_ina_filtered();
    //Serial.printf("pw:%.2f \n",read_ina_filtered());
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}

void fill_features(){
  for(int i = 0 ; i< DATA_BUFFER_SIZE;i++){
    features[feature_idx++] = motion_samples[i].x;
    features[feature_idx++] = motion_samples[i].y;
    features[feature_idx++] = motion_samples[i].z;
    features[feature_idx++] = ina_samples[i];
  }
}
/// f[i] = x,f[i+1] = y,f[i+2] = z,f[i+3] = power -> DATA_BUFFER_SIZE = 50 * 4



data_to_send_t classify() {
  data_to_send_t data_to_send;
  signal_t features_signal;
  features_signal.total_length = DATA_BUFFER_SIZE*4;
  features_signal.get_data = &raw_feature_get_data;

  ei_impulse_result_t result = {0};

  EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    ei_printf("ERR: Failed to run impulse (%d)\n", res);
    return data_to_send;
  }
  
  float max_score = 0;
  const char* predicted_label = "uncertain";
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("  %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    if (result.classification[ix].value > max_score) {
        max_score = result.classification[ix].value;
        predicted_label = result.classification[ix].label;
    }
  }

  ei_printf("Detected: %s (%.2f confidence)\n", predicted_label, max_score * 100);
  
  data_to_send.classification = stringToDataClassification(predicted_label);
  data_to_send.time_stamp = millis();
  data_to_send.prob = max_score * 100;

  return data_to_send;
}


void high_freq_sampling(void *args){

  Serial.printf("--- SAMPLING ---\n");
  data_to_send_t data_to_send;
  xTaskCreatePinnedToCore(task_avg_acc, "task_avg_acc", 4096, xTaskGetCurrentTaskHandle(), 1, NULL,0);
  xTaskCreatePinnedToCore(task_flt_ina, "task_flt_ina", 4096, xTaskGetCurrentTaskHandle(), 1, NULL,1);
  
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  fill_features();
  data_to_send = classify();
  if(data_to_send.classification != CLASS_NORMAL)
    send_anomaly_mqtt(data_to_send);
  Serial.print("\nSampling finished\n");
  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}
