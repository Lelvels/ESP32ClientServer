#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncWebSocket.h>
#include <Wire.h>
#include <LiquidCrystal.h>

const char* JSON_MIME = "application/json";
const char* TEXT_MIME = "text/plain";
// Change here to send data
const int deviceId = 23;

//String getDeviceData = "http://aqua-iot.xyz/api/v1/sensordatas/?deviceId[eq]="+deviceId;
//String postSensorData = "http://192.168.0.5/iothub/api/v1/sensordatas";
String sendDataWebsite = "https://aqua-iot.pro/api/v1/sensordatas";
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

/* Declare any sensor here */
#define DHTpin 15
#define DHTType DHT11
DHT dht(DHTpin, DHTType);

/* Wifi connection */
const char* ssid = "Lethuy_2.4Ghz";
const char* password = "11336688";

void setup(){
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if(WiFi.waitForConnectResult() != WL_CONNECTED){
        Serial.printf("Wifi failed to connect!");
        return;
    }
    dht.begin();

    Serial.printf("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    if((millis() - lastTime) > timerDelay){
        if(WiFi.status() == WL_CONNECTED){
            HTTPClient http;
            Serial.println("\n[+] Starting post method to send data");
            DynamicJsonDocument doc(1000);
            String data_str = "";
            doc["humidity"] = dht.readHumidity();
            doc["temperature"] = dht.readTemperature();
            serializeJson(doc, data_str);
            doc.clear();
            doc["deviceId"] = deviceId;
            doc["data"] = data_str;
            String sensorData = "";
            serializeJson(doc, sensorData);

            http.begin(sendDataWebsite.c_str());
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Connection", "keep-alive");
            http.addHeader("Accept", "application/json");

            int respCode = http.POST(sensorData);
            Serial.print("[+] Sending: ");
            Serial.println(sensorData);
            if(respCode > 0){
                Serial.print("[+] Http Response code: ");
                Serial.println(respCode);
                String payload = http.getString();
                Serial.print("[+] Payload: ");
                Serial.println(payload);
            } else {
                Serial.printf("[+] Error code: ");
                Serial.println(respCode);
            }
            http.end();
        } else {
            Serial.println("Wifi disconnected!");
        }
        lastTime = millis();
    }
}