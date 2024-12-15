#ifndef HLKLD2450LIB_H
#define HLKLD2450LIB_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>
#include <vector>

// Define the BLE service and characteristic UUIDs
#define SERVICE_UUID "0000fff0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000fff1-0000-1000-8000-00805f9b34fb"

// Debugging macros
#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

class HLKLD2450Lib {
public:
    HLKLD2450Lib();
    void begin(const char* sensorMac);
    void loop();
    void registerCallback(void (*callback)(int, int16_t, int16_t));
    bool isConnected();

private:
    void connectToBLE();
    static void notifyCallback(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    void processBuffer();
    void manageBuffer();
    bool initializePSRAM();
    static int16_t customSignedInt16(uint16_t value);

    BLEClient* pClient;
    BLERemoteCharacteristic* pRemoteCharacteristic;
    BLEAddress sensorAddress;
    bool connected;
    std::vector<uint8_t, PSRAMAllocator<uint8_t>> dataBuffer; // Use PSRAM if available
    struct TargetState {
        int16_t x;
        int16_t y;
        int16_t speed;
        uint16_t distanceRes;
        int zeroCount;
    };
    TargetState targetStates[3];
    static void (*userCallback)(int, int16_t, int16_t);
};

#endif
