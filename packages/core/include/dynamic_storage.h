
#pragma once

#include <memory>

namespace core {

template <typename SCALAR_T, typename MEMORY_ALLOCATOR_T> struct DynamicStorage {

    typedef SCALAR_T scalar_t;
    typedef MEMORY_ALLOCATOR_T memory_allocator_t;

    ///
    /// Number of allocated SCALARS in storage.
    ///
    size_t numScalars;

    ///
    /// Pointer to the image memory.
    ///
    std::shared_ptr<scalar_t> storage;

    DynamicStorage() = default;
    DynamicStorage(const DynamicStorage& copyFrom)
        : numScalars(copyFrom.numScalars)
        , storage(copyFrom.storage) {}
    DynamicStorage(const size_t numScalars_)
        : numScalars(numScalars_)
        , storage(memory_allocator_t::template allocate<scalar_t>(numScalars_)) {
        if (storage == nullptr) {
            numScalars = 0;
        }
    }

    ~DynamicStorage() { numScalars = 0; }

    ///
    /// @return Underlying image buffer
    ///
    inline scalar_t* data() { return storage.get(); }
};
}
