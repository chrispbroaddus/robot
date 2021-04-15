
#pragma once

#include <memory>

namespace core {

struct AlignedMemoryAllocator {

    ///
    /// @return Pointer to aligned memory.
    ///
    template <typename SCALAR_T> static SCALAR_T* allocate(const size_t numScalars) {

        constexpr size_t alignment = alignof(SCALAR_T);
        size_t size = sizeof(SCALAR_T) * numScalars + alignment - 1;

        void* memory = new SCALAR_T[size];
        if (memory == nullptr) {
            return nullptr;
        }

        void* aligned_memory = std::align(alignment, sizeof(SCALAR_T), memory, size);

        return reinterpret_cast<SCALAR_T*>(aligned_memory);
    }
};
}
