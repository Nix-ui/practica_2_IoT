#include "LedExecute.hpp"


LedExecute application(1,{Led(21),Led(22),Led(23)},21,3,"192.168.129.175", 12345,"Walter", "Saxomofon1");

void setup(){
    application.setup();
}
void loop(){
    application.loop();
}