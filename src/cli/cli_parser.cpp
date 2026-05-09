#include "cli/cli_parser.h"

#include "hardware/hardware_config.h"
#include "hardware/storage_type.h"
#include "utils/config_file.h"

#include <cstring>
#include <exception>
#include <sstream>

namespace {

bool starts_with(const char* s, const char* prefix) {
    return s && prefix && std::strncmp(s, prefix, std::strlen(prefix)) == 0;
}

std::string flag_name(const char* arg) {
    std::string a = arg;
    if (a.rfind("--", 0) == 0) {
        a = a.substr(2);
    }
    const auto eq = a.find('=');
    if (eq != std::string::npos) {
        a = a.substr(0, eq);
    }
    return a;
}

// Supports `--key value` and `--key=value`.
bool take_value(int& i, int argc, char** argv, const char* arg, std::string& value_out,
                std::string& err) {
    std::string a = arg;
    const auto eq = a.find('=');
    if (eq != std::string::npos) {
        value_out = a.substr(eq + 1);
        return true;
    }
    if (i + 1 >= argc) {
        err = std::string("Missing value after ") + arg;
        return false;
    }
    value_out = argv[++i];
    return true;
}

bool parse_hw_spec(const std::string& text, HardwareConfig& hw, std::string& err) {
    HardwareConfig tmp = hw;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        const auto eq = item.find('=');
        if (eq == std::string::npos) {
            err = "Bad --hw fragment (expected key=value): " + item;
            return false;
        }
        std::string key = item.substr(0, eq);
        std::string val = item.substr(eq + 1);
        // Trim spaces lightly.
        while (!key.empty() && key.front() == ' ') {
            key.erase(key.begin());
        }
        while (!key.empty() && key.back() == ' ') {
            key.pop_back();
        }
        while (!val.empty() && val.front() == ' ') {
            val.erase(val.begin());
        }
        while (!val.empty() && val.back() == ' ') {
            val.pop_back();
        }

        for (char& c : key) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        try {
            if (key == "cpu" || key == "cores") {
                tmp.cpu_cores = std::stoi(val);
            } else if (key == "ram" || key == "memory") {
                tmp.ram_gb = std::stod(val);
            } else if (key == "storage") {
                StorageType st{};
                if (!parse_storage_type(val, st)) {
                    err = "Unknown storage in --hw: " + val;
                    return false;
                }
                tmp.storage = st;
            } else if (key == "network" || key == "net") {
                tmp.network_mbps = std::stod(val);
            } else if (key == "label" || key == "name") {
                tmp.label = val;
            } else {
                err = "Unknown key in --hw: " + key;
                return false;
            }
        } catch (const std::exception&) {
            err = "Bad numeric value in --hw for " + key + ": " + val;
            return false;
        }
    }
    hw = tmp;
    return true;
}

}  // namespace

bool parse_cli(int argc, char** argv, CliOptions& out) {
    if (argc < 2) {
        out.command = CliCommand::Help;
        return true;
    }

    const char* sub = argv[1];
    if (std::strcmp(sub, "help") == 0 || std::strcmp(sub, "--help") == 0 ||
        std::strcmp(sub, "-h") == 0) {
        out.command = CliCommand::Help;
        return true;
    }

    if (std::strcmp(sub, "simulate") == 0) {
        out.command = CliCommand::Simulate;
    } else if (std::strcmp(sub, "compare") == 0) {
        out.command = CliCommand::Compare;
    } else if (std::strcmp(sub, "save") == 0) {
        out.command = CliCommand::Save;
    } else {
        out.error = std::string("Unknown command: ") + sub;
        return false;
    }

    int i = 2;
    if (out.command == CliCommand::Save) {
        if (i >= argc || starts_with(argv[i], "-")) {
            out.error = "Usage: happ save <file> [options]";
            return false;
        }
        out.save_path = argv[i++];
    }

    while (i < argc) {
        const char* arg = argv[i];
        if (!starts_with(arg, "-")) {
            out.error = std::string("Unexpected argument: ") + arg;
            return false;
        }

        const std::string name = flag_name(arg);
        std::string val;

        if (name == "config") {
            if (!take_value(i, argc, argv, arg, val, out.error)) {
                return false;
            }
            if (!load_happ_config(val, out.hardware, out.workload, out.error)) {
                return false;
            }
            ++i;
            continue;
        }

        if (name == "hw") {
            if (!take_value(i, argc, argv, arg, val, out.error)) {
                return false;
            }
            HardwareConfig piece = default_hardware_config();
            if (!parse_hw_spec(val, piece, out.error)) {
                return false;
            }
            out.compare_hardware.push_back(piece);
            ++i;
            continue;
        }

        if (!take_value(i, argc, argv, arg, val, out.error)) {
            return false;
        }

        try {
            if (name == "cpu" || name == "cores") {
                out.hardware.cpu_cores = std::stoi(val);
            } else if (name == "ram" || name == "memory") {
                out.hardware.ram_gb = std::stod(val);
            } else if (name == "storage") {
                StorageType st{};
                if (!parse_storage_type(val, st)) {
                    out.error = "Unknown storage type: " + val;
                    return false;
                }
                out.hardware.storage = st;
            } else if (name == "network" || name == "net") {
                out.hardware.network_mbps = std::stod(val);
            } else if (name == "label" || name == "name") {
                out.hardware.label = val;
            } else if (name == "task") {
                out.workload.task_type = val;
            } else if (name == "complexity") {
                WorkloadComplexity c{};
                if (!parse_complexity(val, c)) {
                    out.error = "Unknown complexity: " + val;
                    return false;
                }
                out.workload.complexity = c;
            } else if (name == "size") {
                out.workload.task_size = std::stod(val);
            } else {
                out.error = std::string("Unknown option: --") + name;
                return false;
            }
        } catch (const std::exception&) {
            out.error = std::string("Bad numeric value for --") + name + ": " + val;
            return false;
        }

        ++i;
    }

    if (out.command == CliCommand::Compare && out.compare_hardware.size() < 2) {
        out.error = "compare requires at least two `--hw cpu=...,ram=...` entries.";
        return false;
    }

    return true;
}
