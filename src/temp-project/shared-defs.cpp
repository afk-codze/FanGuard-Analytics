#include "esp_attr.h"
#include <Arduino.h>
#include "config.h"
#include "shared-defs.h"

QueueHandle_t xQueue_rms = NULL;
TaskHandle_t xCommunicationTaskHandle = NULL;
TaskHandle_t fft_sampling_task_handle = NULL;

void init_shared_queues() {
    xQueue_rms = xQueueCreate(QUEUE_SIZE, sizeof(float));
    if(xQueue_rms ==  NULL) {
        Serial.println("Queue creation failed!");
        while(1); // Halt on critical failure
    }else{
      Serial.println("[COMMUNICATION] shared queue succesfully created!");
    }
    
}