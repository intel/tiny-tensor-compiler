// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// Code COPIED from Double-Batched FFT Library
// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SHARED_HANDLE_20240307_HPP
#define SHARED_HANDLE_20240307_HPP

#include <cstdint>
#include <memory>
#include <type_traits>

namespace tinytc {

template <typename T>
concept handle = requires(T t) {
                     requires sizeof(T) <= sizeof(void *);
                     requires alignof(T) <= alignof(void *);
                 };

/**
 * @brief Reference counted handled
 */
template <handle T> class shared_handle {
  public:
    using value_type = T; ///< Value type
    /**
     * @brief Empty handle
     */
    shared_handle() : handle_(nullptr) {}
    /**
     * @brief Construct shared handle with delete function
     *
     * @param t handle
     * @param delete_handle function that deletes the handle
     */
    shared_handle(T t, void (*delete_handle)(T))
        : handle_(reinterpret_cast<void *>(t), Deleter(delete_handle)) {}
    /**
     * @brief Get handle
     */
    T get() const { return reinterpret_cast<T>(handle_.get()); }
    /**
     * @brief Check whether handle is non-null.
     *
     * @return True if handle is non-null.
     */
    explicit operator bool() const noexcept { return bool(handle_); }

  private:
    struct Deleter {
        Deleter(void (*f)(T)) : f_(f) {}
        void operator()(void *ptr) { f_(reinterpret_cast<T>(ptr)); }
        void (*f_)(T);
    };
    std::shared_ptr<void> handle_;
};

} // namespace tinytc

#endif // SHARED_HANDLE_20240307_HPP
