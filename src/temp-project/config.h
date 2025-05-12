#pragma once
#define WIFI_MAX_RETRIES 10
#define MSG_BUFFER_SIZE 50
#define RETRY_DELAY 2000 / portTICK_PERIOD_MS
#define MQTT_LOOP portTICK_PERIOD_MS
#define PUBLISH_TOPIC "luca/esp32/data"
#define SUBSCRIBE_TOPIC "luca/esp32/acks"

#define QUEUE_SIZE 10

