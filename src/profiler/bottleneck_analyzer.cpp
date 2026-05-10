#include "profiler/bottleneck_analyzer.h"

#include "hardware/storage_type.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace {

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

// Same task weights as the simulator uses — duplicated lightly for explanation text.
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
    // generic
    return {1.0, 0.8, 0.6, 0.3};
}

}  // namespace

void analyze_bottlenecks(const HardwareConfig& hw, const WorkloadSpec& workload,
                         SimulationResult& r) {
    const TaskWeights w = weights_for_task(workload.task_type);
    const double cm = complexity_multiplier(workload.complexity);
    const double size = std::max(0.01, workload.task_size);

    const double mem_need_gb = w.mem_per_size * size * cm;
    const double ram_capacity = std::max(0.5, hw.ram_gb);

    // Use the same time components the simulator already computed so labels match the model.
    const double c = r.compute_component_sec;
    const double io = r.io_component_sec;
    const double n = r.network_component_sec;

    if (mem_need_gb > ram_capacity * 1.0) {
        r.primary_bottleneck = "RAM (memory capacity)";
    } else if (c >= io && c >= n) {
        r.primary_bottleneck = "CPU (compute)";
    } else if (io >= c && io >= n) {
        r.primary_bottleneck = "Storage I/O";
    } else {
        r.primary_bottleneck = "Network";
    }

    if (mem_need_gb > ram_capacity * 1.0) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(2);
        oss << "Workload may need ~" << mem_need_gb << " GiB RAM but only " << ram_capacity
            << " GiB is configured (risk of swapping or OOM).";
        r.warnings.push_back(oss.str());
        r.recommendations.push_back("Add RAM or reduce task size / complexity.");
    }

    std::string task_lower = workload.task_type;
    std::transform(task_lower.begin(), task_lower.end(), task_lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (hw.cpu_cores < 2 &&
        (task_lower == "rendering" || task_lower == "render" || task_lower == "compiling" ||
         task_lower == "compile" || task_lower == "build")) {
        r.warnings.push_back("Only one CPU core - parallel stages will queue.");
    }

    if (hw.storage == StorageType::HDD && w.io > 0.8) {
        r.warnings.push_back("HDD may slow I/O-heavy phases of this workload.");
        r.recommendations.push_back("Consider SSD/NVMe if storage-bound.");
    }

    if (w.net > 1.0 && hw.network_mbps < 50.0) {
        r.warnings.push_back("Network throughput looks low for a network-heavy task.");
        r.recommendations.push_back("Increase link speed or shrink transferred data.");
    }

    if (r.primary_bottleneck.find("CPU") != std::string::npos) {
        r.recommendations.push_back("More CPU cores or a faster per-core tier helps most.");
    } else if (r.primary_bottleneck.find("RAM") != std::string::npos) {
        r.recommendations.push_back("More RAM is the first lever to pull.");
    } else if (r.primary_bottleneck.find("Storage") != std::string::npos) {
        r.recommendations.push_back("Faster storage (SSD/NVMe) reduces I/O wait.");
    } else if (r.primary_bottleneck.find("Network") != std::string::npos) {
        r.recommendations.push_back("Faster network or batching/compression reduces transfer time.");
    }

    // De-duplicate recommendations while preserving order.
    auto dedupe = [](std::vector<std::string>& v) {
        std::vector<std::string> out;
        for (const auto& s : v) {
            if (std::find(out.begin(), out.end(), s) == out.end()) {
                out.push_back(s);
            }
        }
        v.swap(out);
    };
    dedupe(r.recommendations);
}
