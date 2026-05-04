#include "hardware_profiles.h"

HardwareProfile createESP32() {
    return {"ESP32", 80000000, 512 * 1024};
}

HardwareProfile createRaspberryPiZero() {
    return {"Raspberry Pi Zero", 1000000000, 512 * 1024 * 1024};
}