#pragma once

#include "hardware/hardware_config.h"
#include "workload/workload_spec.h"

#include <string>
#include <vector>

enum class CliCommand { Help, Simulate, Compare, Save, Invalid };

struct CliOptions {
    CliCommand command = CliCommand::Invalid;
    std::string error;

    HardwareConfig hardware;
    WorkloadSpec workload;

    // `happ simulate --session`: start an OS-enforced shell instead of only printing estimates.
    bool simulate_constrained_session = false;

    std::string save_path;

    // One entry per `--hw ...` for the compare command.
    std::vector<HardwareConfig> compare_hardware;
};

// Parses `happ <subcommand> [options]`. On error, sets `out.error` and returns false.
bool parse_cli(int argc, char** argv, CliOptions& out);
