// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OBJECT_WRAPPER_20240314_HPP
#define OBJECT_WRAPPER_20240314_HPP

#include "tinytc/cl/error.hpp"

#include <CL/cl.h>

namespace tinytc {

namespace internal {
template <typename T> class opencl_object_traits;
}

/**
 * @brief Wraps OpenCL type to have C++-style reference counting
 *
 * @tparam T OpenCL native type
 */
template <typename T> class opencl_object_wrapper {
  public:
    using native_type = T; ///< Wrapped type

    //! Create empty object
    opencl_object_wrapper() : obj_(nullptr) {}

    /**
     * @brief Wraps OpenCL handle
     *
     * @param obj OpenCL native handle
     * @param needs_retain Set to false if wrapper takes ownership; set to true otherwise
     */
    opencl_object_wrapper(T const &obj, bool needs_retain = false) : obj_(obj) {
        if (needs_retain) {
            CL_CHECK(retain());
        }
    }
    //! Decreases reference count on destruction
    ~opencl_object_wrapper() { release(); }
    //! Copy ctor
    opencl_object_wrapper(opencl_object_wrapper const &other) : obj_(other.obj_) {
        CL_CHECK(retain());
    }
    //! Move ctor
    opencl_object_wrapper(opencl_object_wrapper &&other) noexcept : obj_(other.obj_) {
        other.obj_ = nullptr;
    }
    //! Copy operator
    opencl_object_wrapper &operator=(opencl_object_wrapper const &other) {
        if (obj_ != other.obj_) {
            CL_CHECK(release());
            obj_ = other.obj_;
            CL_CHECK(retain());
        }
        return *this;
    }
    //! Move operator
    opencl_object_wrapper &operator=(opencl_object_wrapper &&other) {
        if (obj_ != other.obj_) {
            CL_CHECK(release());
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }

    //! Get native handle
    T &operator()() { return obj_; }
    //! Get native handle
    T const &operator()() const { return obj_; }
    //! Get native handle
    T get() const { return obj_; }

  private:
    auto retain() -> cl_int {
        if (obj_ != nullptr) {
            return internal::opencl_object_traits<T>::retain(obj_);
        }
        return CL_SUCCESS;
    }
    auto release() -> cl_int {
        if (obj_ != nullptr) {
            return internal::opencl_object_traits<T>::release(obj_);
        }
        return CL_SUCCESS;
    }

    T obj_;
};

namespace internal {
template <> class opencl_object_traits<cl_program> {
  public:
    static cl_int release(cl_program obj) { return clReleaseProgram(obj); }
    static cl_int retain(cl_program obj) { return clRetainProgram(obj); }
};

template <> class opencl_object_traits<cl_kernel> {
  public:
    static cl_int release(cl_kernel obj) { return clReleaseKernel(obj); }
    static cl_int retain(cl_kernel obj) { return clRetainKernel(obj); }
};

template <> class opencl_object_traits<cl_event> {
  public:
    static cl_int release(cl_event obj) { return clReleaseEvent(obj); }
    static cl_int retain(cl_event obj) { return clRetainEvent(obj); }
};
} // namespace internal

} // namespace tinytc

#endif // OBJECT_WRAPPER_20240314_HPP
