#pragma once
#include <Arduino.h>

class Led {
public:
    enum class State {
        OFF,
        ON,
        BLINK
    };

    Led(uint8_t pin) 
        : m_pin(pin),
          m_currentState(State::OFF),
          m_blinkInterval(0),
          m_previousMillis(0),
          m_isOn(false) {
        pinMode(m_pin, OUTPUT);
        digitalWrite(m_pin, LOW);
    }

    uint8_t getPin() const { 
        return m_pin; 
    }

    void setBlinksPerSecond(uint16_t blinksPerSecond) {
        m_blinkInterval = (blinksPerSecond > 0) ? 1000 / blinksPerSecond : 0;
    }

    void setState(State newState) {
        m_currentState = newState;
        applyState();
    }

    void update() {
        if (m_currentState == State::BLINK && m_blinkInterval > 0) {
            handleBlink();
        }
    }

private:
    const uint8_t m_pin;
    State m_currentState;
    uint32_t m_blinkInterval;
    uint32_t m_previousMillis;
    bool m_isOn;

    void applyState() {
        switch(m_currentState) {
            case State::OFF:
                turnOff();
                break;
            case State::ON:
                turnOn();
                break;
            case State::BLINK:
                m_previousMillis = millis(); // Reset blink timer
                break;
        }
    }

    void turnOn() {
        digitalWrite(m_pin, HIGH);
        m_isOn = true;
    }

    void turnOff() {
        digitalWrite(m_pin, LOW);
        m_isOn = false;
    }

    void toggle() {
        m_isOn = !m_isOn;
        digitalWrite(m_pin, m_isOn ? HIGH : LOW);
    }

    void handleBlink() {
        uint32_t currentMillis = millis();
        if(currentMillis - m_previousMillis >= m_blinkInterval) {
            m_previousMillis = currentMillis;
            toggle();
        }
    }
};