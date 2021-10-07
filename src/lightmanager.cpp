#include "lightmanager.h"

int LightController::begin(){
    //read config file
    File ledConfig = SPIFFS.open("/lightconfig.txt", "r");
    if(ledConfig){
        //get length, pin, and start color from config file
        uint8_t strip_length, pin, brightness;
        uint8_t grbcolor[3];
        ledConfig.readBytes((char*)&strip_length, 1);
        ledConfig.readBytes((char*)&pin, 1);
        ledConfig.readBytes((char*)&brightness, 1);
        ledConfig.readBytes((char*)grbcolor, 3);
        Serial.printf(
            "Configuring strip> Length: %d Pin: %d Brightness: %d (%d, %d, %d)\n",
            strip_length,
            pin,
            brightness,
            grbcolor[0],
            grbcolor[1],
            grbcolor[2]
        );
        this->lightLength = strip_length;
        this->strip = Adafruit_NeoPixel(strip_length, pin, NEO_GRB+NEO_KHZ800);
        this->strip.setBrightness(brightness);
        for(int i=0;i<strip_length;i++){
            this->strip.setPixelColor(i, grbcolor[0], grbcolor[1], grbcolor[2]);
        }
        this->strip.show();
        return 0;
    } else{
        Serial.println("Couldnt Config LED");
        return 1;
    }
}

int LightController::pushFrame(uint8_t* frame, int frameLength){
    //make sure the frame has the correct length
    if(frameLength > this->lightLength*3){
        //dont do anything this frame is bad
        return -1;
    }
    //slow method?
    int f = 0;
    for(int i=0;i<frameLength;i++){
        f = 3*i;
        this->strip.setPixelColor(i ,frame[f], frame[f+1], frame[f+2]);
    }
    this->strip.show();
}