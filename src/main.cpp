#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncWebSocket.h>
#include <Wire.h>

/* Declare any sensor here */
/* 
    DHT: pin 15,
    LED: pin 2 cua ESP32  
*/
#define MESSAGE_LENGTH 1000
#define LEDPin 2

/* Wifi connection */
const char* ssid = "ThaoAn/2.4G";
const char* password = "123456789";

const char* JSON_MIME = "application/json";
const char* TEXT_MIME = "text/plain";
// Change here to send data
const int deviceId = 7;

String hostname = "https://aqua-iot.pro/api/v1/sensordatas";
String twinurl = "https://aqua-iot.pro/api/v1/devices/"+String(deviceId);
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
    pinMode(LEDPin, OUTPUT);
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
                "data": json
            } 
            */
            // Bước 1: tạo chuỗi JSON cho trường data
            doc["ph"] = random(0, 9);
            doc["tds"] = random(0, 1000);
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
                        "id": x,
                        "name": x,
                        "enabled": x,
                        "connectionState": x,
                        "deviceToCloudMessages": x,
                        "cloudToDeviceMessages": x,
                        "desired": {"led":true},
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