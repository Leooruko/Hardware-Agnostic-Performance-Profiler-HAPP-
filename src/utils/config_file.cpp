#include "utils/config_file.h"

#include "hardware/storage_type.h"
#include "workload/workload_spec.h"

#include <cctype>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

std::string to_lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

}  // namespace

bool load_happ_config(const std::string& path, HardwareConfig& hw, WorkloadSpec& workload,
                      std::string& error_message) {
    std::ifstream in(path);
    if (!in) {
        error_message = "Could not open config file: " + path;
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = to_lower(trim(line.substr(0, eq)));
        std::string val = trim(line.substr(eq + 1));

        try {
            if (key == "cpu" || key == "cores" || key == "cpu_cores") {
                hw.cpu_cores = std::stoi(val);
            } else if (key == "ram" || key == "ram_gb" || key == "memory") {
                hw.ram_gb = std::stod(val);
            } else if (key == "storage") {
                StorageType st{};
                if (!parse_storage_type(val, st)) {
                    error_message = "Unknown storage type in config: " + val;
                    return false;
                }
                hw.storage = st;
            } else if (key == "network" || key == "network_mbps" || key == "net") {
                hw.network_mbps = std::stod(val);
            } else if (key == "label" || key == "name") {
                hw.label = val;
            } else if (key == "task" || key == "task_type") {
                workload.task_type = val;
            } else if (key == "complexity") {
                WorkloadComplexity c{};
                if (!parse_complexity(val, c)) {
                    error_message = "Unknown complexity in config: " + val;
                    return false;
                }
                workload.complexity = c;
            } else if (key == "size" || key == "task_size") {
                workload.task_size = std::stod(val);
            }
        } catch (const std::exception&) {
            error_message = "Bad numeric value for key '" + key + "': " + val;
            return false;
        }
    }

    return true;
}

bool save_happ_config(const std::string& path, const HardwareConfig& hw,
                      const WorkloadSpec& workload, std::string& error_message) {
    std::ofstream out(path);
    if (!out) {
        error_message = "Could not write config file: " + path;
        return false;
    }

    out << "# HAPP configuration (key=value). Lines starting with # are comments.\n";
    out << "cpu=" << hw.cpu_cores << "\n";
    out << "ram=" << hw.ram_gb << "\n";
    out << "storage=" << storage_type_label(hw.storage) << "\n";
    out << "network=" << hw.network_mbps << "\n";
    if (!hw.label.empty()) {
        out << "label=" << hw.label << "\n";
    }
    out << "task=" << workload.task_type << "\n";
    out << "complexity=" << complexity_label(workload.complexity) << "\n";
    out << "size=" << workload.task_size << "\n";
    return true;
}
