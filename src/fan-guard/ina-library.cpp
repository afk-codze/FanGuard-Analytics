#include <ina_anomalies_classification_inferencing.h>
#include "esp_attr.h"
#include "ina-library.h"
#include "shared-defs.h"
Adafruit_INA219 ina219;

// Variables for filtering
float filtered_bus_voltage = 0;   
float filtered_shunt_voltage = 0; 
float filtered_current = 0;       
float filtered_power = 0;         
float filtered_load_voltage = 0;  
float prev_bus_voltage = 0;       
float prev_current = 0;          

RTC_DATA_ATTR float baseline_mean = 0;
RTC_DATA_ATTR float baseline_std = 0;
float power_readings[BASELINE_SAMPLES];
RTC_DATA_ATTR int total_readings = 0;
bool first_reading = true; 

RTC_DATA_ATTR float max_deviation = 0;
RTC_DATA_ATTR int ina_threshold = 0;

float features[DATA_BUFFER_SIZE];
int feature_idx = 0;

void ina219_init() {
  Serial.begin(115200);
  while (!Serial) {
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  Serial.println("Based on INA219 Power Monitor");

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { vTaskDelay(pdMS_TO_TICKS(10)); }
  }

  ina219.setCalibration_16V_400mA();

  // Initialize filters with first reading
  vTaskDelay(pdMS_TO_TICKS(1000));
  filtered_bus_voltage = ina219.getBusVoltage_V();
  filtered_shunt_voltage = ina219.getShuntVoltage_mV();
  filtered_current = ina219.getCurrent_mA();
  filtered_power = ina219.getPower_mW();
  filtered_load_voltage = filtered_bus_voltage + (filtered_shunt_voltage / 1000);

  prev_bus_voltage = filtered_bus_voltage;
  prev_current = filtered_current;

  Serial.println("INA219 initialized successfully");
}

// float get_averaged_reading_ina_mpu(float (*read_function1)(),float (*read_function2)(), int samples) { 
//   float sum_ina = 0;
//   xyzFloat mpu_sample;
//   float sum_mpu_x = 0;
//   float sum_mpu_y = 0;
//   float sum_mpu_z = 0;
//   for (int i = 0; i < samples; i++) {
//     sum_ina += read_function1();
//     mpu_sample = read_function2();
//     sum_mpu_x += mpu_sample.x;
//     sum_mpu_y += mpu_sample.y;
//     sum_mpu_z += mpu_sample.z;
//     vTaskDelay(pdMS_TO_TICKS(1));
//   }
//   return {sum_ina / samples,sum_mpu_x/samples,sum_mpu_y/samples,sum_mpu_z/samples};
// }


xyzFloat get_rms_reading_mpu(MPU6500_WE& mpu_instance, int samples) { 
  float sum_x_squared = 0;
  float sum_y_squared = 0;
  float sum_z_squared = 0;
  xyzFloat temp;
  
  for (int i = 0; i < samples; i++) {
    temp = mpu_instance.getGValues();
    sum_x_squared += temp.x * temp.x;
    sum_y_squared += temp.y * temp.y;
    sum_z_squared += temp.z * temp.z;
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  
  temp.x = sqrt(sum_x_squared / samples);
  temp.y = sqrt(sum_y_squared / samples);
  temp.z = sqrt(sum_z_squared / samples);
  
  return temp;
}

float get_averaged_reading(float (*read_function)(), int samples) { 
  float sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += read_function();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  return sum / samples;
}

float get_bus_voltage() { return ina219.getBusVoltage_V(); }     
float get_shunt_voltage() { return ina219.getShuntVoltage_mV(); } 
float get_current() { return ina219.getCurrent_mA(); }          
float get_power() { return ina219.getPower_mW(); }               

float spike_filter(float new_value, float prev_value, float threshold) { 
  if (abs(new_value - prev_value) > threshold) {
    return prev_value;
  }
  return new_value;
}

float low_pass_filter(float new_value, float prev_filtered, float alpha) { 
  return alpha * new_value + (1 - alpha) * prev_filtered;
}

float read_ina_filtered() {             
  float raw_power = get_averaged_reading(get_power, SAMPLE_COUNT);                   
  if (first_reading) {
    filtered_power = raw_power;
    first_reading = false;
  } else {
    filtered_power = low_pass_filter(raw_power, filtered_power, ALPHA);                        
  }
  return filtered_power;
}

float calculate_mean(float* array, int size) { 
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
  }
  return sum / size;
}

float calculate_std_dev(float* array, int size, float mean) { 
  float sum_squared_diff = 0;
  for (int i = 0; i < size; i++) {
    float diff = array[i] - mean;
    sum_squared_diff += diff * diff;
  }
  return sqrt(sum_squared_diff / size);
}

bool detect_threshold_anomaly(float reading) {
  float deviation = abs(reading - baseline_mean);

  if (deviation > max_deviation) {
    max_deviation = deviation;
  }

  return deviation > ina_threshold;
}

void build_baseline_ina(void *args){
  for(int i=0; i < BASELINE_SAMPLES;i++){
    power_readings[i] = read_ina_filtered();
    Serial.print("Calibrating... "); Serial.print(i);
    Serial.print("/"); Serial.print(BASELINE_SAMPLES);
    Serial.print(" | Power: "); Serial.print(power_readings[i], 2); Serial.println(" mW");
    vTaskDelay(pdMS_TO_TICKS(READING_DELAY));
  }

  baseline_mean = calculate_mean(power_readings, BASELINE_SAMPLES);   
  baseline_std = calculate_std_dev(power_readings, BASELINE_SAMPLES, baseline_mean); 

  // Ensure minimum standard deviation to avoid false positives
  if (baseline_std < 1.0) {
    baseline_std = 1.0;
  }

  Serial.println("\n=== BASELINE ESTABLISHED ===");
  Serial.print("Mean Power: "); Serial.print(baseline_mean, 2); Serial.println(" mW");
  Serial.print("Std Deviation: "); Serial.print(baseline_std, 2); Serial.println(" mW");
  Serial.print("Anomaly Threshold: "); Serial.print(ANOMALY_THRESHOLD * baseline_std, 2); Serial.println(" mW");
  Serial.print("Samples Used: "); Serial.println(BASELINE_SAMPLES);
  Serial.println("Starting anomaly detection...");
  Serial.println("===========================\n");
  ina_threshold = ANOMALY_THRESHOLD * baseline_std;

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = features[offset + i];
  }
  return 0;
}

data_to_send_t classify() {
  data_to_send_t data_to_send;
  signal_t features_signal;
  features_signal.total_length = DATA_BUFFER_SIZE;
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
  data_to_send.type = TYPE_INA;
  data_to_send.time_stamp = millis();
  data_to_send.prob = max_score * 100;

  return data_to_send;
}


void ina_periodic_check(void *args){
  Serial.printf("*** ina_periodic_check ***");
  data_to_send_t data_to_send;
  unsigned long start = millis(),ts_anom_found = 0,ts=0;
  int local_anomalies = 0;
  bool ina_anomaly = false;
  for (int i =0; i<DATA_BUFFER_SIZE; i++) {
    features[feature_idx++] = read_ina_filtered(); 
    vTaskDelay(pdTICKS_TO_MS(1000/PERIODIC_CHECK_SAMPLES));
  }
  ts = millis();
  Serial.print(ts-start);

  data_to_send = classify();
  
  if(data_to_send.type == TYPE_INA && data_to_send.classification != CLASS_NORMAL){
    send_anomaly_mqtt(data_to_send);
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}