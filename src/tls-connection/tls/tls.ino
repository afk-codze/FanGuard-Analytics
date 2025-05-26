#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "shared-defs.h"
#include "communication.h"
#include <ArduinoJson.h>


const char* ssid = "Angelo";
const char* password = "Pompeo00";
const char* mqtt_server = "192.168.210.29";
const int mqtt_port = 8883;

const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDkTCCAnmgAwIBAgIUcUX7wPMwUka5mU1YVlF0ueG4NocwDQYJKoZIhvcNAQEL
BQAwWDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM
GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDERMA8GA1UEAwwIZmFuZ3VhcmQwHhcN
MjUwNTIzMTQwMDQ1WhcNMzUwNTIxMTQwMDQ1WjBYMQswCQYDVQQGEwJBVTETMBEG
A1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkg
THRkMREwDwYDVQQDDAhmYW5ndWFyZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC
AQoCggEBAJ/krZQJrsdoHpdOEHmEHJe/2F+1DhWhuswi6puOmPFmeONmscwZHoGB
pColJXtucsfU6NSblIrfeuT4O4bjyMEyJch/YqZM1QFDB+MiufJfCLiG9EQh7pEE
hx/Ki9cHbdpS6G0GHg/zGjerEIhKhHoVX3pTTNy62UdFl9USxSQtzrBMc3Xl9dx3
TKVDqdBZtQyGjFLOQYlw2TqsDAulQRcUywKGL+lnWF4QJeApPGLkt3yXOREv7Mi1
5tTPA7YN01cyaSD/e5x8k+9KQA11j8OVKFIYd+3hl+//47/cfEDkWt65zFBOYA4i
lPRNr3DwB8iNQBoTfVmKCf+vYuaFZh8CAwEAAaNTMFEwHQYDVR0OBBYEFAXh1U6W
Wnke0UReujrM111uyJPfMB8GA1UdIwQYMBaAFAXh1U6WWnke0UReujrM111uyJPf
MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBABJFTAYdseuUN766
OO+zU+K4h61aU3qQWKiS/LUg4lvfkL0bD8ZahVaN7GHkOxUSk4nug+YiRu24sDcK
JlUctxk5xF8UnXw12xncJmuF/uEJ3B9htZN5vIg3znPdHE0a0T+ZRdIu44dXm+MX
EGa0Mv29YWoEykoVgysPfBzzfPVmdhRDp8W/qrE1KPyOMw298hHDCOk+sle46CYd
CnKdWANnlexfUrBw15kjbfQv14TOIecayobvLwOI0HVSnixNP5ImV2Poxpn5SPM+
zwHe88qnSO1SwkkXyGUrbQUHNi2PXGuOxrYWu7dITrjvz4zADkXhnKQ73cj11oaH
rZegh7A=
-----END CERTIFICATE-----
)EOF";


const char* client_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDPjCCAiYCFHHSlwOB7/9r2vqdUUKBu+AqKTjcMA0GCSqGSIb3DQEBCwUAMFgx
CzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl
cm5ldCBXaWRnaXRzIFB0eSBMdGQxETAPBgNVBAMMCGZhbmd1YXJkMB4XDTI1MDUy
MzE0MDE0NFoXDTM1MDUyMTE0MDE0NFowXzELMAkGA1UEBhMCQVUxEzARBgNVBAgM
ClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEY
MBYGA1UEAwwPZmFuZ3VhcmRfY2xpZW50MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A
MIIBCgKCAQEA2Y1sWdnSBWR1Bfh4OqNXL7r5JZOFHi7zzL+miYEmRwdGmDIrQEMC
y3hShyp807GshyHX5kBMxuiJj0CBHqKblQQQZzQCB5YVcp+/35r34vKJvVk2zb8k
rhpAYNb5mRT5rkyM1Uc2Wp8kvKbdQ7Y9dmNaEL3mUQWEHll0Jcmui7URnXjHptFO
4GpAKBvKsgjOof76+A4mZ+GuvsbZDa6RmIfa0sqK0Ygr5rWxH/KL8cntvAmQQ/mP
cGBRNSg6VMcV1atDFpY0CrTEbml8zElTipHtwWdVFTEb//xonRMNGggK5zAGdH7e
+CutCfmQIL7s8j2opoo9zsCdK8XJm7BdOQIDAQABMA0GCSqGSIb3DQEBCwUAA4IB
AQBfn+h3j9yIzxhoL2Q/Ngg7IzeOT7gbtWbJe/LpAsw50RIhsPyldCjYT+Wb+c7L
uoXPU42LVlilqIx0r1YqBGZrUVAeI6RrzeYf+Z/SrPYHV2EiY9MIUrhcmI5HHcfV
K3p0A+BGMQMo5t6M9pX6JgzAdXP2QygNq4YzdwCOGWrCognxfCIFGChwTX7s+B5C
d9qcCdgHVFH8oV4+KUDUrXmqU+EflsUdRnjmyQAIJR34M8b0zSh155IWLdCdinrM
lys9WHQlq2kuyLQZaYfQ+3dQifRA0bRljY0Kz3Bj0xCBHmU1mE8a2sxeIqptapFy
0egx09hfGi3oklnRxL5hbj8r
-----END CERTIFICATE-----
)EOF";

#define TLS 1 // 0 for HMAC 1 for TLS

#if TLS == 1
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif 

PubSubClient client(wifiClient);


unsigned long handshake_start;



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

void send_mqtt(){

    handshake_start = micros();
    #if TLS == 1 
    wifiClient.setCACert(ca_cert);  
    #endif
    client.setServer(mqtt_server, mqtt_port);
    if (client.connect("arduinoClient")) {
        
        Serial.println("Connected securely to MQTT broker!");
        


        #if TLS == 1
            client.publish("test/topic", "{\"status\":\"ANOMALY\",\"x\":0.266646,\"y\":1.059339,\"z\":0.406033,\"session_id\":10,\"seq\":1,\"time\":3484}");
            Serial.print("TLS Transmission Time: ");
        #endif

        #if TLS == 0
            char json_buffer[256];  // Ensure the buffer is large enough for your JSON payload

                // Create and populate a data_to_send_t instance
                data_to_send_t data_to_send;
                data_to_send.time_stamp = 123456789;  // Example timestamp
                data_to_send.rms_array = new float[3]{1.23, 4.56, 7.89};  // Example RMS values
                data_to_send.anomaly = true;  // Example anomaly flag

                // Call the function
                if (prepare_signed_json(data_to_send, json_buffer, sizeof(json_buffer))) {
                    Serial.println("[JSON] Successfully prepared JSON:");
                    Serial.println(json_buffer);  // Print the resulting JSON
                } else {
                    Serial.println("[JSON] Failed to prepare JSON");
                }

            client.publish("test/topic", json_buffer);
            Serial.print("HMAC Transmission Time: ");
        #endif

        
       unsigned long handshake_end = micros();
        Serial.println(handshake_end - handshake_start);
    } else {
        Serial.println("Failed to connect");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    
}

void loop() {
    client.loop();
    send_mqtt();
    delay(1000);

}
