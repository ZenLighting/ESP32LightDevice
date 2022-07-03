/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
//#include <WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>
//#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

// ========================= configfiles.h ===================


char* stripNewline(char* toFormat){
    int length = strlen(toFormat);
    if(toFormat[length-1] == '\r' || toFormat[length-1] == '\n'){
        toFormat[length-1] = 0;
    }
    return toFormat;
}

class WifiConfigFile{
    public:
        int begin();
        bool initialized=false;
        char ssid[50];
        char password[50];
        char hwid[50];
};

int WifiConfigFile::begin(){
    //SPIFFS must be initialized
    //make sure the file exists
    File file = SPIFFS.open("/network.txt", "r");
    if(!file){
        Serial.println("network.txt was not available for reading");
        return -1;
    }
    String ssid = file.readStringUntil('\n');
    String password = file.readStringUntil('\n');
    String hwid = file.readStringUntil('\n');
    Serial.println(ssid);
    Serial.println(password);
    Serial.println(hwid);
    bzero(this->ssid, 50);
    bzero(this->password, 50);
    strcpy(this->ssid, ssid.c_str());
    if(this->ssid[strlen(this->ssid)-1] == '\r' || this->ssid[strlen(this->ssid)-1] == '\n'){
        this->ssid[strlen(this->ssid)-1] = 0;
    }
    strcpy(this->password, password.c_str());
    strcpy(this->hwid, hwid.c_str());
    Serial.printf("Network Config -> SSID: %s, Password: %s\n", this->ssid, this->password);
    this->initialized = true;
    file.close();
    return 1;
}

class LightConfigFile{
    public:
        int begin();
        bool initialized=false;
        uint8_t stripLength;
        uint8_t pin;
};

int LightConfigFile::begin(){
    //SPIFFS must be initialized
    //make sure the file exists
    File file = SPIFFS.open("/lights.txt", "r");
    if(!file){
        Serial.println("lights.txt was not available for reading");
        return -1;
    }
    String lightLength = file.readStringUntil('\n');
    String pinString = file.readStringUntil('\n');
    this->pin = atoi(pinString.c_str());
    this->stripLength = atoi(lightLength.c_str());
    Serial.printf("Light Config -> Strip: %d, pin: %d\n", this->stripLength, this->pin);
    this->initialized = true;
    file.close();
    return 1;
}

class Timer{
    public:
        Timer(unsigned long delayms, void (*runnable)(unsigned long));
        void tick(unsigned long current_time);
    private:
        unsigned long delayMs;
        unsigned long startTime;
        void (*toRun)(unsigned long);
};

Timer::Timer(unsigned long delayms, void (*runnable)(unsigned long)){
    this->toRun = runnable;
    this->delayMs = delayms;
    this->startTime = millis();
}

void Timer::tick(unsigned long current_time){
    if(current_time - this->startTime >= this->delayMs){
        (*this->toRun)(current_time);
        this->startTime = current_time;
    }
}

// ==================== global defs

#define LEDPIN 13
#define LEDLENGTH 38

WifiConfigFile wifiConfig;
LightConfigFile lightConfig;
//UDPBroadcastManager broadcaster;
// WiFi network name and password:
//const char * networkName = "NETGEAR30";
//const char * networkPswd = "gentlecream139";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "192.168.1.255";
const int udpPort = 1261;
const int broadcastPort = 1260;
const int messageBufferLength = 200;
String hwId = "RingLightDevice";
char messageBuffer[messageBufferLength];
int messageSize;
CRGB leds[LEDLENGTH];
//Adafruit_NeoPixel* lightStrip;
IPAddress broadcastAddress;

//Are we currently connected?
boolean connected = false;
unsigned long current_time;

//The udp library class
WiFiUDP udp;

