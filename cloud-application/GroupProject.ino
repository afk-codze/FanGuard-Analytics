
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>


void mqttReconnect();
void transmissionTask(void* pvParameters);
void sendOverWifiMqtt(double val);


#define QUEUE_SIZE 20

// WiFi connection variables
const char* ssid = "Angelo";
const char* password = "Pompeo00";
const char* mqttServer = "192.168.56.34";  //ip address of smartphone
int port = 1883;
const char* topic = "angelo/signal";
char clientId[50];

QueueHandle_t transmissionQueue;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  // put your setup code here, to run once:
  Serial.println("WiFi connection setup");



  transmissionQueue = xQueueCreate(QUEUE_SIZE, sizeof(double));
  if (transmissionQueue == 0) {
    printf("Failed to create queue= %p\n", transmissionQueue);
  }
  xTaskCreate(transmissionTask, "transmissionTask", 4096, NULL, 1, NULL); // This task is responsible to handle the transmission of the aggregate value over either wifi or lora


}

void loop() {
  // put your main code here, to run repeatedly:

}

// Task to handle transmission
void transmissionTask(void* pvParameters) {
    double value;

    // Connect to WiFi and MQTT server first
    establishConnections();

    while(1) {
        // Ensure WiFi and MQTT connections are valid before attempting to send data
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            reconnectWiFi();
        }

        if (!client.connected()) {
            Serial.println("MQTT disconnected, reconnecting...");
            mqttReconnect();
        }

        // Check if there is data in the transmission queue to send
        if (xQueueReceive(transmissionQueue, &value, pdMS_TO_TICKS(2000))) {
            sendOverWifiMqtt(value);  // Send the data over MQTT
        }

        delay(10);  // Small delay to prevent overloading the CPU
    }
}

// Establish WiFi and MQTT connections
void establishConnections() {
    // WiFi connection setup
    reconnectWiFi();

    // MQTT connection setup
    if (!client.connected()) {
        mqttReconnect();
    }
}

// WiFi reconnect function
void reconnectWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(5000);
        Serial.println("Trying to reconnect to WiFi...");
    }
    Serial.println("WiFi connected successfully");
}

// MQTT reconnect function
void mqttReconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        
        // Unique client ID
        sprintf(clientId, "clientId-%ld", random(10));  // You can change the client ID as needed

        if (client.connect(clientId)) {
            Serial.println("MQTT connected successfully");
        } else {
            Serial.print("MQTT connection failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);  // Retry connection after 5 seconds
        }
    }
}

// Send data over MQTT
void sendOverWifiMqtt(double val) {
    const int MSG_BUFFER_SIZE = 100;
    char msg[MSG_BUFFER_SIZE];

    // Prepare the message to be sent
    snprintf(msg, MSG_BUFFER_SIZE, "%.2f", val);

    Serial.printf("Publishing message: %.2f to topic %s\n", val, topic);

    // Send the message via MQTT
    client.publish(topic, msg);
}