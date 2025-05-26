#include <Arduino.h>
#include "shared-defs.h"

volatile bool sending_window = false;
QueueHandle_t xQueue_data = NULL;
TaskHandle_t fft_sampling_task_handle = NULL;
TaskHandle_t communication_mqtt_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;
TaskHandle_t wifi_task_handle = NULL;
bool this_reboot_send_rms = true;
bool g_is_wakeup_from_deep_sleep = false;
int id_device = 0;
void init_shared_queues() {
    xQueue_data = xQueueCreate(QUEUE_SIZE, sizeof(data_to_send_t));
    if(xQueue_data ==  NULL) {
        Serial.println("Queue creation failed!");
        while(1); // Halt on critical failure
    }
    
}