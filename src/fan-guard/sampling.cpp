#include "projdefs.h"
#include "sampling.h"


xyzFloat motion_samples[NUM_SAMPLES];
float ina_samples[NUM_SAMPLES];

float features_mpu[DATA_BUFFER_SIZE * 4];
int feature_idx_mpu = 0;

int raw_feature_get_data_mpu(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = features_mpu[offset + i];
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
    motion_samples[i] = get_averaged_reading_mpu(my_mpu6500,NUM_AVG_ACC);
    Serial.printf("x_avg:%.2f , y_avg:%.2f , z_avg:%.2f \n",motion_samples[i].x,motion_samples[i].y,motion_samples[i].z);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}

void task_flt_ina(void *args){
  for(int i =0; i<DATA_BUFFER_SIZE;i++){
    ina_samples[i] = read_ina_filtered();
    Serial.printf("pw:%.2f \n",ina_samples[i]);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}


/// f[i] = avg_x,f[i+1] = avg_y,f[i+2] = avg_z,f[i+3] = power -> BUFFER_SIZE = 50 * 4


// void classify() {
//   signal_t features_signal;
//   features_signal.total_length = DATA_BUFFER_SIZE;
//   features_signal.get_data = &raw_feature_get_data;

//   ei_impulse_result_t result = {0};

//   EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
//   if (res != EI_IMPULSE_OK) {
//     ei_printf("ERR: Failed to run impulse (%d)\n", res);
//     return;
//   }
  
//   float max_score = 0;
//   const char* predicted_label = "uncertain";
//   for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
//     ei_printf("  %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
//     if (result.classification[ix].value > max_score) {
//         max_score = result.classification[ix].value;
//         predicted_label = result.classification[ix].label;
//     }
//   }

//   ei_printf("Detected: %s (%.2f confidence)\n", predicted_label, max_score * 100);
// }

void high_freq_sampling(void *args){

  Serial.printf("--- SAMPLING ---\n");

  xTaskCreate(task_avg_acc, "task_avg_acc", 4096, xTaskGetCurrentTaskHandle(), 1, NULL);
  xTaskCreate(task_flt_ina, "task_flt_ina", 4096, xTaskGetCurrentTaskHandle(), 1, NULL);
  
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  //classify();
  
  Serial.print("\nSampling finished\n");

  // if(anomaly_class)
  //   send_anomaly_mqtt(anomaly_class,motion_samples,ina_samples);

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}
