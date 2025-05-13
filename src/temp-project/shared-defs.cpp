#include "esp_attr.h"
#include <Arduino.h>
#include "config.h"
#include "shared-defs.h"

QueueHandle_t xQueue_data = NULL;
TaskHandle_t fft_sampling_task_handle = NULL;
RTC_DATA_ATTR bool this_reboot_send_rms = true;
RTC_DATA_ATTR bool g_is_wakeup_from_deep_sleep = false;
RTC_DATA_ATTR int id_device = 0;
void init_shared_queues() {
    xQueue_data = xQueueCreate(QUEUE_SIZE, sizeof(data_to_send_t));
    if(xQueue_data ==  NULL) {
        Serial.println("Queue creation failed!");
        while(1); // Halt on critical failure
    }
    
}