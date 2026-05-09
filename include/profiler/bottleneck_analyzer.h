#pragma once

#include "hardware/hardware_config.h"
#include "simulator/simulation_result.h"
#include "workload/workload_spec.h"

// Fills bottleneck string, warnings, and recommendations from rough utilization math.
// Keeps the rules in one place so the simulator stays easy to read.
void analyze_bottlenecks(const HardwareConfig& hw, const WorkloadSpec& workload,
                         SimulationResult& io);
