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
const int deviceId = 42;

String getDeviceData = "https://aqua-iot.pro/api/v1/sensordatas/?deviceId[eq]="+String(deviceId);
String postSensorData = "https://aqua-iot.pro/api/v1/sensordatas";
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

/* DHT 11 */
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
            Serial.println("\n[+] Starting get data method!");
            http.begin(getDeviceData.c_str());
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Connection", "keep-alive");
            http.addHeader("Accept", "application/json");

            int httpResponseCode = http.GET();
            if(httpResponseCode > 0){
                Serial.print("[+] HTTP Response code: ");
                Serial.println(httpResponseCode);
                String payload = http.getString();
                Serial.println(payload);
            } else {
                Serial.printf("[+] Error code: ");
                Serial.println(httpResponseCode);
            }
            Serial.println("\n[+] Starting post method to send data");
            DynamicJsonDocument doc(1000);
            doc["deviceId"] = deviceId;
            doc["humidity"] = dht.readHumidity();
            doc["temperature"] = dht.readTemperature();
            String sensorData = "";
            serializeJson(doc, sensorData);

            http.begin(postSensorData.c_str());
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Connection", "keep-alive");
            http.addHeader("Accept", "application/json");


            int respCode = http.POST(sensorData);
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