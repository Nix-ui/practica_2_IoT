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
bool serverConnected = false;

void maintainConnection() {
    if(!server.isConnected()) {
        Serial.println("[CONN] Intentando conectar al servidor...");
        serverConnected = server.connectToServer();
        
        if(serverConnected) {
            Serial.println("[CONN] Conexión establecida");
            // Re-registrar LEDs después de reconexión
            Led::State initialState = Led::State::OFF;
            uint8_t stateBytes[1];
            for(uint8_t i = 0; i < 3; i++) {
                memcpy(stateBytes, &initialState, sizeof(Led::State));
                server.post(
                    ServerClient::Resource::RES_LED, 
                    wireLed.getLedPin(i), 
                    stateBytes, 
                    sizeof(Led::State)
                );
            }
        } else {
            Serial.println("[CONN] Falló la conexión");
        }
    }
}

void changedInterval() {
    char buffer[32] = {0};
    
    if(server.get(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET)) {
        byte status = server.receiveStatus();
        
        if(status == ServerClient::Status::STATUS_OK) {
            if(server.receiveData(
                ServerClient::Resource::RES_SENSOR, 
                SENSOR_ID_TO_GET, 
                buffer, 
                sizeof(buffer))
            ) {
                byte intervalGet = buffer[0];
                Serial.print("[DATA] Intervalo recibido: ");
                Serial.println(intervalGet);

                if(intervalGet >= 1 && intervalGet <= 3 && actualInterval != intervalGet) {
                    uint16_t targetPin = wireLed.getLedPin(intervalGet - 1);
                    
                    // Actualizar estado local
                    wireLed.setAllState(Led::State::OFF);
                    wireLed.setState(targetPin, Led::State::ON);
                    
                    // Actualizar servidor
                    Led::State state = Led::State::ON;
                    uint8_t stateBytes[1];
                    memcpy(stateBytes, &state, sizeof(Led::State));
                    
                    if(server.update(
                        ServerClient::Resource::RES_LED, 
                        targetPin, 
                        stateBytes, 
                        sizeof(Led::State))
                    ) {
                        actualInterval = intervalGet;
                        Serial.print("[LED] Actualizado pin ");
                        Serial.println(targetPin);
                    }
                }
            }
        } else {
            Serial.print("[ERROR] Código de estado: 0x");
            Serial.println(status, HEX);
        }
    } else {
        Serial.println("[ERROR] Falló la solicitud GET");
    }
}

void setup() {
    Serial.begin(115200);

    if(!server.wifiConnect("Matheo", "cg4hts55fh4fyr7")) {
        Serial.println("[FATAL] Error en WiFi");
        while(true);
    }

    maintainConnection();

    wireLed.setAllState(Led::State::OFF);
    Serial.println("[INIT] Sistema listo");
}

void loop() {
    maintainConnection();
    
    if(server.isConnected()) {
        changedInterval();
        wireLed.updateAll();  // Para parpadeos
    }
    
    delay(1000);
}