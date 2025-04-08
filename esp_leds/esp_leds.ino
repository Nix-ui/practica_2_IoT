#include "ServerClient.hpp"
#include "LedController.hpp"

const uint16_t SENSOR_ID_TO_GET = 1; 
ServerClient server("192.168.87.62", 12345);

// Configuración de LEDs
Led blueLed(21);
Led orangeLed(22);
Led redLed(23);
Led leds[] = {blueLed, orangeLed, redLed};
LedController wireLed(leds, 3, 21);

byte actualInterval = 0;

void changedInterval() {
    char buffer[32]; // Buffer para recibir datos

    if (server.get(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET)) {
        if (server.receiveData(
            ServerClient::Resource::RES_SENSOR, 
            SENSOR_ID_TO_GET, 
            buffer, 
            sizeof(buffer))
        ) {
            byte intervalGet = static_cast<byte>(buffer[0]);
            Serial.print("Intervalo recibido: ");
            Serial.println(intervalGet);
            
            if (actualInterval != intervalGet && intervalGet > 0 && intervalGet <= 3) {
                // Apagar LED anterior usando el controlador
                if (actualInterval != 0) {
                    uint16_t prevPin = wireLed.getLedPin(actualInterval - 1);
                    Led::State stateOff = Led::State::OFF;
                    uint8_t stateBytes[1];
                    memcpy(stateBytes, &stateOff, sizeof(Led::State));
                    
                    server.update(
                        ServerClient::Resource::RES_LED, 
                        prevPin, 
                        stateBytes, 
                        sizeof(Led::State)
                    );
                }

                // Actualizar estado local
                actualInterval = intervalGet;
                wireLed.setAllState(Led::State::OFF);
                wireLed.setState(wireLed.getLedPin(actualInterval - 1), Led::State::ON);

                // Notificar al servidor
                Led::State stateOn = Led::State::ON;
                uint8_t stateBytes[1];
                memcpy(stateBytes, &stateOn, sizeof(Led::State));
                
                server.update(
                    ServerClient::Resource::RES_LED, 
                    wireLed.getLedPin(actualInterval - 1), 
                    stateBytes, 
                    sizeof(Led::State)
                );
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    server.wifiConnect("Matheo", "cg4hts55fh4fyr7");
    Led::State initialState = Led::State::OFF;
    uint8_t stateBytes[1];
    
    if (server.connectToServer()) {
        for (byte i = 0; i < 3; ++i) {
            memcpy(stateBytes, &initialState, sizeof(Led::State));
            server.post(
                ServerClient::Resource::RES_LED, 
                wireLed.getLedPin(i), 
                stateBytes, 
                sizeof(Led::State)
            );
        }
    }
    
    // Estado inicial local
    wireLed.setAllState(Led::State::OFF);
}

void loop() {
    changedInterval();
    wireLed.updateAll(); // Actualizar parpadeos
    delay(100);
}