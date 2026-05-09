#pragma once

#include <string>

// Coarse difficulty knob for how demanding the workload is.
enum class WorkloadComplexity { Low, Medium, High };

bool parse_complexity(const std::string& text, WorkloadComplexity& out);
std::string complexity_label(WorkloadComplexity c);

// What the user is trying to do. The simulator maps this to simple numeric weights.
struct WorkloadSpec {
    std::string task_type = "generic";
    WorkloadComplexity complexity = WorkloadComplexity::Medium;
    // Relative size of the job (e.g. GB processed, or an abstract "unit" count).
    double task_size = 1.0;
};

WorkloadSpec default_workload_spec();

