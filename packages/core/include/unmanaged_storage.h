
#pragma once

namespace core {

template <typename SCALAR_T> struct UnmanagedStorage {

    typedef SCALAR_T scalar_t;

    ///
    /// Number of allocated SCALARS in storage.
    ///
    size_t numScalars;

    ///
    /// Pointer to the image memory.
    ///
    scalar_t* storage;

    UnmanagedStorage() = delete;
    UnmanagedStorage(const UnmanagedStorage& copyFrom)
        : numScalars(copyFrom.numScalars)
        , storage(copyFrom.storage) {}
    UnmanagedStorage(const size_t numScalars_, scalar_t* storage_)
        : numScalars(numScalars_)
        , storage(storage_) {}

    ~UnmanagedStorage() = default;

    ///
    /// @return Underlying image buffer
    ///
    inline scalar_t* data() { return storage; }
};
}
