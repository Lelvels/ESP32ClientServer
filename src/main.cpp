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
const int deviceId = 3;

//String sendDataWebsite = "https://aqua-iot.pro/api/v1/sensordatas";
String hostname = "http://aqua-iot.pro/api/v1/sensordatas";
String twinurl = "http://aqua-iot.pro/api/v1/devices/"+String(deviceId);
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

/* Declare any sensor here */
#define DHTpin 15
#define DHTType DHT11
DHT dht(DHTpin, DHTType);

/* Wifi connection */
const char* ssid = "ThaoAn/2.4G";
const char* password = "123456789";

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
            //Gửi dữ liệu lên server
            Serial.println("\n[+] Starting post method to send data:");
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

            http.begin(hostname.c_str());
            http.addHeader("Content-Type", JSON_MIME);
            http.addHeader("Connection", "keep-alive");
            http.addHeader("Accept", JSON_MIME);

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

            //Nhận dữ liệu về
            Serial.println("\n[+] Starting get method to receive desired twin:");
            http.begin(twinurl.c_str());
            // Send HTTP GET request
            int httpResponseCode = http.GET();
            if (httpResponseCode>0) {
                Serial.print("[+] HTTP Response code: ");
                Serial.println(httpResponseCode);
                String payload = http.getString();
                Serial.println("[+]" + payload);
                deserializeJson(doc, payload);
                
                String desired = doc["data"]["desired"];
                Serial.println("[+]" + desired);
                doc.clear();
                deserializeJson(doc, desired);
                String hello = doc["hello"];
                int ph = doc["desired_ph"];
                int temp = doc["desired_temp"];
                int humid = doc["desired_humid"];
                Serial.println("\n[+] PH mong muon: " + String(ph) + 
                ", Nhiet do mong muon: " + String(temp) + 
                ", Do am mong muon: " + String(humid) +
                ", Nguoi dung gui tin nhan: " + String(hello));
            }
            else {
                Serial.print("[+] Error code: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        } else {
            Serial.println("[+] Wifi disconnected!");
        }
        lastTime = millis();
    }
}