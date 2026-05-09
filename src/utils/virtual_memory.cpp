#include "utils/virtual_memory.h"

#include <iostream>

VirtualMemory::VirtualMemory(std::size_t size) : memory(size), used(0) {}

void* VirtualMemory::allocate(std::size_t size) {
    if (used + size > memory.size()) {
        std::cout << "Memory limit exceeded.\n";
        return nullptr;
    }

    void* ptr = &memory[used];
    used += size;
    return ptr;
}

std::size_t VirtualMemory::getUsed() const { return used; }

std::size_t VirtualMemory::getCapacity() const { return memory.size(); }
