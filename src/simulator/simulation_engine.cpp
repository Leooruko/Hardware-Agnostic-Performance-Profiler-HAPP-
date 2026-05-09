#include "simulator/simulation_engine.h"

#include "profiler/bottleneck_analyzer.h"
#include "hardware/storage_type.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace {

struct TaskWeights {
    double cpu;
    double mem_per_size;
    double io;
    double net;
};

TaskWeights weights_for_task(const std::string& task_type) {
    std::string t = task_type;
    std::transform(t.begin(), t.end(), t.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (t == "rendering" || t == "render") {
        return {1.2, 1.4, 0.4, 0.1};
    }
    if (t == "compiling" || t == "compile" || t == "build") {
        return {1.4, 0.6, 0.5, 0.1};
    }
    if (t == "database" || t == "db") {
        return {0.9, 0.8, 1.2, 0.2};
    }
    if (t == "streaming" || t == "download" || t == "upload") {
        return {0.4, 0.3, 0.3, 1.5};
    }
    return {1.0, 0.8, 0.6, 0.3};
}

double complexity_multiplier(WorkloadComplexity c) {
    switch (c) {
        case WorkloadComplexity::Low:
            return 0.6;
        case WorkloadComplexity::Medium:
            return 1.0;
        case WorkloadComplexity::High:
            return 1.6;
    }
    return 1.0;
}

// Maps storage class to extra seconds per "I/O unit" in the model.
double storage_latency_scale(StorageType s) {
    switch (s) {
        case StorageType::HDD:
            return 1.45;
        case StorageType::SSD:
            return 1.0;
        case StorageType::NVMe:
            return 0.82;
    }
    return 1.0;
}

}  // namespace

SimulationResult simulate(const HardwareConfig& hw, const WorkloadSpec& workload) {
    SimulationResult r;
    const TaskWeights w = weights_for_task(workload.task_type);
    const double cm = complexity_multiplier(workload.complexity);
    const double size = std::max(0.01, workload.task_size);

    const int cores = std::max(1, hw.cpu_cores);
    const double ram = std::max(0.5, hw.ram_gb);

    // Baseline "reference" machine: 1 core, 8 GiB, SSD, 500 Mb/s — keeps numbers sane.
    constexpr double kSecondsPerUnit = 2.0;

    // Compute time falls with more cores, but not perfectly (simple sub-linear scaling).
    const double parallel_factor = 1.0 / std::pow(static_cast<double>(cores), 0.72);
    r.compute_component_sec = kSecondsPerUnit * w.cpu * cm * size * parallel_factor;

    r.io_component_sec =
        kSecondsPerUnit * 0.55 * w.io * cm * size * storage_latency_scale(hw.storage);

    // Network: scale inversely with Mbps; 500 Mb/s is the "1.0x" reference.
    const double net_scale = 500.0 / std::max(1.0, hw.network_mbps);
    r.network_component_sec = kSecondsPerUnit * 0.45 * w.net * cm * size * net_scale;

    // Total time: take the dominant path lightly blended so all three matter a little.
    const double dominant = std::max({r.compute_component_sec, r.io_component_sec, r.network_component_sec});
    const double sum = r.compute_component_sec + r.io_component_sec + r.network_component_sec;
    r.execution_time_sec = 0.65 * dominant + 0.35 * (sum / 3.0);

    // If memory is undersized, add a penalty to mimic paging / thrashing.
    const double mem_need = w.mem_per_size * size * cm;
    if (mem_need > ram) {
        const double pressure = (mem_need - ram) / std::max(0.1, ram);
        r.execution_time_sec *= (1.0 + 0.35 * pressure);
    }

    // CPU%: high when compute dominates; cap at 100.
    const double compute_share = r.compute_component_sec / std::max(1e-6, sum);
    r.cpu_usage_percent = std::min(100.0, 35.0 + 65.0 * compute_share);

    r.memory_usage_gb = std::min(mem_need, ram * 1.05);

    analyze_bottlenecks(hw, workload, r);
    return r;
}
