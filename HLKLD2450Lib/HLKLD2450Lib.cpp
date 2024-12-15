#include "HLKLD2450Lib.h"
#include <esp_heap_caps.h>

void (*HLKLD2450Lib::userCallback)(int, int16_t, int16_t) = nullptr;

HLKLD2450Lib::HLKLD2450Lib()
    : pClient(nullptr), pRemoteCharacteristic(nullptr), sensorAddress(""), connected(false) {
    memset(targetStates, 0, sizeof(targetStates));
    if (psramFound()) {
        DEBUG_PRINTLN("PSRAM found and initialized.");
        dataBuffer.reserve(2 * 1024 * 1024); // Reserve 2MB if PSRAM is available
    } else {
        DEBUG_PRINTLN("PSRAM not available, using regular memory.");
        dataBuffer.reserve(38); // Use ideal buffer size without PSRAM
    }
}

void HLKLD2450Lib::begin(const char* sensorMac) {
    sensorAddress = BLEAddress(sensorMac);
    BLEDevice::init("");
    pClient = BLEDevice::createClient();
    connectToBLE();
}

void HLKLD2450Lib::loop() {
    if (!connected) {
        connectToBLE();
        delay(5000);
    } else {
        processBuffer();
        delay(10);
    }
}

void HLKLD2450Lib::connectToBLE() {
    int retryCount = 0;
    while (!connected && retryCount < 3) {
        if (pClient->connect(sensorAddress)) {
            BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
            if (pRemoteService != nullptr) {
                pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
                if (pRemoteCharacteristic != nullptr && pRemoteCharacteristic->canNotify()) {
                    pRemoteCharacteristic->registerForNotify(notifyCallback);
                    connected = true;
                    DEBUG_PRINTLN("Successfully connected to BLE sensor.");
                }
            }
        }
        if (!connected) {
            retryCount++;
            DEBUG_PRINTLN("Failed to connect to BLE device. Retrying...");
            delay(5000 * retryCount); // Exponential backoff
        }
    }
}

void HLKLD2450Lib::notifyCallback(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    // Append incoming data to the buffer
    for (size_t i = 0; i < length; i++) {
        dataBuffer.push_back(pData[i]);
    }
}

int16_t HLKLD2450Lib::customSignedInt16(uint16_t value) {
    bool isNegative = (value & 0x8000) == 0;
    uint16_t magnitude = value & 0x7FFF;

    return isNegative ? -static_cast<int16_t>(magnitude) : static_cast<int16_t>(magnitude);
}

void HLKLD2450Lib::processBuffer() {
    size_t start = 0;
    while (start + FRAME_SIZE <= dataBuffer.size()) {
        if (dataBuffer[start] == 0xAA && dataBuffer[start + 1] == 0xFF && dataBuffer[start + 2] == 0x03 &&
            dataBuffer[start + 3] == 0x00 && dataBuffer[start + FRAME_SIZE - 2] == 0x55 && dataBuffer[start + FRAME_SIZE - 1] == 0xCC) {
            for (int target = 0; target < 3; target++) {
                int offset = start + 4 + target * 8;

                uint16_t rawX = dataBuffer[offset] | (dataBuffer[offset + 1] << 8);
                uint16_t rawY = dataBuffer[offset + 2] | (dataBuffer[offset + 3] << 8);
                uint16_t rawSpeed = dataBuffer[offset + 4] | (dataBuffer[offset + 5] << 8);
                uint16_t distanceRes = dataBuffer[offset + 6] | (dataBuffer[offset + 7] << 8);

                int16_t x = customSignedInt16(rawX);
                int16_t y = customSignedInt16(rawY);
                int16_t speed = customSignedInt16(rawSpeed);

                if (x == 0 && y == 0 && speed == 0) {
                    targetStates[target].zeroCount++;
                } else {
                    targetStates[target].zeroCount = 0;
                }

                targetStates[target] = {x, y, speed, distanceRes, targetStates[target].zeroCount};

                if (targetStates[target].zeroCount < 10) {
                    if (userCallback) {
                        userCallback(target + 1, x, y);
                    }
                }
            }

            dataBuffer.erase(dataBuffer.begin() + start, dataBuffer.begin() + start + FRAME_SIZE);
        } else {
            start++;
        }
    }
    manageBuffer();
}

void HLKLD2450Lib::manageBuffer() {
    if (dataBuffer.size() > 1000) {
        dataBuffer.erase(dataBuffer.begin(), dataBuffer.begin() + 500);
    }
}

void HLKLD2450Lib::registerCallback(void (*callback)(int, int16_t, int16_t)) {
    userCallback = callback;
}

bool HLKLD2450Lib::isConnected() {
    return connected;
}