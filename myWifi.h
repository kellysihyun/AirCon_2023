#ifndef myWifi_h
#define myWifi_h

#include "Arduino.h"

#include <WiFi.h>
#include "time.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

const String ssidList[3] = {"Galaxy A123642", "novashin2", "senWifi_FreeXXX_2.4G"};
const String passList[3] = {"", "0311shin", "youngdong 123!"};

const char* _serverUrlLive = "http://geo.pionman.com:5000/airCon/setAirValues";
const char* _serverUrlDev = "http://192.168.0.19:5000/airCon/setAirValues";
const char* serverUrl = _serverUrlLive;

class myWifi {
  public:
    myWifi() {
    };

    bool begin(const char* ssid, const char* password) {
      WiFi.begin(ssid, password);
      int retry = 0;
      while (retry < 3 && WiFi.status() != WL_CONNECTED) {
        delay(1000);
        retry++;
        Serial.println("Connecting to WiFi...");
      }
      if(WiFi.status() != WL_CONNECTED) {
        Serial.println("Fialed WiFi connecting..." + String(ssid) + " : " + String(password));
        return false;
      }
      else {
        Serial.println("WiFi connect : " + String(ssid));
        return true;
      }
    } 
    
    bool tryConnect() {
      for(int idx = 0; idx < sizeof(ssidList); idx++) {
        String _ssid = String(ssidList[idx]);
        if(idx == 2) {
          for(int idx2 = 0; idx2 < 100; idx2++) {
            char str[30];
            sprintf(str, "senWifi_Free%02d_2.4G", idx2);
            //Serial.print(str);
            _ssid = String(str);
            //Serial.println("   gen SSID = " + _ssid);
            bool ret = begin(_ssid.c_str(), String(passList[idx]).c_str()); 
            if(ret)
              return true;
          }
        }

        bool ret = begin(_ssid.c_str(), String(passList[idx]).c_str()); 
        if(ret) {
          // if(idx == 1 )
          //   serverUrl = _serverUrlDev;
          return true;    
        }
      }
    }

    bool sendAirConditionData(char* device, int temp, int mois, int illu, int co2, int tvoc, int dust1, int dust5, int dust10) {
      if (WiFi.status() != WL_CONNECTED) 
        return false;

      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      String device_id(device);
      DynamicJsonDocument doc(1024);
      doc["deviceId"] = device_id;
      doc["temp"] = temp;
      doc["mois"] = mois;
      doc["illu"] = illu;
      doc["co2"] = co2;
      doc["tvoc"] = tvoc;
      doc["dust1"] = dust1;
      doc["dust5"] = dust5;
      doc["dust10"] = dust10;

      String jsonString;
      serializeJson(doc, jsonString);
      Serial.println(jsonString);

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        // String response = http.getString();
        // Serial.println(response);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      http.end();
      delay(1000);
      return true;
    }

    void printLocalTime()
    {
      struct tm timeinfo;
      if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
      }
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }

    void stopWifi() {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }

  private:


};

#endif


