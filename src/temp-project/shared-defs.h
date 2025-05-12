#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H
#include <Wire.h>
#include <MPU6500_WE.h>
#include <config.h>
#define MPU6500_ADDR 0x68

struct ANOMALY{
  unsigned long time_stamp;
  float *rms_array;
};

typedef ANOMALY anomaly_data_t;
extern TaskHandle_t xCommunicationTaskHandle;
extern TaskHandle_t fft_sampling_task_handle;
extern QueueHandle_t xQueue_rms;
extern RTC_DATA_ATTR float rms_mem[QUEUE_SIZE];
extern RTC_DATA_ATTR int rms_mem_size;
extern RTC_DATA_ATTR bool anomaly_detected;
extern RTC_DATA_ATTR bool anomaly_sent;
extern RTC_DATA_ATTR bool wifi_established;
extern RTC_DATA_ATTR bool mqtt_established;
extern RTC_DATA_ATTR anomaly_data_t anomaly;
extern RTC_DATA_ATTR MPU6500_WE myMPU6500;
void init_shared_queues();
#endif