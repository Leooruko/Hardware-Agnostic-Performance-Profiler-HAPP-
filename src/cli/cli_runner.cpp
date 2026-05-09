#include "cli/cli_runner.h"

#include "cli/cli_parser.h"
#include "hardware/hardware_config.h"
#include "hardware/storage_type.h"
#include "simulator/simulation_engine.h"
#include "simulator/simulation_result.h"
#include "utils/config_file.h"
#include "workload/workload_spec.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

void print_help() {
    std::cout << R"(
HAPP — Hardware-Agnostic Performance Profiler (MVP)

Commands:
  happ simulate [options]      Estimate metrics for one machine definition.
  happ compare  [options]      Compare multiple machines on the same workload.
  happ save <file> [options]   Write hardware/workload settings to a text file.
  happ help                    Show this text.

Common options (simulate / compare / save):
  --cpu <n>           CPU core count (default 4)
  --ram <gb>          RAM in GiB (default 8)
  --storage <type>    hdd | ssd | nvme (default ssd)
  --network <mbps>    Network speed in Mb/s (default 100)
  --label <text>      Optional friendly name for reports
  --task <name>       rendering | compiling | database | streaming | generic
  --complexity <lvl>  low | medium | high
  --size <number>     Workload size scalar (default 1.0)
  --config <file>     Load key=value pairs when seen; later flags override

Compare-only:
  --hw cpu=4,ram=8,storage=ssd,network=100[,label=Home PC]
      Repeat --hw for each configuration (at least two).

Example:
  happ simulate --cpu 4 --ram 8 --storage ssd --task rendering --complexity high --size 2
)";
}

void print_workload_banner(const WorkloadSpec& w) {
    std::cout << "\nWorkload: task=" << w.task_type << ", complexity=" << complexity_label(w.complexity)
              << ", size=" << w.task_size << "\n";
}

void run_compare_table(const std::vector<HardwareConfig>& configs, const WorkloadSpec& workload) {
    print_workload_banner(workload);
    std::cout << "\n=== Comparison (estimated) ===\n";
    std::cout << std::left << std::setw(18) << "Label" << std::setw(8) << "Cores" << std::setw(8)
              << "RAM" << std::setw(10) << "Storage" << std::setw(12) << "Net Mb/s"
              << std::setw(12) << "Time (s)" << std::setw(14) << "CPU %" << std::setw(12)
              << "Mem (GiB)" << "Bottleneck\n";

    std::vector<SimulationResult> results;
    results.reserve(configs.size());

    double best_time = 1e100;
    for (const auto& c : configs) {
        SimulationResult r = simulate(c, workload);
        results.push_back(r);
        best_time = std::min(best_time, r.execution_time_sec);
    }

    std::cout << std::fixed << std::setprecision(2);
    for (std::size_t i = 0; i < configs.size(); ++i) {
        const auto& c = configs[i];
        const auto& r = results[i];
        const std::string lab = c.label.empty() ? ("cfg " + std::to_string(i + 1)) : c.label;
        std::cout << std::setw(18) << lab << std::setw(8) << c.cpu_cores << std::setw(8) << c.ram_gb
                  << std::setw(10) << storage_type_label(c.storage) << std::setw(12)
                  << c.network_mbps << std::setw(12) << r.execution_time_sec << std::setw(14)
                  << r.cpu_usage_percent << std::setw(12) << r.memory_usage_gb
                  << r.primary_bottleneck << "\n";
    }

    std::cout << "\nFastest scenario (~" << best_time << " s):\n";
    for (std::size_t i = 0; i < configs.size(); ++i) {
        if (results[i].execution_time_sec <= best_time * 1.0001) {
            print_simulation_report(results[i], configs[i].label.empty()
                                                     ? ("Winner: config " + std::to_string(i + 1))
                                                     : ("Winner: " + configs[i].label));
        }
    }
}

}  // namespace

int run_happ_cli(int argc, char** argv) {
    CliOptions opt;
    opt.hardware = default_hardware_config();
    opt.workload = default_workload_spec();

    if (!parse_cli(argc, argv, opt)) {
        std::cerr << "Error: " << opt.error << "\n";
        return 1;
    }

    if (opt.command == CliCommand::Help) {
        print_help();
        return 0;
    }

    if (opt.command == CliCommand::Save) {
        std::string err;
        if (!save_happ_config(opt.save_path, opt.hardware, opt.workload, err)) {
            std::cerr << "Error: " << err << "\n";
            return 1;
        }
        std::cout << "Wrote " << opt.save_path << "\n";
        return 0;
    }

    if (opt.command == CliCommand::Simulate) {
        std::cout << "HAPP simulate\n";
        opt.hardware.print_summary();
        print_workload_banner(opt.workload);
        SimulationResult r = simulate(opt.hardware, opt.workload);
        print_simulation_report(r);
        return 0;
    }

    if (opt.command == CliCommand::Compare) {
        std::cout << "HAPP compare\n";
        run_compare_table(opt.compare_hardware, opt.workload);
        return 0;
    }

    std::cerr << "Internal error: unhandled command.\n";
    return 2;
}
