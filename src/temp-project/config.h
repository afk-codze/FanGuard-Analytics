#pragma once
#define WIFI_MAX_RETRIES 10
#define MSG_BUFFER_SIZE 50
#define RETRY_DELAY 1000 / portTICK_PERIOD_MS
#define MQTT_LOOP portTICK_PERIOD_MS
#define DATASTREAM_RMS_TOPIC "luca/esp32/data"
#define ANOMALY_RMS_TOPIC "luca/esp32/data"
#define PREF_SESSION_ID "session"

#define QUEUE_SIZE 10

