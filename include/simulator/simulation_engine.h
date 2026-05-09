#pragma once

#include "hardware/hardware_config.h"
#include "simulator/simulation_result.h"
#include "workload/workload_spec.h"

// Runs a very small analytic model: no threads, no real timing — just estimates.
SimulationResult simulate(const HardwareConfig& hw, const WorkloadSpec& workload);
