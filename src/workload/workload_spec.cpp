#include "workload/workload_spec.h"

#include <algorithm>
#include <cctype>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool parse_complexity(const std::string& text, WorkloadComplexity& out) {
    const std::string k = to_lower(text);
    if (k == "low" || k == "l") {
        out = WorkloadComplexity::Low;
        return true;
    }
    if (k == "medium" || k == "med" || k == "m") {
        out = WorkloadComplexity::Medium;
        return true;
    }
    if (k == "high" || k == "h") {
        out = WorkloadComplexity::High;
        return true;
    }
    return false;
}

WorkloadSpec default_workload_spec() {
    WorkloadSpec w;
    w.task_type = "generic";
    w.complexity = WorkloadComplexity::Medium;
    w.task_size = 1.0;
    return w;
}

std::string complexity_label(WorkloadComplexity c) {
    switch (c) {
        case WorkloadComplexity::Low:
            return "low";
        case WorkloadComplexity::Medium:
            return "medium";
        case WorkloadComplexity::High:
            return "high";
    }
    return "medium";
}
