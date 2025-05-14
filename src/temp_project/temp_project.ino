
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fft-and-adaptive-sampling.h"
#include <MPU6500_WE.h>
#include <Wire.h>
#include "shared-defs.h"
#include "communication.h"
#include "hmac.h"
#include "driver/adc.h"
#include "esp_bt.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "esp_pm.h"

#define BUZZER_PIN 23
//MPU6500_WE myMPU6500(MPU6500_ADDR);
const int csPin = 5;  // Chip Select Pin
// const int mosiPin = 22;  // "MOSI" Pin
// const int misoPin = 21;  // "MISO" Pin
// const int sckPin = 16;  // SCK Pin
bool useSPI = true;    // SPI use flag


/* There are two constructors for SPI: */
MPU6500_WE myMPU6500 = MPU6500_WE(&SPI, csPin, useSPI);

TaskHandle_t send_anomaly_task_handler  = NULL;

void buzzer_anomaly_task(void *args){
  if(anomaly_detected){
    while(1){
      digitalWrite(BUZZER_PIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(100));
      digitalWrite(BUZZER_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(900));
    }
  }
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN,OUTPUT);
  main_task_handle = xTaskGetCurrentTaskHandle();
  if(!myMPU6500.init()){
    Serial.println("MPU6500 does not respond");
  }
  else{
    Serial.println("MPU6500 is connected");
  }

  hmac_auth_init();

  // Queues initialization
  init_shared_queues();

  // Calc id device
  for (int i = 0; i < 17; i = i + 8) {
    id_device |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.printf("Device ID: %llX\n", id_device); // Print device ID
  Serial.println("MPU6500 - calibrating...");
  delay(1000);
  myMPU6500.autoOffsets();

  Serial.println("Done!");

  myMPU6500.setAccRange(MPU9250_ACC_RANGE_2G);
  myMPU6500.enableAccDLPF(true);
  myMPU6500.setAccDLPF(MPU9250_DLPF_6);
  
  //xTaskCreatePinnedToCore(send_anomaly_task, "send_anomaly_task", 4096, NULL, 1, &send_anomaly_task_handler,1);
  //xTaskCreate(comunication_task, "comunication_task", 4096, NULL, 1, &wifi_task_handle);
  //ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100000));
  xTaskCreate(fft_sampling_task, "fft_sampling_task", 8192, send_anomaly_task_handler, 2, &fft_sampling_task_handle);
  //xTaskCreatePinnedToCore(buzzer_anomaly_task, "buzzer_anomaly_task", 4096, NULL, 1, NULL,1);
}

void loop() {
 vTaskDelete(NULL); // Delete the task if it is not needed anymore
}
