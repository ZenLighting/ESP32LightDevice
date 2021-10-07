#include <Adafruit_NeoPixel.h>
#include <SPIFFS.h>

class LightController{
    public:
    int begin();
    int pushFrame(uint8_t* frame, int frameLength);
    
    private:
    int lightLength;
    Adafruit_NeoPixel strip;
};