void registerWithServer(unsigned long currentTime){
    udp.beginPacket("192.168.1.11", 80);
    udp.printf("HEARTBEAT,ID:%s\n",hwId.c_str());
    udp.endPacket();
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case WL_CONNECTED: {
          //When connected set
          IPAddress localaddr = WiFi.localIP();
          broadcastAddress = IPAddress(localaddr[0], localaddr[1], localaddr[2], 255);
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(udpPort);
          //broadcaster.begin(1000, udp, broadcastPort, hwId);
          connected = true;
          break;
      }
      case WL_DISCONNECTED:
      {
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      }
      default: 
        break;
    }
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);
  /**while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected");**/

  Serial.println("Waiting for WIFI connection...");
}

void sendBroadcast(unsigned long currentTime){
    Serial.println("Broadcasting");
    udp.beginPacket(broadcastAddress, broadcastPort);
    sprintf(
        messageBuffer, 
        "{"
            "\"name\":\"%s\","
            "\"strip\": {"
                "\"length\": %d"
            "},"
            "\"communication\": {"
                "\"protocols\": [0]"
            "}"
        "}",
        wifiConfig.hwid,
        LEDLENGTH
    );
    udp.println(messageBuffer);
    //Serial.println(messageBuffer);
    //Serial.println(udp.endPacket());
    udp.endPacket();
}


enum SetLightEncodingValues{
    LIGHT_BYTES,
    LIGHT_STRING
};

void handleSetLight(char* messageBuffer, int messageLength){
    //make sure message length > 1
    if(messageLength < 1){
        Serial.println("Messagelength not large enough to set lights");
        return;
    }

    if(messageBuffer[0] == LIGHT_BYTES){
        // we take the buffer as raw data and put it into the lights
        // it is encoded as rgb for each pixel -1 for light encoding info
        if(messageLength-1 == LEDLENGTH*3){
            //uint8_t* pixelStrip = lightStrip->getPixels();
            memcpy(leds, &messageBuffer[1], messageLength-1);
            FastLED.show();
            //lightStrip->show();
            return;
        } else{
            Serial.printf("Incorrect size to push to pixels: %d, should be %d", messageLength-1, LEDLENGTH*3);
            return;
        }
    } else if(messageBuffer[0] == LIGHT_STRING){
        Serial.println("String not yet supported");
    } else{
        Serial.printf("Unknown light code %d\n", messageBuffer[0]);
    }
}

unsigned char opcode;
void handleUDPMessage(char* messageBuffer, int messageLength){
    opcode = messageBuffer[0];
    switch(opcode){
        case 0: //set light
            handleSetLight(&messageBuffer[1], messageLength-1);
            break;
        default:
            Serial.printf("Unknown opcode %d\b", opcode);
            break;
    }
}

Timer broadcastTimer(1000, &sendBroadcast);
Timer registerTimer(1000, &registerWithServer);


void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);
  if(!SPIFFS.begin()){
    Serial.println("SPIFFS Failed to mount");
    return;
  }
  wifiConfig.begin();
  lightConfig.begin();
  //Connect to the WiFi network
  connectToWiFi(wifiConfig.ssid, wifiConfig.password);
  Serial.printf("STARTING PIXELS");
  Serial.printf("%d %d \n", lightConfig.stripLength, lightConfig.pin);
  //unfortunately cant change led lengths
  FastLED.addLeds<WS2812, LEDPIN>(leds, LEDLENGTH);

  //lightStrip = new Adafruit_NeoPixel(lightConfig.stripLength, lightConfig.pin, NEO_GRB + NEO_KHZ800);
  Serial.println("HERE");
  Serial.printf("ENDING PIXEL SETUP");
}

void loop(){
  //only send data when connected
  //Serial.println("Looping");
  if(connected){
    current_time = millis();
    registerTimer.tick(current_time);
    broadcastTimer.tick(current_time);
    //Send a packet
    //recv commands
    if(udp.parsePacket()){
        messageSize = udp.read(messageBuffer, messageBufferLength);
        handleUDPMessage(messageBuffer, messageSize);
    }
    //broadcaster.tick(current_time);
  }
}
