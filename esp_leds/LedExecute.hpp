#pragma once
#include "ServerClient.hpp"
#include "LedController.hpp"

class LedExecute{
    private:
        ServerClient server;
        Led* leds;
        byte actualInterval;
        LedController wireLed;
        const char* ssid;
        const char* password;
        unsigned long lastUpdate;
        uint16_t SENSOR_ID_TO_GET;
        void updateLedState(byte newInterval) {
            if(newInterval == actualInterval || newInterval < 1 || newInterval > 3) return;
            
            uint16_t targetPin = wireLed.getLedPin(newInterval - 1);
            wireLed.setAllState(Led::State::OFF);
            wireLed.setState(targetPin, Led::State::ON);
            
            // Corrección aplicada aquí
            uint8_t stateBytes[1];
            Led::State stateOn = Led::State::ON;  // Variable local
            memcpy(stateBytes, &stateOn, sizeof(Led::State));
            
            if(server.update(ServerClient::Resource::RES_LED, targetPin, stateBytes, sizeof(Led::State))) {
                actualInterval = newInterval;
                Serial.print("LED actualizado: Pin ");
                Serial.println(targetPin);
            }
        }
        void checkSensorInterval() {
            char buffer[32] = {0};
            if(server.get(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET)) {
                if(server.receiveData(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET, buffer, 32)) {
                    updateLedState(static_cast<byte>(buffer[0]));
                }
            }
        }        
    public:
        LedExecute(uint16_t SENSOR_ID,Led leds[],uint8_t firstPin,uint8_t numLeds, const char* serverIP, uint16_t serverPort, const char* ssid, const char* password):
        leds(leds),
        server(serverIP,serverPort),ssid(ssid),password(password),
        wireLed(leds,numLeds,firstPin),
        actualInterval(0),lastUpdate(0),SENSOR_ID_TO_GET(1){};
        void setup(){
            server.wifiConnect(ssid, password);
            server.connectToServer();
            wireLed.setAllState(Led::State::OFF);
        }
        void loop() {
            if(millis() - lastUpdate > 300) {  // Consulta cada 300ms
                checkSensorInterval();
                lastUpdate = millis();
            }
            wireLed.updateAll();
            delay(10);
        }
};