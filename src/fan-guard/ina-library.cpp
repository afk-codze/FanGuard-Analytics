#include "esp_attr.h"
#include "ina-library.h"
#include "shared-defs.h"
#include "sampling.h"

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

void ina219_init() {
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { vTaskDelay(pdMS_TO_TICKS(10)); }
  }else{
    Serial.println("ina219 is connected");
  }

  ina219.setCalibration_32V_1A();
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
    vTaskDelay(pdMS_TO_TICKS(1000/g_sampling_frequency));

  }
  
  temp.x = sqrt(sum_x_squared / samples);
  temp.y = sqrt(sum_y_squared / samples);
  temp.z = sqrt(sum_z_squared / samples);
  
  return temp;
}

// float get_averaged_reading(float (*read_function)(), int samples) { 
//   float sum = 0;
//   for (int i = 0; i < samples; i++) {
//     sum += read_function();
//     vTaskDelay(pdMS_TO_TICKS(1000/g_sampling_frequency));
//   }
//   return sum / samples;
// }

float get_averaged_reading(float (*read_function)(), int samples) { 
  float sum = 0;
  float prev_good_reading = 0;
  
  for (int i = 0; i < samples; i++) {
    float reading = read_function();
    
    if (reading < -1) {
      reading = prev_good_reading;
    } else{
      prev_good_reading = reading;  // Store as reference for next iteration
    }
    
    sum += reading;
    vTaskDelay(pdMS_TO_TICKS(1000/g_sampling_frequency));
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
  
  // Filter out clearly erroneous negative values
  if (raw_power < -1.0) {  // Adjust threshold as needed
    raw_power = filtered_power;  // Use previous good value
  }

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
