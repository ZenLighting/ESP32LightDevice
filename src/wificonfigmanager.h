#include <WiFi.h>
#include <SPIFFS.h>

enum WifiConfigManagerModes {
    STA,
    AP
};


class WifiConfigManager{
    private:
        char ssid[50];
        char password[50];
        bool startWifiConfigAP();
    public:
        int begin();
        WifiConfigManagerModes currentMode;
        int setWifiConfig(String ssid, String password);
        int tick();
        bool attemptWifiConnection();
};