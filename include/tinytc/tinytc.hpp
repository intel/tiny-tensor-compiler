// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240403_HPP
#define TINYTC_20240403_HPP

#include "tinytc/tinytc.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

// TODO: Remove
#include "tinytc/ir/scalar_type.hpp"

////////////////////////////
////////// Macros //////////
////////////////////////////

//! Capture error code and throw error code if unsuccessful
#define TINYTC_CHECK(X)                                                                            \
    [](tinytc_status_t code) {                                                                     \
        if (code != tinytc_success) {                                                              \
            auto code_ec = static_cast<status>(code);                                              \
            throw code_ec;                                                                         \
        }                                                                                          \
    }(X)

namespace tinytc {

//! Convert error code to string
inline char const *error_string(status code) {
    return ::tinytc_error_string(static_cast<::tinytc_status_t>(code));
}

////////////////////////////
// C++ reference counting //
////////////////////////////

//! Wraps destroy calls for type T
template <typename T> struct handle_traits {};

/**
 * @brief Wraps a C handle in a reference-counted object
 *
 * @tparam T C handle type (handle type = pointer to opaque struct)
 */
template <typename T> class handle {
  public:
    //! Traits shortcut
    using traits = handle_traits<T>;

    //! Create empty (invalid) handle
    handle() : obj_{nullptr} {}
    //! Create handle from C handle
    handle(T obj, bool needs_retain = false) : obj_(obj) {
        if (needs_retain) {
            TINYTC_CHECK(retain());
        }
    }
    //! Decrease reference count
    ~handle() {
        if (obj_) {
            release();
        }
    }
    //! Copy ctor
    handle(handle const &other) : obj_(other.obj_) { TINYTC_CHECK(retain()); }
    //! Move ctor
    handle(handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    handle &operator=(handle const &other) {
        if (obj_ != other.obj_) {
            TINYTC_CHECK(release());
            obj_ = other.obj_;
            TINYTC_CHECK(retain());
        }
        return *this;
    }
    //! Move operator
    handle &operator=(handle &&other) {
        if (obj_ != other.obj_) {
            TINYTC_CHECK(release());
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Convert handle to C handle
    auto get() const -> T { return obj_; }
    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(handle<T> const &other) const { return !(*this == other); }

  protected:
    auto retain() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::retain(obj_);
        }
        return tinytc_success;
    }
    auto release() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::release(obj_);
        }
        return tinytc_success;
    }
    T obj_;
};

////////////////////////////
///////// Data type ////////
////////////////////////////

//! Check if mode i is dynamic ('?')
inline bool is_dynamic_value(std::int64_t i) { return i == dynamic; }

template <> struct handle_traits<tinytc_data_type_t> {
    static auto retain(tinytc_data_type_t handle) -> tinytc_status_t {
        return tinytc_data_type_retain(handle);
    }
    static auto release(tinytc_data_type_t handle) -> tinytc_status_t {
        return tinytc_data_type_release(handle);
    }
};

//! Reference-counted program handle
class data_type : public handle<tinytc_data_type_t> {
  public:
    using handle::handle;

    data_type(scalar_type type) {
        TINYTC_CHECK(tinytc_scalar_type_create(&obj_, static_cast<tinytc_scalar_type_t>(type)));
    }
};

inline data_type memref_type(scalar_type scalar_ty, std::vector<std::int64_t> const &shape,
                             std::vector<std::int64_t> const &stride = {},
                             location const &loc = {}) {
    tinytc_data_type_t mt;
    TINYTC_CHECK(tinytc_memref_type_create(&mt, static_cast<tinytc_scalar_type_t>(scalar_ty),
                                           shape.size(), shape.data(), stride.size(), stride.data(),
                                           &loc));
    return data_type(mt);
}
inline data_type group_type(data_type memref_ty) {
    tinytc_data_type_t gt;
    TINYTC_CHECK(tinytc_group_type_create(&gt, memref_ty.get()));
    return data_type(gt);
}

} // namespace tinytc

#endif // TINYTC_20240403_HPP
