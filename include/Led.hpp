#pragma once

class Led {
public:
    enum State {
        OFF,
        ON,
        BLINK
    };

private:
    byte pin;
    bool isOn;
    State finalState;
    unsigned long previousMillis;
    unsigned long blinkInterval;

    // Cambia el estado del LED (ON/OFF)
    void toggleState() {
        isOn = !isOn;
        digitalWrite(pin, isOn ? HIGH : LOW);
    }

    // Maneja la lógica de parpadeo
    void handleBlink() {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            toggleState();
        }
    }

public:
    // Constructor de la clase Led
    Led(byte pin) : pin(pin), isOn(false), finalState(State::OFF), previousMillis(0), blinkInterval(0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // Asegura que el LED comienza apagado
    }

    // Establece la velocidad de parpadeo en parpadeos por segundo
    void setBlinksPerSecond(unsigned long blinksPerSecond) {
        if (blinksPerSecond > 0) {
            blinkInterval = 1000 / blinksPerSecond;
        } else {
            blinkInterval = 0; // Desactiva el parpadeo si blinksPerSecond es cero
        }
    }

    // Enciende el LED
    void turnOn() {
        digitalWrite(pin, HIGH);
        isOn = true;
    }

    // Apaga el LED
    void turnOff() {
        digitalWrite(pin, LOW);
        isOn = false;
    }

    // Cambia el estado del LED (ON/OFF)
    void toggle() {
        toggleState();
    }

    // Establece el estado del LED (ON, OFF, BLINK)
    void setState(State state) {
        finalState = state;
        switch (finalState) {
            case State::OFF:
                turnOff();
                break;
            case State::ON:
                turnOn();
                break;
            case State::BLINK:
                break; // No llama a blink() aquí, se llama en el loop.
        }
    }

    // Actualiza el estado del LED (debe llamarse en el loop())
    void update() {
        if (finalState == State::BLINK) {
            handleBlink();
        }
    }
};