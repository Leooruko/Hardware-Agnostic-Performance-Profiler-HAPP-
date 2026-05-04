#include "hardware_profile.h"
#include <iostream>


void HardwareProfile::print() const{
    cout << "=== Hardware Profile ===\n";
    cout << "Name: " << name << "\n";
    cout << "CPU: " << frequency_hz / 1e6 << " MHz\n";
    cout << "RAM: " << ram_bytes / 1024 << " KB\n";
    cout << "===========================================\n";
}    

