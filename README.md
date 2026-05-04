# HAPP — Hardware-Agnostic Performance Profiler

**Simulate CPU and memory limits on your development machine so you can stress-test code the way it might run on constrained hardware—without buying every board.**

---

## Problem statement

Shipping software for IoT and embedded targets usually means juggling slow boards, small RAM, and hard-to-reproduce environments. Running everything on a fast laptop hides allocation spikes, timing assumptions, and “works on my machine” issues that only show up when resources are tight. Physical hardware for every scenario is expensive, fragile on the bench, and slow to iterate against.

---

## Solution

**HAPP** applies **constraint-based simulation**: you pick a **hardware profile** (target CPU speed and RAM), and the system helps you run and observe workloads as if those limits mattered. It is **not** a CPU architecture simulator—there are no instruction pipelines, branch predictors, or ISA-level emulation. Instead, HAPP focuses on **practical limits** (memory ceilings, throttled execution, task behavior) so you can reason about how your code behaves when the machine is “smaller” than your dev box.

---

## Key features

| Area | Description |
|------|-------------|
| **Hardware profiles** | Named targets with CPU frequency and RAM budget (foundation for all other limits). |
| **Virtual memory constraints** | Enforce or approximate a smaller addressable / usable RAM envelope for workloads. *(Planned / in progress.)* |
| **Execution throttling** | Simulate slower effective CPU speed so compute-heavy paths reveal themselves under time pressure. *(Planned.)* |
| **Task execution & monitoring** | Run units of work under profiles and observe scheduling-friendly behavior. *(Planned.)* |
| **Telemetry** | Track CPU and memory usage over time for comparison across profiles. *(Planned.)* |

---

## System architecture (high-level)

Modules are designed to stay small and composable:

- **Hardware profile** — Describes a target: name, nominal CPU frequency, RAM bytes. Other subsystems read this as the source of truth for limits.
- **Virtual memory** — Maps profile RAM into policies (caps, warnings, or simulated pressure) without pretending to be a full MMU.
- **Execution controller** — Applies throttling and ties “how fast we pretend the CPU is” to wall-clock or logical progress of tasks.
- **Scheduler (optional)** — Orders tasks under constraints; can stay minimal at first and grow only if needed.
- **Telemetry** — Samples usage and emits summaries or streams for debugging and regression-style comparisons.

Together they form a **constraint layer** on top of normal C++ execution—not a second machine inside the first.

---

## Getting started

### Requirements

- **CMake** (3.10 or newer)
- **MinGW** or **GCC** with C++17 support

### Build

From the project root:

```bash
mkdir build
cd build
cmake ..
mingw32-make
```

On Windows with MinGW, the executable is **`happ.exe`** in the `build` directory. Run it from `build`:

```bash
./happ.exe
```

*(On Unix-like systems with Make, use `make` instead of `mingw32-make`.)*

---

## Project structure

```
HAPP/
├── CMakeLists.txt      # Build configuration
├── README.md
├── include/            # Public headers
├── src/                # Implementation
├── build/              # Out-of-tree build (local; not always committed)
└── docs/               # Design notes, diagrams, extra documentation
```

---

## Current progress

- **Done:** Project setup (CMake, C++17), executable wiring.
- **Done:** **Hardware Profile** module — structured profiles, factory-style helpers for sample devices, basic reporting.
- **In progress:** **Virtual memory** subsystem (design and integration with profiles).

---

## Roadmap

1. Virtual memory system (limits, integration with profiles)
2. CPU throttling (execution controller)
3. Task scheduler (lightweight, optional depth)
4. Telemetry (CPU / memory sampling and reporting)

---

## Design philosophy

- **Simplicity over overengineering** — Fewer moving parts until a feature proves its worth.
- **Practical simulation over full hardware emulation** — Model what affects your code’s resource story, not every transistor.
- **Systems thinking** — Profiles, memory, time, and observation should fit together as one coherent tool, not a bag of demos.

---

## Future improvements

- **JSON (or similar) hardware profiles** — Load targets from files for CI and team-shared device catalogs.
- **GUI visualization (optional)** — Charts for usage over time and profile comparison.
- **Deeper simulation options** — Still constraint- and behavior-oriented, not microarchitectural replay.

---

## Author / motivation

This project is being built as a **learning exercise**: a concrete way to explore how **resource constraints** shape software behavior, and how a small profiler-shaped tool can make that visible on everyday hardware. Contributions and experiments are welcome as the roadmap expands.
