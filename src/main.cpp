#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncWebSocket.h>
#include <Wire.h>
#include <LiquidCrystal.h>

/* Declare any sensor here */
#define DHTpin 15
#define DHTType DHT11
#define MESSAGE_LENGTH 1000

/* Wifi connection */
const char* ssid = "ThaoAn/2.4G";
const char* password = "123456789";

const char* JSON_MIME = "application/json";
const char* TEXT_MIME = "text/plain";
// Change here to send data
const int deviceId = 3;
DHT dht(DHTpin, DHTType);

//String sendDataWebsite = "https://aqua-iot.pro/api/v1/sensordatas";
String hostname = "http://aqua-iot.pro/api/v1/sensordatas";
String twinurl = "http://aqua-iot.pro/api/v1/devices/"+String(deviceId);
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;


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
            //Tạo document để thực hiện serialize và deserialize json, xem thêm tại: https://arduinojson.org/
            DynamicJsonDocument doc(MESSAGE_LENGTH);
            String data_str = "";

            //Cấu trúc tin nhắn gửi dữ liệu
            /* 
            {
                "deviceId": int,
                "data": string (dạng json)
            } 
            */
            // Bước 1: tạo chuỗi JSON cho trường data
            doc["humidity"] = dht.readHumidity();
            doc["temperature"] = dht.readTemperature();
            serializeJson(doc, data_str);

            //Bước 2: tạo tin nhắn dưới dạng JSON
            doc.clear();
            doc["deviceId"] = deviceId;
            doc["data"] = data_str;
            String sensorData = "";
            serializeJson(doc, sensorData);

            //Bước 3: Setup http client, bao gồm tên miền gửi lên và header
            http.begin(hostname.c_str());
            http.addHeader("Content-Type", JSON_MIME);
            http.addHeader("Connection", "keep-alive");
            http.addHeader("Accept", JSON_MIME);

            //Bước 4: Gửi dữ liệu lên, method là POST rồi in ra
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

            //Phần 2: Thực hiện nhận chuỗi điều khiển
            Serial.println("\n[+] Starting get method to receive desired twin:");
            //Bước 1: Setup http client với hostname
            http.begin(twinurl.c_str());
            http.addHeader("Accept", JSON_MIME);
            //Bước 2: thực hiện phương thức GET để lấy dữ liệu
            int httpResponseCode = http.GET();
            if (httpResponseCode>0) {
                //Bước 3: Lấy dữ liệu từ chuỗi JSON nhận về, cấu trúc của nó có dạng như sau
                /* 
                {
                    "data": {
                        "id": 3,
                        "name": "espcong",
                        "enabled": "Enabled",
                        "connectionState": "Disconnected",
                        "deviceToCloudMessages": 131,
                        "cloudToDeviceMessages": 0,
                        "desired": "{\"hello\": \"whatlad\", 
                            \"desired_ph\": \"7\", 
                            \"desired_temp\": \"20\", 
                            \"desired_humid\": \"85\"}",
                        "reported": "{}"
                    }
                }
                */
                Serial.print("[+] HTTP Response code: ");
                Serial.println(httpResponseCode);
                String payload = http.getString();
                Serial.println("[+] Payload: " + payload);
                //Phân tích dữ liệu trả về
                deserializeJson(doc, payload);

                //Lấy dữ liệu desired
                String desired = doc["data"]["desired"];
                Serial.println("[+] Desired string: " + desired);
                doc.clear();
                //Biến dữ liệu desired thành object!
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