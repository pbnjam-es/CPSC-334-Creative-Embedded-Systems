#include "WiFi.h"

const char* ssid = "yale wireless";
const char* password = "";
const char* piHost = "10.67.69.8";
int piPort = 5000;

WiFiClient client;

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_MODE_STA);
    
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWifi Connected");
    Serial.println(WiFi.localIP());
}

void loop() {
    Serial.println(WiFi.macAddress());
    while (client.connect(piHost, piPort)) {
        int rawValue = analogRead(10);
        client.println(rawValue);
        delay(500);
    }
    
}