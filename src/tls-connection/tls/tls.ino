#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

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

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    wifiClient.setCACert(ca_cert);
    client.setServer(mqtt_server, mqtt_port);

    if (client.connect("arduinoClient")) {
        Serial.println("Connected securely to MQTT broker!");
        client.publish("test/topic", "Hello via TLS!");
    } else {
        Serial.println("Failed to connect");
    }
}

void loop() {
    client.loop();
}
