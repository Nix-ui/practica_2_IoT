#include "ServerClient.hpp"
#include "LedController.hpp"

const uint16_t SENSOR_ID_TO_GET = 1;
ServerClient server("192.168.129.175", 12345);

Led blueLed(21);
Led orangeLed(22);
Led redLed(23);
Led leds[] = {blueLed, orangeLed, redLed};
LedController wireLed(leds, 3, 21);

byte actualInterval = 0;
unsigned long lastUpdate = 0;

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

void setup() {
    Serial.begin(115200);
    server.wifiConnect("Walter", "Saxomofon1");
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