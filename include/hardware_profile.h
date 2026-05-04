#pragma once
#include <string>
#include <cstdint>

using namespace std;

struct HardwareProfile{
    string name;
    uint32_t frequency_hz;
    uint32_t ram_bytes;

    void print() const;
};

