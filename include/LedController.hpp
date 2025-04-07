#pragma once
#include "Controller.hpp"
#include "Led.hpp"

class LedController : public Controller<Led::State, uint8_t> {
public:
    LedController(Led* leds, uint8_t numLeds, uint8_t firstPin)
        : m_leds(leds),
          m_numLeds(numLeds),
          m_firstPin(firstPin) {
    }

    void setAllState(Led::State state) override {
        for(uint8_t i = 0; i < m_numLeds; ++i) {
            m_leds[i].setState(state);
        }
    }

    void setState(uint8_t targetPin, Led::State state) override {
        for(uint8_t i = 0; i < m_numLeds; ++i) {
            if(m_leds[i].getPin() == targetPin) {
                m_leds[i].setState(state);
                return;
            }
        }
        Serial.print("Pin ");
        Serial.print(targetPin);
        Serial.println(" no encontrado en el controlador");
    }

    void updateAll() {
        for(uint8_t i = 0; i < m_numLeds; ++i) {
            m_leds[i].update();
        }
    }

    uint8_t getLedPin(uint8_t index) const {
        if(index < m_numLeds) {
            return m_leds[index].getPin();
        }
        return 0;
    }

    uint8_t getLedCount() const {
        return m_numLeds;
    }

    uint8_t getFirstPin() const {
        return m_firstPin;
    }

private:
    Led* m_leds;
    const uint8_t m_numLeds;
    const uint8_t m_firstPin;
};