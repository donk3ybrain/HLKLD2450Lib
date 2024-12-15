#include <Arduino.h>
#include "HLKLD2450Lib.h"

#define LD2450_MAC "AA:BB:CC:DD:EE:FF" // Replace with your sensors MAC Address
#define RX1 18
#define TX1 17

HLKLD2450Lib bleClient;

HardwareSerial UARTout(1); // Use Serial1 for Hardware Serial communication

void setup() {
    Serial.begin(115200);
    UARTout.begin(256000, SERIAL_8N1, RX1, TX1);
    bleClient.begin(LD2450_MAC);
    bleClient.registerCallback([](int target, int16_t x, int16_t y) {
        // Prints target number, x, and y coordinates to Serial
        Serial.print(target);
        Serial.print(", ");
        Serial.print(x);
        Serial.print(", ");
        Serial.println(y);

        // Sends target number, x, and y coordinates out via Serial1
        UARTout.print(target);
        UARTout.print(", ");
        UARTout.print(x);
        UARTout.print(", ");
        UARTout.println(y);
    });
}

void loop() {
    bleClient.loop();
    if (!bleClient.isConnected()) {
        Serial.println("Reconnecting...");
    }
}
