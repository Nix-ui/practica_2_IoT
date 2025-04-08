#include "SensorExecute.hpp"
const uint16_t SENSOR_ID = 1;

SensorExecute aplication(SENSOR_ID, 2, 4, "192.168.129.175", 12345, "Walter", "Saxomofon1");
void setup() {
  aplication.setup();
} 

void loop() {
  aplication.loop();
}