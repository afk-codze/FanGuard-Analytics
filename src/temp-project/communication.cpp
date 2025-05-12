#include "esp_attr.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>
#include "communication.h"
#include <PubSubClient.h>
#include "shared-defs.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include "config.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
// Network Configuration
#define MSG_BUFFER_SIZE 50  // Maximum size for MQTT messages

/* Global Variables --------------------------------------------------------- */
float start_time = 0.0;      // Timestamp when communication starts (ms)
float finish_time = 0.0;     // Timestamp when communication ends (ms)

WiFiClient espClient;        // WiFi client instance
PubSubClient client(espClient);  // MQTT client instance

char msg[MSG_BUFFER_SIZE];   // Buffer for MQTT messages

RTC_DATA_ATTR bool mqtt_established = false;

/* WiFi Management ---------------------------------------------------------- */
/**
 * @brief Initializes and manages WiFi connection
 * @note Implements retry logic with status monitoring
 */
void wifi_init(){
  Serial.printf("\n[WiFi] Connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA); // Set station mode
  
  // Disable power saving for faster connections (uses more power)
  esp_wifi_set_ps(WIFI_PS_NONE);
  
  // Configure static IP (much faster than DHCP)
  //WiFi.config(staticIP, gateway, subnet, dns);
  
  // For faster reconnections, set these parameters:
  // Lower min scan time - don't spend too long scanning
  WiFi.scanNetworks(true, false, false, 150);


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int numberOfTries = WIFI_MAX_RETRIES;

  while (true) {
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: 
        Serial.printf("[WiFi] SSID not found\n"); 
        break;

      case WL_CONNECT_FAILED:
        Serial.printf("[WiFi] Failed - WiFi not connected! \n");
        vTaskDelete(NULL); 
        break;

      case WL_CONNECTION_LOST: 
        Serial.printf("[WiFi] Connection was lost\n"); 
        break;

      case WL_SCAN_COMPLETED:  
        Serial.printf("[WiFi] Scan is completed\n"); 
        break;

      case WL_DISCONNECTED:    
        //Serial.printf("[WiFi] WiFi is disconnected\n"); 
        break;

      case WL_CONNECTED:
        Serial.printf("[WiFi] WiFi is connected!\n");
        xTaskCreatePinnedToCore(connect_mqtt, "task_mqtt", 4096, NULL, 1, NULL,1);
        return;

      default:
        //Serial.printf("[WiFi] WiFi Status: %d\n", WiFi.status());
        break;
    }
    vTaskDelay(1);
  }
}

/* MQTT Functions ----------------------------------------------------------- */
/**
 * @brief Establishes and maintains MQTT connection
 * @param pvParameters FreeRTOS task parameters (unused)
 */
void connect_mqtt(void *pvParameters) {
  char clientId[50];
  long r = random(1000);
  sprintf(clientId, "clientId-%ld", 1);
  Serial.printf("\n[MQTT] Connecting to %s\n", MQTT_SERVER);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setSocketTimeout(60);
  client.setKeepAlive(60);
  while (!client.connect(clientId)) {
    Serial.printf(".");
    vTaskDelay(RETRY_DELAY);
  }

  if (!client.connected()) {
    Serial.printf("[MQTT] Timeout\n");
    vTaskDelete(NULL);     
  }

  Serial.printf("[MQTT] Connected\n");

  Serial.printf("[MQTT] subscribe to topic: %s\n", SUBSCRIBE_TOPIC);
  client.subscribe(SUBSCRIBE_TOPIC,1);
  xTaskNotifyGive(xCommunicationTaskHandle);
  mqtt_established = true;
  xTaskCreatePinnedToCore(communication_mqtt_task, "task_publish", 4096, NULL, 1, NULL,1);
  client.loop();

  // Main MQTT maintenance loop
  // while (1) {
  //   client.loop();
  //   vTaskDelay(MQTT_LOOP);
  // }

  vTaskDelete(NULL); 
}
  
/**
 * @brief Publishes data to MQTT broker
 * @param val Value to publish
 * @param i Sample index
 */
void send_to_mqtt(float val){

  unsigned long timestamp = millis();
  snprintf(msg, MSG_BUFFER_SIZE, "{\"value\":%.2f,\"time\":%lu}",val, timestamp);    
  Serial.printf("**** %s ****\n", msg);

  if(client.publish(PUBLISH_TOPIC, msg)){
    Serial.printf("[MQTT] Publishing rms: %s\n", msg);
    client.disconnect();
    xTaskNotifyGive(fft_sampling_task_handle);
  }else{
    Serial.printf("[MQTT] ERROR while publishing rms: %s\n", msg);
    if (!client.connected()) {
      vTaskDelete(NULL); 
    }
  }
}


/* Main Communication Task -------------------------------------------------- */
/**
 * @brief Handles outgoing MQTT communications
 * @param pvParameters FreeRTOS task parameters (unused)
 */
void communication_mqtt_task(void *pvParameters){
    Serial.println("------ communication_mqtt_task ------");

    float val = 0.0;
    while(1){
      if(xQueueReceive(xQueue_rms, &val, (TickType_t)portMAX_DELAY)) {
        send_to_mqtt(val);
      }
    }
  vTaskDelete(NULL); 
}
