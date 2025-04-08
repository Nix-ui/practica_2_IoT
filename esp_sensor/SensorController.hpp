#include "UltraSonicSensor.hpp"
#include "ServerClient.hpp"

class SensorController{
  private:
    UltraSonicSensor mySensor;
    ServerClient server;
    byte actualInterval;
  public:
    SensorController(uint16_t SENSOR_ID,uint8_t triggerPin, uint8_t echoPin, const char* serverIP, uint16_t serverPort, const char* ssid, const char* password):
    mySensor(triggerPin, echoPin),
    server(serverIP, serverPort),
    actualInterval(0){
    mySensor.setSensorId(SENSOR_ID);
    }
    void setup(){
        Serial.begin(115200);
        Serial.println("Sensor Controller Initialized");
        server.wifiConnect(ssid, password);
        server.connectToServer();
        uint8_t intervalToSend[2]; // Array de 2 bytes
        intervalToSend[0] = actualInterval;
        intervalToSend[1] = 0;
        server.post(ServerClient::Resource::RES_SENSOR, mySensor.getSensorId(), intervalToSend, 1);
        server,disconnectFromServer();
    }
    void loop(){
        byte cm = mySensor.getDistanceCm();
        byte interval= (cm/30)+1;
        Serial.print("Raw Distance (cm): ");
        Serial.println(cm);
        if(actualInterval != interval && interval<4){
            actualInterval = interval;
            uint8_t intervalToSend[2]; // Array de 2 bytes
            intervalToSend[0] = interval;
            intervalToSend[1] = 0;
            if(server.connectToServer()){
            if (server.update(ServerClient::Resource::RES_SENSOR, mySensor.getSensorId(), intervalToSend, 1)) {
                int status = server.receiveStatus();
                Serial.print("Update Sensor Status: 0x");
                Serial.println(status, HEX);
            }
            server.disconnectFromServer();
            }
        }
    }
    delay(100);
};