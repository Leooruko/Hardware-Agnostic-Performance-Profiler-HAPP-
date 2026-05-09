#pragma once

#include "hardware/storage_type.h"

#include <string>

// Describes a machine the user wants to simulate. Values are intentionally simple
// so beginners can map flags directly to fields.
struct HardwareConfig {
    int cpu_cores = 1;
    double ram_gb = 4.0;
    StorageType storage = StorageType::SSD;
    // Effective throughput for large transfers (Mb/s). Used for network-bound work.
    double network_mbps = 100.0;

    std::string label;

    void print_summary() const;
};

// Baseline used before CLI flags / config files (and for each `--hw` compare entry).
HardwareConfig default_hardware_config();

