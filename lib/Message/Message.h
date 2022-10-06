#pragma once
#include <Arduino.h>
#include <map>
#include <ArduinoJson.h>
#define MAX_JSON_BUF 1024

class ResponseMessage{
    private:
        /* TODO: Make this can be any kind of key,value */
        char* deviceName;
        std::map<String, String> message;
    public:
        ResponseMessage(){};
        ~ResponseMessage(){};
        ResponseMessage(const char *deviceName, std::map<String, String> message){
            this->deviceName = (char*) deviceName;
            this->message = message;
        }
        String getDeviceName()
        {
            return this->deviceName;
        }
        void setDeviceName(const char* device_name)
        {
            this->deviceName = (char*)device_name;
        }
        void setMessage(std::map<String, String> map){
            this->message = map;
        }
        std::map<String, String> get_message(){
            return message;
        }
        String toJsonString(){
            DynamicJsonDocument doc(MAX_JSON_BUF);
            String jsonString;
            doc["deviceName"] = deviceName; 
            for(auto itr = message.begin(); itr != message.end(); itr++){
                const char* key = itr->first.c_str(); 
                const char* value = itr->second.c_str();
                doc["message"][key] = value;
            }
            serializeJson(doc, jsonString);
            return jsonString;
        }
};

class RequestMessage{
    private:
        String deviceName;
        String hostName;
        String message;
    public:
        RequestMessage();
        ~RequestMessage();
};