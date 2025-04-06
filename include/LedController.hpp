#pragma once
#include "Controller.hpp"
#include "Led.hpp"
class LedController : public Controller {
    public:
        LedController(Led* leds, byte numLeds,byte firstPin) 
            : m_leds(leds), 
              m_numLeds(numLeds),
              m_firstPin(firstPin) {}
        
        void setAllState(Led::State state) override {
            for (byte i = 0; i < m_numLeds; ++i) {
                m_leds[i].setState(state);
            }
        }
    
        void setState(byte ledPin, Led::State state) override {
            for (byte i = 0; i < m_numLeds; ++i) {
                if (m_leds[i].getPin() == ledPin) {
                    m_leds[i].setState(state);
                    return;
                }
            }
            // Opcional: manejo de error si el pin no existe
        }
        byte getFirstPin(){
          return m_firstPin;
        }
    private:
        Led* m_leds;
        byte m_numLeds;
        byte m_firstPin;
    };