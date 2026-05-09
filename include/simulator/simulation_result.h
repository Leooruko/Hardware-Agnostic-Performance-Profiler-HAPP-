#pragma once

#include <string>
#include <vector>

// Everything we show the user after a run. Numbers are estimates, not measurements.
struct SimulationResult {
    double execution_time_sec = 0.0;
    double cpu_usage_percent = 0.0;
    double memory_usage_gb = 0.0;

    // Human-readable bottleneck: "CPU", "RAM", "Storage I/O", "Network", "Balanced", ...
    std::string primary_bottleneck;

    std::vector<std::string> warnings;
    std::vector<std::string> recommendations;

    // Extra transparency for learning/debugging.
    double compute_component_sec = 0.0;
    double io_component_sec = 0.0;
    double network_component_sec = 0.0;
};

void print_simulation_report(const SimulationResult& r, const std::string& title = {});
