#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H
#include <Wire.h>
#include <MPU6500_WE.h>
#include <config.h>
#define MPU6500_ADDR 0x68

struct DATA_TO_SEND{
  unsigned long time_stamp = 0;
  float *rms_array = {0};
  volatile bool anomaly = false;
};

typedef DATA_TO_SEND data_to_send_t;

extern TaskHandle_t fft_sampling_task_handle;
extern TaskHandle_t main_task_handle;
extern TaskHandle_t communication_mqtt_task_handle;
extern TaskHandle_t wifi_task_handle;
extern volatile bool sending_window;
extern QueueHandle_t xQueue_data;
extern bool anomaly_detected;
extern int id_device;
extern bool anomaly_sent;
extern bool this_reboot_send_rms;
extern data_to_send_t anomaly;
extern MPU6500_WE myMPU6500;
extern bool g_is_wakeup_from_deep_sleep;

void init_shared_queues();
#endif