
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "fft-and-adaptive-sampling.h"
#include <MPU6500_WE.h>
#include <Wire.h>
#include "shared-defs.h"
#include "communication.h"

#define BUZZER_PIN 23
MPU6500_WE myMPU6500(MPU6500_ADDR);

// RTC variables are implicitly volatile
RTC_DATA_ATTR bool initialized = false;

// Store components separately to avoid struct copy issues
RTC_DATA_ATTR float acc_off_x = 0.0;
RTC_DATA_ATTR float acc_off_y = 0.0;
RTC_DATA_ATTR float acc_off_z = 0.0;

TaskHandle_t send_anomaly_task_handler  = NULL;

void comunication_task(void *pvParameters) {

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

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN,OUTPUT);
  if(!myMPU6500.init()){
    Serial.println("MPU6500 does not respond");
  }
  else{
    Serial.println("MPU6500 is connected");
  }
  
  // Queues initialization
  init_shared_queues();

  if(!initialized){
    Serial.println("MPU6500 - calibrating...");
    delay(1000);
    myMPU6500.autoOffsets();
    
    // Get offsets as regular xyzFloat
    xyzFloat accOffsets = myMPU6500.getAccOffsets();
    
    // Store individual components to RTC memory
    acc_off_x = accOffsets.x;
    acc_off_y = accOffsets.y;
    acc_off_z = accOffsets.z;
    
    Serial.printf("acc_off: %.2f, %.2f, %.2f\n", acc_off_x, acc_off_y, acc_off_z);
    
    initialized = true;
    Serial.println("Done!");
  } else {
    Serial.println("Using saved calibration values");
    
    // Create temporary xyzFloat objects
    xyzFloat accOffsets;
    accOffsets.x = acc_off_x;
    accOffsets.y = acc_off_y;
    accOffsets.z = acc_off_z;
    
    // Now set the offsets
    myMPU6500.setAccOffsets(accOffsets);
    
    Serial.printf("acc_off: %.2f, %.2f, %.2f\n", acc_off_x, acc_off_y, acc_off_z);
  }

  myMPU6500.setAccRange(MPU9250_ACC_RANGE_2G);
  myMPU6500.enableAccDLPF(true);
  myMPU6500.setAccDLPF(MPU9250_DLPF_6);
  
  if(this_reboot_send_rms){
    // this code will run only if we need to send a message to the mqtt server in this cycle
    // so if it's the first time setup is executed or if (last sample % window_size) == 0
    xTaskCreatePinnedToCore(comunication_task, "comunication_task", 4096, NULL, 1, NULL,1);
    this_reboot_send_rms = false;
  }
  xTaskCreatePinnedToCore(send_anomaly_task, "send_anomaly_task", 4096, NULL, 1, &send_anomaly_task_handler,1);
  //xTaskCreatePinnedToCore(buzzer_anomaly_task, "buzzer_anomaly_task", 4096, NULL, 1, NULL,1);
  xTaskCreatePinnedToCore(fft_sampling_task, "fft_sampling_task", 4096, send_anomaly_task_handler, 1, &fft_sampling_task_handle,0);
}

void loop() {
 vTaskDelete(NULL); // Delete the task if it is not needed anymore
}
