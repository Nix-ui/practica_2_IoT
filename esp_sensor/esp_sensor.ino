#include "SensorController.hpp"
const uint16_t SENSOR_ID = 1;

SensorController program(SENSOR_ID, 2, 4, "192.168.129.175", 12345, "Walter", "Saxomofon1");
void setup() {
  program.setup();
} 

void loop() {
  program.loop();
}