# Practica 2 IoT
<style>
  img{
    background-color: #fff;
  }
</style>
## Integrantes
- Rocha
- Quiroga
- La Torre
  
  ## Indice
- [Practica 2 IoT](#practica-2-iot)
  - [Integrantes](#integrantes)
  - [Indice](#indice)
    - [Descripción](#descripción)
    - [Objetivos](#objetivos)
    - [Protocolo de Aplicacion](#protocolo-de-aplicacion)
    - [Materiales](#materiales)
    - [Diagramas](#diagramas)



  ### Descripción

### Objetivos

### Protocolo de Aplicacion
- Comandos
  - GET ITERVALS
    - Arguments:None
    - Return:
      - 0,0,10
      - 1,10,20
      - 2,20,30
      - -1

### Materiales
- ESP32(30pins)
  - ![ESP32(30pins)](img/esp32_30pins.svg)
- ESP32(38pins)
  - ![ESP32(38pins)](img/esp32_38pins.svg)
- LED
  - ![RED LED](img/redLed.svg) ![GREEN LED](img/greenLed.svg) ![BLUE LED](img/blueLed.svg)
- Resistor 220$\Omega$
  - ![Resistor](img/resistor_220_omn.svg)
- UltraSonicSensor
  - ![UltraSonicSensor](img/ultrasonicSensor.svg)


  ### Diagramas
- Diagrama del prototipo
    - ![Diagramado del prototipo](img/prototype.svg)
- Diagrama de arquitectura
  - ![Diagrama de arquitectura](img/archectitureDiagram.svg)

- Diagrama de Circuitos
  - ![Diagrama](img/project_esquemático.svg)
- Diagrama de Clases
  - ESP30 Pins (Led Controller)
    - ```mermaid
         %%{init: {'theme':'neutral'}}%%
          classDiagram
            direction RL
            class Led{
              -ledPin : byte
              -actualState : bool 
              -finalState : byte
              -previousMillis : unsigned long 
              -interval : unsigned long
              +STATE_ON : byte$
              +STATE_OFF : byte$
              +STATE_BLINK : byte$
              -turnOn() void
              -turnOff() void
              -blink() void
              +setBlinksPerSecond(byte bps) void
              +setState(byte state) void
              }
            Controller <-- LedController
            LedController --> Led
            Controller : +setState() virtual void
            Controller: + setAllStates() virtual void
            class LedController{
              -leds : Led[]
              +setAllStates(byte state) void
              +setState(byte ledIndex, byte state) void
            }      
      ``` 
    - ```mermaid
         %%{init: {'theme':'neutral'}}%%
                classDiagram
                    direction RL
                    class UltraSonicSensor{
                      -trigerPin : byte
                      -echoPin : byte
                      -getDistance() long
                      +getDistanceCm() int
                    }
        ```
  - ESP38 Pins (UltraSonicSensor)
    - ```mermaid
           %%{init: {'theme':'neutral'}}%%
                classDiagram
                    direction RL
                    class UltraSonicSensor{
                      -trigerPin : byte
                      -echoPin : byte
                      -getDistance() long
                      +getDistanceCm() int
                    }
      ```
    - ```mermaid
           %%{init: {'theme':'neutral'}}%%
                classDiagram
                    direction RL
                    class SensorExecute{
                      -mySensor : UltraSonicSensor
                      -client : ServerClient
                      + setup() void
                      + loop() void
                    }
                    SensorExecute <--UltraSonicSensor
                    SensorExecute <--ServerClient
      ```
  - ServerClient
    - ```mermaid
           %%{init: {'theme':'neutral'}}%%
                classDiagram
                    direction RL
                    class ServerClient{
                      +Resource : enum
                      +Command : enum
                      +Status : enum
                      -m_host : const char
                      -m_port : unit_16
                      -isConnected : bool
                      +wifiConnect((const char*, const char* ) bool
                      +connectToServer() bool
                      +get(Resource, uint16_t) bool
                      +post(Resource, uint16_t, const uint8_t*, uint32_t) bool
                      +update(Resource, uint16_t, const uint8_t*, uint32_t) bool
                      +disconnectToServer() bool
                      +isConnected() bool
                      +reciveStatus() bool
                      +reciveData(Resource, uint16_t, char*, size_t) bool
                      -sendPacket(byte,byte,uint16_t,uint8_t,uint32_t) bool
                    }

      ```
- Diagrama de comportamiento
  - ```mermaid
    %%{init: {'theme':'dark'}}%%
             sequenceDiagram
                Participant A as SERVER
                Note left of A: Conect to wifi
                Note left of A: Server start
                Participant B as ESP-32(SensorExecute)
                Note left of B: Conect to wifi
                Participant C as UltraSonicSensor
                Participant D as ESP-32(LedExecute)
                Note left of D: Conect to wifi
                Participant E as LedController
                Note over  E: if(interval == 0) setAllLeds(STATE_ON);
                Note over  E: if(interval == 1) setAllLeds(STATE_OFF) <br> setLed(redLedPin,STATE_ON)
                Note over  E: if(interval == 2) setAllLeds(STATE_OFF) <br>setLed(redLedPin,STATE_ON)
                Note over  E: if(interval == -1) setAllLeds(STATE_OFF); 
                Participant F as blueLed
                Participant G as orangeLed
                Participant H as redLed
                B->>+A:connectToServer()
                A-->>-B:conectionStatus
                B->>+A:POST : SENSOR, SENSOR_ID ACTUAL_INTERVAL sizeof(ACTUALINTERVAL)
                A-->>-B:RESPONSE: STATUS
                loop when Interval changed
                    loop when distance change
                        C->>+B:getDistance()
                        B-->>-C:theDistance change the interval?
                        alt True
                            B-->>+B:change Interval
                        end
                    end
                    B->>+A: UPDATE : SENSOR, SENSOR_ID ACTUAL_INTERVAL sizeof(ACTUALINTERVAL)
                    A-->>-B:RESPONSE: STATUS
                end
                D->>+A: POST : LED_RES, blueLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                D->>A: POST : LED_RES, orangeLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                D->>A: POST : LED_RES, redLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                A-->>-D:RESPONSE : STATUS
                loop when timeout pass
                    D->>+A: GET : RES_SENSOR, SENSOR_ID, responseVariable, sizeof(RESPONSE)
                    A-->>-D: RESPONSE:responseVariable = ACTUAL_INTERVAL
                end
                alt ACTUAL_INTERVAL == 0
                    D->>+E:setAllLeds(STATE_OFF)
                    E->>+F:turnOff()
                    E->>+G:turnOf()
                    E->>+H:turnOf()
                    E-->>-D: RESPONSE: STATUS
                    D->>+A: UPDATE : LED_RES, blueLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, orangeLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, redLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    A-->>-D: RESPONSE: STATUS
                else ACTUAL_INTERVAL == 1
                    D->>+E:setAllState(STATE_OFF) <br>setState(blueLedPin, STATE_ON)
                    E->>+F:turnOff()
                    E->>+G:turnOff()
                    E->>+H:turnOff()
                    E->>+F:turnOn()
                    E-->>-D: RESPONSE: STATUS
                    D->>+A: UPDATE : LED_RES, blueLedPin, Led::STATE_ON,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, orangeLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, redLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    A-->>-D: RESPONSE: STATUS
                else ACTUAL_INTERVAL == 2
                    D->>+E:setAllState(STATE_OFF) <br>setState(orangeLedPin, STATE_ON)
                    E->>+F:turnOff()
                    E->>+G:turnOff()
                    E->>+H:turnOff()
                    E->>G:turnOn()
                    E-->>-D: RESPONSE: STATUS
                    D->>+A: UPDATE : LED_RES, blueLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, orangeLedPin, Led::STATE_ON,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, redLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    A-->>-D: RESPONSE: STATUS
                else ACTUAL_INTERVAL == 3
                    D->>+E:setAllState(STATE_OFF) <br>setState(redLedPin, STATE_ON)
                    E->>+F:turnOff()
                    E->>+G:turnOff()
                    E->>+H:turnOff()
                    E->>H:turnOn()
                    E-->>-D: RESPONSE: STATUS
                    D->>+A: UPDATE : LED_RES, blueLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, orangeLedPin, Led::STATE_OFF,sizeof(Led::STATE)
                    D->>A: UPDATE : LED_RES, redLedPin, Led::STATE_ON,sizeof(Led::STATE)
                    A-->>-D: RESPONSE: STATUS
                end
    ```