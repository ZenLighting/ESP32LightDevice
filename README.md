# ESP32 Light Device
Light devices either run off an esp8266 or an ESP32, the code should be mostly the same

# Configuration
## network.txt
### Description:
Contains network configuration information  
### Content:
ssid  
password

## lights.txt
### Description
Contains information about the light setup
### Content
light strip length (int)  
light strip pin (int)  

## service.txt
### Description
Contains config information related to the connection to external services ex) the light registry we should be hitting with our information ip or address  
### Content
registry address  


# Network Communication Description
For lower overhead all of our communication will be done via UDP, either through UDP network wide broadcasts (discovery), heartbeat to registry server, or actual light state messages from any application

## Message Description
broadcast discovery message:  
```json
{
    "hwid": "<the hardware id of the esp device>",
    "lights": {
        "length": "<the length of the light strip>",
        "state": "<an array of integer values representing rgb state>",
    },
    "service": {
        "registry": "<ip of registry server>",
        "comm_state": "MANUAL_CONTROL, SERVER_CONTROL",
    }
}
```