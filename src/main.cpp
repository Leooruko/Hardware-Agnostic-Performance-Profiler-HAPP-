#include <iostream>
#include "hardware_profile.h"
#include "hardware_profiles.h"
using namespace std;

int main(){
    HardwareProfile profile = createESP32();
    profile.print();

    return 0;
}