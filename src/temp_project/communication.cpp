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
#include "esp_task_wdt.h"

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
  //WiFi.mode(WIFI_STA); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
  int numberOfTries = WIFI_MAX_RETRIES;

  while (1) {
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
        return;

      default:
        Serial.printf("[WiFi] WiFi Status: %d\n", WiFi.status());
        break;
    }
    vTaskDelay(500);
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
 * @brief Establishes MQTT connection
 * 
 */
void connect_mqtt() {
  char clientId[50];
  long r = random(1000);
  sprintf(clientId, "%d", id_device);
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

}

void send_mqtt(){
  data_to_send_t rms_data;
  if(anomaly_detected && !anomaly_sent){
    send_to_mqtt(anomaly);
  }
  
  if(xQueueReceive(xQueue_data, &rms_data, (TickType_t)100)) {
    Serial.printf("----%d,%d----",anomaly.time_stamp,rms_data.time_stamp);
    send_to_mqtt(rms_data);
    
  }
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
  
  // Prepare signed JSON message
  if (!prepare_signed_json(data_to_send, msg, MSG_BUFFER_SIZE)) {
    Serial.println("[MQTT] Failed to prepare authenticated message");
    return;
  }

  String topicStr;
  if (!data_to_send.anomaly) {
    topicStr = String(id_device) + "/" + DATASTREAM_RMS_TOPIC;
  } else {
    topicStr = String(id_device) + "/" + ANOMALY_RMS_TOPIC;
  }
  const char* topic = topicStr.c_str();
  Serial.printf("\ndata_to_send.anomaly:%d,anomaly_sent:%d,anomaly_detected:%d\n",data_to_send.anomaly,anomaly_sent,anomaly_detected);
  if(!data_to_send.anomaly || (data_to_send.anomaly && !anomaly_sent)){
    Serial.printf("\n**** %s ****\n",msg);
    //Serial.printf("\ndata_to_send.anomaly:%d,anomaly_sent:%d,anomaly_detected:%d\n",data_to_send.anomaly,anomaly_sent,anomaly_detected);
    Serial.printf("\nTopic: %s\n",topic);
    if (client.publish(topic, msg)){
      Serial.printf("[MQTT] Publishing: %s\n", msg);
      if (data_to_send.anomaly)
        anomaly_sent = true;
    }else{
      Serial.printf("[MQTT] ERROR while publishing rms: %s\n", msg);
      Serial.printf("client.connected(): %d",client.connected());
    }
  }
}

void communication_function(){
  wifi_init();
  connect_mqtt();
  send_mqtt();
}
