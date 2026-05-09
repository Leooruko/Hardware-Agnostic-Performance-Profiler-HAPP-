#include "simulator/simulation_result.h"

#include <iomanip>
#include <iostream>

void print_simulation_report(const SimulationResult& r, const std::string& title) {
    if (!title.empty()) {
        std::cout << "\n=== " << title << " ===\n";
    } else {
        std::cout << "\n=== Simulation result ===\n";
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Estimated execution time: " << r.execution_time_sec << " s\n";
    std::cout << "Estimated CPU usage:      " << r.cpu_usage_percent << " %\n";
    std::cout << "Estimated memory usage:   " << r.memory_usage_gb << " GiB\n";
    std::cout << "Primary bottleneck:       " << r.primary_bottleneck << "\n";

    if (!r.warnings.empty()) {
        std::cout << "\nWarnings:\n";
        for (const auto& w : r.warnings) {
            std::cout << "  - " << w << "\n";
        }
    }

    if (!r.recommendations.empty()) {
        std::cout << "\nRecommendations:\n";
        for (const auto& rec : r.recommendations) {
            std::cout << "  - " << rec << "\n";
        }
    }

    std::cout << "\n(Components: compute " << r.compute_component_sec << " s, I/O "
              << r.io_component_sec << " s, network " << r.network_component_sec << " s)\n";
}
