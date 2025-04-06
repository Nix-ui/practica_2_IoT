#include "ServerClient.hpp"
#include "LedController.hpp"

const uint16_t SENSOR_ID_TO_GET = 3; 
ServerClient server("192.168.50.62", 12345);
Led blueLed(21);
Led orangeLed(22);
Led redLed(23);
Led leds[]={blueLed,orangeLed,redLed};
LedController wireLed(leds,3,21);
byte actualInterval = 0;
void changedInterval(){
  if(server.get(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET)){
    String intervalStr = server.receiveData(ServerClient::Resource::RES_SENSOR, SENSOR_ID_TO_GET);
    if (intervalStr.length() > 0) {
      byte intervalGet = static_cast<byte>(intervalStr[0]); // Obtener el primer carácter y castear a byte
      Serial.println(intervalGet);
      if(actualInterval != intervalGet ){
        Led::State constantStateOff = Led::State::OFF;
        if(actualInterval != 0){
          server.update(ServerClient::Resource::RES_LED, static_cast<uint16_t>(wireLed.getFirstPin()+actualInterval-1), static_cast<const byte*>(static_cast<void*>(&constantStateOff)), sizeof(Led::State));
        }
        actualInterval = intervalGet;
        wireLed.setAllState(Led::State::OFF);
        wireLed.setState(wireLed.getFirstPin()+actualInterval-1, Led::State::ON);
        Led::State constantStateOn = Led::State::ON; // Variable local para ON
        server.update(ServerClient::Resource::RES_LED, static_cast<uint16_t>(wireLed.getFirstPin()+actualInterval-1), static_cast<const byte*>(static_cast<void*>(&constantStateOn)), sizeof(Led::State));
      }
    } else {
      Serial.println("No se recibieron datos de intervalo o hubo un error.");
    }
  }
}


void setup() {
  Serial.begin(115200);
  server.wifiConnect("Matheo", "cg4hts55fh4fyr7");
  server.connectToServer();
  Led::State constantStateOff = Led::State::OFF;
  server.post(ServerClient::Resource::RES_LED,static_cast<uint16_t>(blueLed.getPin()), static_cast<const byte*>(static_cast<void*>(&constantStateOff)),sizeof(Led::State));
  server.post(ServerClient::Resource::RES_LED,static_cast<uint16_t>(orangeLed.getPin()), static_cast<const byte*>(static_cast<void*>(&constantStateOff)),sizeof(Led::State));
  server.post(ServerClient::Resource::RES_LED,static_cast<uint16_t>(redLed.getPin()), static_cast<const byte*>(static_cast<void*>(&constantStateOff)),sizeof(Led::State));
}

void loop() {
  // put your main code here, to run repeatedly:
  changedInterval();
  delay(1000);
}
