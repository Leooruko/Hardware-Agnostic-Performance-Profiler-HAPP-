#pragma once

#include <cstddef>
#include <vector>

// Tiny bump-pointer allocator over a fixed buffer — useful for demos of RAM limits.
class VirtualMemory {
   public:
    explicit VirtualMemory(std::size_t size);

    void* allocate(std::size_t size);
    std::size_t getUsed() const;
    std::size_t getCapacity() const;

   private:
    std::vector<unsigned char> memory;
    std::size_t used;
};
