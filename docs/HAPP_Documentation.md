# HAPP Solution Documentation

**Project:** Hardware-Agnostic Performance Profiler (HAPP)  
**Audience:** Lecturer presentation  
**Repository:** `Hardware-Agnostic-Performance-Profiler-HAPP-`

## 1. Executive Summary

HAPP is a C++17 command-line simulator that estimates application performance under constrained hardware profiles. Instead of emulating CPU instructions, it applies a practical analytical model across compute, storage I/O, network throughput, and memory pressure so developers can compare scenarios quickly.

## 2. Problem Context

Embedded and IoT software often fails when moved from high-performance development machines to lower-resource targets. Typical gaps include:
- Underestimated memory needs
- Hidden compute bottlenecks
- Storage-induced slowdowns
- Network sensitivity for transfer-heavy tasks

HAPP addresses these by simulating “what-if” hardware environments using configurable parameters (CPU cores, RAM, storage class, network Mbps).

## 3. Project Goals

- Provide a lightweight hardware-constrained performance estimation tool.
- Make bottleneck causes explicit (CPU, RAM, storage, network).
- Support repeatable CLI workflows for simulation, comparison, and config persistence.
- Remain extensible for future telemetry and deeper resource modeling.

## 4. Architecture Overview

HAPP is organized into focused modules:

- **CLI layer** (`cli_parser`, `cli_runner`): command parsing, option validation, and user-facing reporting.
- **Hardware model** (`hardware_config`, `storage_type`): machine profile representation and storage classification.
- **Workload model** (`workload_spec`): task type, complexity, and size controls.
- **Simulation engine** (`simulation_engine`): core estimation model for time and resource usage.
- **Profiler analysis** (`bottleneck_analyzer`): bottleneck detection, warnings, and recommendations.
- **Result reporting** (`simulation_result`): normalized output object and printable report.
- **Utilities** (`config_file`, `virtual_memory`): config persistence and educational fixed-capacity allocator.

## 5. Implementation Details

### 5.1 Command Interface

Supported commands:
- `simulate`: run one profile + workload scenario.
- `compare`: run multiple hardware profiles against the same workload (requires at least two `--hw` entries).
- `save`: write current hardware/workload setup to key=value file.
- `help`: usage guidance.

Input style supports both `--flag value` and `--flag=value`.

### 5.2 Hardware Configuration Model

`HardwareConfig` includes:
- `cpu_cores` (int)
- `ram_gb` (double)
- `storage` (`HDD | SSD | NVMe`)
- `network_mbps` (double)
- optional `label`

Default baseline is 4 cores, 8 GiB RAM, SSD, 100 Mb/s.

### 5.3 Workload Model

`WorkloadSpec` includes:
- `task_type` (e.g., `rendering`, `compiling`, `database`, `streaming`, `generic`)
- `complexity` (`Low`, `Medium`, `High`)
- `task_size` (double scalar)

Each task maps to heuristic weights (`cpu`, `mem_per_size`, `io`, `net`) for simulation.

### 5.4 Simulation Algorithm

The simulation computes three core time components:
1. **Compute component**: inversely scaled by `cores^0.72` (sub-linear parallelism).
2. **I/O component**: scaled by storage latency factor (HDD slower, NVMe faster).
3. **Network component**: scaled against a 500 Mb/s reference.

Total execution estimate combines dominant and average effects:

`execution_time = 0.65 * dominant_component + 0.35 * average(component_sum)`

Memory pressure penalty is applied when modeled memory requirement exceeds configured RAM.

### 5.5 Bottleneck Analysis Logic

After simulation, the profiler:
- labels primary bottleneck (`RAM`, `CPU`, `Storage I/O`, `Network`),
- emits warnings for risky conditions (e.g., low RAM, HDD with I/O-heavy task),
- provides targeted recommendations.

Recommendations are deduplicated while preserving insertion order.

### 5.6 Configuration Persistence

`load_happ_config` and `save_happ_config` support key=value profiles. Config files can be layered via `--config` and overridden by later CLI flags.

### 5.7 Virtual Memory Utility

`VirtualMemory` demonstrates fixed-capacity allocation using a bump-pointer model. It currently serves as a simple educational mechanism and extension point for future memory pressure integration.

## 6. Build and Execution Workflow

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Example Run

```bash
./happ simulate --cpu 4 --ram 8 --storage ssd --task rendering --complexity high --size 2
```

### Example Compare

```bash
./happ compare \
  --hw cpu=2,ram=4,storage=hdd,network=50,label=EdgeA \
  --hw cpu=8,ram=16,storage=nvme,network=500,label=EdgeB \
  --task database --complexity high --size 2
```

## 7. Current Status vs Roadmap

Implemented:
- Core CLI command handling
- Analytical simulation engine
- Bottleneck classifier and recommendation output
- Config file load/save

Planned / partial:
- Richer virtual memory integration
- CPU throttling controller
- Scheduler-like execution behavior
- Time-series telemetry and visualization

## 8. Design Tradeoffs

- **Chosen approach:** analytical, explainable, fast simulation.
- **Not targeted:** instruction-level CPU emulation or OS-accurate scheduling.
- **Benefit:** easier iteration, suitable for teaching, comparison, and early-stage capacity planning.
- **Limit:** estimates are heuristic and should be validated against real hardware benchmarks.

## 9. Testing and Validation Strategy

Suggested academic validation steps:
1. Define representative workloads by domain (render, compile, DB, stream).
2. Execute identical scenarios across real hardware and HAPP profiles.
3. Compare trend consistency (ranking/order) rather than exact absolute timing.
4. Calibrate constants (`kSecondsPerUnit`, scaling factors) to improve realism.

## 10. Conclusion

HAPP demonstrates a practical and extensible method for profiling resource sensitivity without requiring all target devices physically present. The current implementation is suitable as a foundational MVP for coursework, with clear pathways to stronger fidelity and visualization in subsequent iterations.
