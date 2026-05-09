#include "hardware/hardware_config.h"

#include <iostream>

HardwareConfig default_hardware_config() {
    HardwareConfig h;
    h.cpu_cores = 4;
    h.ram_gb = 8.0;
    h.storage = StorageType::SSD;
    h.network_mbps = 100.0;
    return h;
}

void HardwareConfig::print_summary() const {
    std::cout << "Hardware";
    if (!label.empty()) {
        std::cout << " [" << label << "]";
    }
    std::cout << ": " << cpu_cores << " cores, " << ram_gb << " GiB RAM, "
              << storage_type_label(storage) << ", network " << network_mbps << " Mb/s\n";
}
