#pragma once

#include "hardware/hardware_config.h"
#include "workload/workload_spec.h"

#include <string>

// Loads simple key=value lines from a text file (optional for MVP).
// Unknown keys are ignored; missing keys keep the passed-in defaults.
bool load_happ_config(const std::string& path, HardwareConfig& hw, WorkloadSpec& workload,
                      std::string& error_message);

// Writes the current hw/workload pair in the same format.
bool save_happ_config(const std::string& path, const HardwareConfig& hw,
                      const WorkloadSpec& workload, std::string& error_message);
