#pragma once

#include "hardware/hardware_config.h"

#include <string>

// True when this build can start an OS-enforced constrained shell (Windows Job Object).
bool constrained_session_available();

// Short note about what is / is not enforced for the interactive session.
std::string constrained_session_limitations_note();

// Spawns an interactive shell (same console) whose process tree is limited by the
// current platform's mechanisms. On Windows: job-wide committed memory cap and CPU
// hard cap derived from --cpu vs logical processors.
// Returns the shell's exit code on success, or -1 on setup failure (see `error`).
int run_constrained_interactive_session(const HardwareConfig& hw, std::string& error);
