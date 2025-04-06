#include "UltraSonicSensor.hpp"
#include "ServerClient.hpp"
const uint16_t SENSOR_ID=1;


UltraSonicSensor mySensor(2,4);
ServerClient server("192.168.50.62", 12345);
byte actualInterval = 0;
void intervalChanged(){
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
      if (server.update(ServerClient::Resource::RES_SENSOR, SENSOR_ID, intervalToSend, 1)) {
        int status = server.receiveStatus();
        Serial.print("POST Sensor Status: 0x");
        Serial.println(status, HEX);
      }
      server.disconnectFromServer();
    }
  }
}

void setup() {
  Serial.begin(115200);
  server.wifiConnect("WIFI_NAME","PASSWORD");
  uint8_t intervalToSend[2]; // Array de 2 bytes
  intervalToSend[0] = actualInterval;
  intervalToSend[1] = 0;
  server.post(ServerClient::Resource::RES_SENSOR, SENSOR_ID, intervalToSend, 1);
} 

void loop() {
  intervalChanged();
  Serial.println(actualInterval);
  delay(1000);
}