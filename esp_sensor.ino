#include "UltraSonicSensor.hpp"
#include "ServerClient.hpp"
const uint16_t SENSOR_ID=1;


UltraSonicSensor mySensor(2,4);
ServerClient server("192.168.50.62", 12345);
byte actualInterval = 0;
void intervalChanged() {
  byte cm = mySensor.getDistanceCm();
  byte interval = (cm / 30) + 1;

  // Debug: Mostrar valores calculados
  Serial.print("Distancia (cm): ");
  Serial.print(cm);
  Serial.print(" -> Intervalo calculado: ");
  Serial.println(interval);

  if (actualInterval != interval && interval < 4) {
    actualInterval = interval;
    uint8_t intervalToSend[4]; // Usar 4 bytes para compatibilidad con servidor
    memset(intervalToSend, 0, sizeof(intervalToSend));
    intervalToSend[0] = interval;

    if (server.connectToServer()) {
      bool success = server.update( // Usar UPDATE en lugar de POST
        ServerClient::Resource::RES_SENSOR, 
        SENSOR_ID, 
        intervalToSend, 
        sizeof(intervalToSend)
      );
      
      if (success) {
        byte status = server.receiveStatus();
        Serial.print("UPDATE Sensor Status: 0x");
        Serial.println(status, HEX);
      } else {
        Serial.println("Fallo al enviar UPDATE");
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