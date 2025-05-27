#include "shared-defs.h"
#include "ina-library.h"
#include "MPU6500-library.h"
#include "sampling.h"

/*

Import libraries from ../fan-guard

*/

float ina_samples[NUM_SAMPLES];
xyzFloat motion_samples[NUM_SAMPLES];

void task_avg_acc(void *args){
  for(;;){
    xyzFloat temp = get_rms_reading_mpu(my_mpu6500,NUM_AVG_ACC);
    Serial.printf("%.2f,%.2f,%.2f",temp.x,temp.z,temp.y);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}

void task_flt_ina(void *args){
  for(;;){
    Serial.printf(",%.2f\n",read_ina_filtered());
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  xTaskNotifyGive((TaskHandle_t)args);
  vTaskDelete(NULL);
}


void setup() {
  Serial.begin(115200);
  Wire.begin(41,42);
  // Set up interrupt pin
  pinMode(INT_PIN, INPUT_PULLUP); // Set up interrupt pin with pull-up  
  ina219_init();
  acc_init();

}

void loop() {
  
  xTaskCreate(task_avg_acc, "task_avg_acc", 4096, xTaskGetCurrentTaskHandle(), 1, NULL);
  xTaskCreate(task_flt_ina, "task_flt_ina", 4096, xTaskGetCurrentTaskHandle(), 1, NULL);
  
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}
