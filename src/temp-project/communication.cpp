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
#define MSG_BUFFER_SIZE 256  // Maximum size for MQTT messages

/* Global Variables --------------------------------------------------------- */

WiFiClient espClient;        // WiFi client instance
PubSubClient client(espClient);  // MQTT client instance

char msg[MSG_BUFFER_SIZE];   // Buffer for MQTT messages

/* WiFi Management ---------------------------------------------------------- */
/**
 * @brief Initializes and manages WiFi connection
 * @note Implements retry logic with status monitoring
 */
void wifi_init(){
  Serial.printf("\n[WiFi] Connecting to %s\n", WIFI_SSID);
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
        Serial.printf("[WiFi] WiFi is disconnected\n"); 
        break;

      case WL_CONNECTED:
        Serial.printf("[WiFi] WiFi is connected!\n");
        xTaskCreatePinnedToCore(connect_mqtt, "task_mqtt", 4096, NULL, 1, NULL,1);
        return;

      default:
        Serial.printf("[WiFi] WiFi Status: %d\n", WiFi.status());
        break;
    }

    vTaskDelay(RETRY_DELAY);

    if (numberOfTries <= 0) {
      Serial.printf("[WiFi] Max retries exceeded\n");
      WiFi.disconnect();
      vTaskDelete(NULL); 
    } else {
      numberOfTries--;
    }
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
  sprintf(clientId, "%d", id_device);
  Serial.printf("\n[MQTT] Connecting to %s\n", MQTT_SERVER);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  
  while (!client.connect(clientId)) {
    Serial.printf(".");
    vTaskDelay(RETRY_DELAY);
  }

  if (!client.connected()) {
    Serial.printf("[MQTT] Timeout\n");
    vTaskDelete(NULL);     
  }

  Serial.printf("[MQTT] Connected\n");

  xTaskCreatePinnedToCore(communication_mqtt_task, "task_publish", 4096, NULL, 1, NULL,1);
  
  // Main MQTT maintenance loop
  while (1) {
    client.loop();
    vTaskDelay(MQTT_LOOP);
  }

  vTaskDelete(NULL); 
}

bool prepare_signed_json(data_to_send_t data_to_send, char* json_buffer, size_t buffer_size) {
  // Create a JSON document for the payload
  StaticJsonDocument<200> doc;
  
  // Add sensor data
  if(data_to_send.anomaly)
    doc["status"] = "ANOMALY";
  
  doc["x"] = data_to_send.rms_array[0];
  doc["y"] = data_to_send.rms_array[1];
  doc["z"] = data_to_send.rms_array[2];
  
  // Prepare authentication data
  auth_data_t auth;
  auth.session_id = g_session_id;
  auth.seq_number = get_next_sequence_number();
  auth.timestamp = data_to_send.time_stamp;
  
  // Add authentication data to JSON
  doc["session_id"] = auth.session_id;
  doc["seq"] = auth.seq_number;
  doc["time"] = auth.timestamp;
  
  // Serialize the JSON document to a temporary buffer for HMAC calculation
  char temp_buffer[200];
  size_t json_len = serializeJson(doc, temp_buffer, sizeof(temp_buffer));
  
  // Calculate HMAC
  uint8_t hmac_result[HMAC_OUTPUT_LENGTH];
  if (!calculate_hmac(temp_buffer, json_len, &auth, hmac_result)) {
    Serial.println("[AUTH] Failed to calculate HMAC");
    return false;
  }
  
  // Convert HMAC to hex string
  char hmac_hex[HMAC_OUTPUT_LENGTH * 2 + 1];
  hmac_to_hex_string(hmac_result, hmac_hex);
  
  // Add HMAC to JSON document
  doc["hmac"] = hmac_hex;
  
  // Serialize the final JSON with HMAC
  size_t final_len = serializeJson(doc, json_buffer, buffer_size);
  if (final_len >= buffer_size - 1) {
    Serial.println("[AUTH] JSON buffer too small");
    return false;
  }
  
  return true;
}


/**
 * @brief Publishes data to MQTT broker
 */
void send_to_mqtt(data_to_send_t data_to_send){

  Serial.printf("**** %s ****\n",msg);
  
  // Prepare signed JSON message
  if (!prepare_signed_json(data_to_send, msg, MSG_BUFFER_SIZE)) {
    Serial.println("[MQTT] Failed to prepare authenticated message");
    return;
  }

  const char* topic = !data_to_send.anomaly ? (String(id_device) + "/" + DATASTREAM_RMS_TOPIC).c_str() : (String(id_device) + "/" + ANOMALY_RMS_TOPIC).c_str();

  if (client.publish(topic, msg)){
    Serial.printf("[MQTT] Publishing: %s\n", msg);
  }else{
    Serial.printf("[MQTT] ERROR while publishing rms: %s\n", msg);
    if (!client.connected()) 
      vTaskDelete(NULL);
  }

  client.disconnect();
  xTaskNotifyGive(fft_sampling_task_handle);
}


/* Main Communication Task -------------------------------------------------- */
/**
 * @brief Handles outgoing MQTT communications
 * @param pvParameters FreeRTOS task parameters (unused)
 */
void communication_mqtt_task(void *pvParameters){
    Serial.println("------ communication_mqtt_task ------");
    data_to_send_t rms_data;
    while(1){
      if(xQueueReceive(xQueue_data, &rms_data, (TickType_t)portMAX_DELAY)) {
        send_to_mqtt(rms_data);
      }
    }
  vTaskDelete(NULL); 
}
