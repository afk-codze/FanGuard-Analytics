
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fft-and-adaptive-sampling.h"
#include <Adafruit_ADXL345_U.h>
#include <Wire.h>
#include "shared-defs.h"
#include "communication.h"

#define BUZZER_PIN 23
#define ADXL345_SDA 41
#define ADXL345_SCL 42


RTC_DATA_ATTR Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

TaskHandle_t send_anomaly_task_handler  = NULL;

void comunication_task(void *pvParameters) {
  xCommunicationTaskHandle = xTaskGetCurrentTaskHandle();

  // Start WiFi connection
  wifi_init();

  vTaskDelete(NULL);
}


void buzzer_anomaly_task(void *args){
  if(anomaly_detected){
    while(1){
      digitalWrite(BUZZER_PIN, HIGH);
    }
  }
  vTaskDelete(NULL);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(ADXL345_SDA, ADXL345_SCL);
  pinMode(BUZZER_PIN, OUTPUT);
  init_shared_queues();

  if (!accel.begin()) {
    Serial.println("ADXL345 not found");
    while (true)
      delay(10);
  } else {
    Serial.println("ADXL345 connected");
  }

  xTaskCreatePinnedToCore(comunication_task, "comunication_task", 8192, xTaskGetCurrentTaskHandle(), 2, NULL,1);
  //xTaskCreatePinnedToCore(send_anomaly_task, "send_anomaly_task", 4096, NULL, 1, &send_anomaly_task_handler,1);
  //xTaskCreatePinnedToCore(buzzer_anomaly_task, "buzzer_anomaly_task", 4096, NULL, 1, NULL,1);
  xTaskCreatePinnedToCore(fft_sampling_task, "fft_sampling_task", 8192, send_anomaly_task_handler, 1, &fft_sampling_task_handle,0);
}

void loop() {
 vTaskDelete(NULL); // Delete the task if it is not needed anymore
}