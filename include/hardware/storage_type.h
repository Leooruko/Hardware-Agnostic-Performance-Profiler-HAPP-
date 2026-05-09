#pragma once

#include <string>

// How we label storage for simple I/O scaling in the simulator.
// SSD and NVMe are treated as faster than spinning disk (HDD).
enum class StorageType { HDD, SSD, NVMe };

// Parse user strings like "ssd", "hdd", "nvme" (case-insensitive).
// On failure, returns false and leaves `out` unchanged.
bool parse_storage_type(const std::string& text, StorageType& out);

std::string storage_type_label(StorageType s);
