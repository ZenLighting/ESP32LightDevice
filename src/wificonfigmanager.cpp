#include "wificonfigmanager.h"

int WifiConfigManager::begin(){
    //check if all wifi credentials are saved
    bzero(this->ssid, 50);
    bzero(this->password, 50);
    Serial.println("Get the buffer out");
    File wifiConfigFile = SPIFFS.open("/wificonf.txt", "r");
    if(wifiConfigFile){
        Serial.println(wifiConfigFile.readBytesUntil('\n', this->ssid, 50));
        Serial.println(wifiConfigFile.readBytesUntil('\n', this->password, 50));
        wifiConfigFile.close();
        Serial.println("Read Values");
        Serial.println(this->ssid);
        Serial.println(this->password);
        //Serial.printf("(begin) Attempting to connect with SSID: %s PASSWORD: %s\n", this->ssid, this->password);
        if(this->attemptWifiConnection()){
            return 0;
        }
    }
    //if there is no config file or could not connect we need to start access point
    this->startWifiConfigAP();
    return 1;
}

bool WifiConfigManager::attemptWifiConnection(){
    //WiFi.mode(WIFI_STA);
    //Serial.printf("\nConnecting to wifi with SSID: %s and PASSWORD: %s\n", this->ssid, this->password);
    WiFi.begin(this->ssid, this->password);
    Serial.println("Connecting to WiFi with 20 second timeout");
    time_t now;
    time_t new_time;
    time(&now);
    while(true){
        //Serial.print(".");
        if(WiFi.status() == WL_CONNECTED){
            Serial.println("Connected to Wifi");
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            this->currentMode = STA;
            return true;
        }
        time(&new_time);
        if(new_time > now+20){
            break;
        }
    }
    Serial.println("Failed to connect to wifi");
    return false;
}

bool WifiConfigManager::startWifiConfigAP(){
    uint64_t chipid;
    chipid = ESP.getEfuseMac();
    char ssid[27];
    snprintf(ssid, 27, "ZENLIGHTDEVICE-%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, "zenlightdevice");
    this->currentMode = AP;
    Serial.println("Access Point mode initiated");
    Serial.printf("IP: %s \n", WiFi.softAPIP().toString().c_str());
}

int WifiConfigManager::setWifiConfig(String ssid, String password){
    //transfer into config variables
    bzero(this->ssid, 50);
    bzero(this->password, 50);
    strcpy(this->ssid, ssid.c_str());
    strcpy(this->password, password.c_str());
    Serial.printf("Set config variables to SSID: %s PASSWORD: %s\n", this->ssid, this->password);

    File wifiConfigFile = SPIFFS.open("/wificonf.txt", "w");
    wifiConfigFile.printf("%s\n", this->ssid);
    wifiConfigFile.printf("%s\n", this->password);
    wifiConfigFile.close();
    wifiConfigFile = SPIFFS.open("/wificonf.txt", "r");
    Serial.println("Reading File");
    for(int i=0;i<wifiConfigFile.size();i++){
        char c = wifiConfigFile.read();
        Serial.print(c);
    }
    Serial.println("Exit File");
    wifiConfigFile.close();
    /*bzero(this->ssid, 50);
    bzero(this->password, 50);
    strcpy(this->ssid, ssid.c_str());
    strcpy(this->password, password.c_str());*/
}

int WifiConfigManager::tick(){
    if(WiFi.getMode() == WIFI_STA){
        if(!WiFi.isConnected()){
            Serial.println("station has become disconnected");
            this->startWifiConfigAP();
            return 1;
        }
    }
    return 0;
}
