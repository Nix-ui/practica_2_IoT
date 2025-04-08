#pragma once

#include <Arduino.h>

class UltraSonicSensor {
private:
    byte triggerPin;
    byte echoPin;

    // Constante para la conversión de microsegundos a centímetros (ajustada para int)
    static constexpr int SOUND_SPEED_CM_US = 17; // Aproximación entera

    // Realiza la medición del tiempo de eco en microsegundos
    int measureEchoTime() {
        digitalWrite(triggerPin, LOW);
        delayMicroseconds(2);
        digitalWrite(triggerPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(triggerPin, LOW);
        return pulseIn(echoPin, HIGH);
    }

public:
    // Constructor de la clase UltraSonicSensor
    UltraSonicSensor(byte triggerPin, byte echoPin) : triggerPin(triggerPin), echoPin(echoPin) {
        pinMode(triggerPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    // Obtiene la distancia en microsegundos (entero)
    int getDistanceUs() {
        return measureEchoTime();
    }

    // Obtiene la distancia en centímetros (entero)
    int getDistanceCm() {
        int echoTime = measureEchoTime();
        if (echoTime == 0) {
            return -1; // Indica un error
        }
        return (echoTime * SOUND_SPEED_CM_US) / 1000; // Ajuste para obtener cm
    }
};