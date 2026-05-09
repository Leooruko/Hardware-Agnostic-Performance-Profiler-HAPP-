#include "hardware/storage_type.h"

#include <algorithm>
#include <cctype>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool parse_storage_type(const std::string& text, StorageType& out) {
    const std::string k = to_lower(text);
    if (k == "hdd" || k == "disk" || k == "spinning") {
        out = StorageType::HDD;
        return true;
    }
    if (k == "ssd" || k == "sata_ssd") {
        out = StorageType::SSD;
        return true;
    }
    if (k == "nvme" || k == "pcie") {
        out = StorageType::NVMe;
        return true;
    }
    return false;
}

std::string storage_type_label(StorageType s) {
    switch (s) {
        case StorageType::HDD:
            return "HDD";
        case StorageType::SSD:
            return "SSD";
        case StorageType::NVMe:
            return "NVMe";
    }
    return "Unknown";
}
