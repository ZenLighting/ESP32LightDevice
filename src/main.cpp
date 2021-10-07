#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "ESPAsyncWebServer.h"
#include "wificonfigmanager.h"
#include "lightmanager.h"

WifiConfigManager wifiManager;
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
LightController lightController;

String header;
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
}

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(!index){
    Serial.printf("BodyStart: %u B\n", total);
  }
  for(size_t i=0; i<len; i++){
    Serial.write(data[i]);
  }
  if(index + len == total){
    Serial.printf("BodyEnd: %u B\n", total);
  }
}

void connectToMQTT(char* mqttHost){
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
}

void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  SPIFFS.begin(true);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Requested Index.html");
    //request->send(200, "HELLO");
    request->send(SPIFFS, "/index.html");
  });
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
      Serial.println("Got POST");
      if(request->hasArg("ssid") && request->hasArg("password")){
        String ssid = request->arg("ssid");
        String password = request->arg("password");
        Serial.println(ssid);
        Serial.println(password);
        wifiManager.setWifiConfig(ssid, password);
        Serial.printf("%s %s\n", ssid.c_str(), password.c_str());
        request->send(200);
        //server.end();
        if(wifiManager.attemptWifiConnection()){
          //connected to wifi
          Serial.println("Connected to WIFI from main");
        } else{
          //server.begin();
        }
      } else{
        Serial.println("Missing args");
      }
  });
  delay(1000);
  if(lightController.begin() != 0){
    Serial.println("Something went wrong with the lights");
  }
  if(wifiManager.begin()){
    //we are in the not connected to wifi state, start web server up
    Serial.println("Starting async server");
    server.begin();
  }
}

void loop() {
  if(wifiManager.tick() == 1){
    server.begin();
  }
}