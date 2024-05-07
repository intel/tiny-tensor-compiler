// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_ARGUMENT_HANDLER_20240424_HPP
#define SYCL_ARGUMENT_HANDLER_20240424_HPP

#include "../cl/argument_handler.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/tinytc_ze.hpp"
#include "tinytc/types.h"

#include <CL/cl.h>
#include <cstdint>
#include <level_zero/ze_api.h>
#include <sycl/sycl.hpp>

namespace tinytc {

class sycl_argument_handler {
  public:
    virtual ~sycl_argument_handler() = default;
    virtual void set_arg(sycl::kernel const &krnl, std::uint32_t arg_index, std::size_t arg_size,
                         void const *arg_value) const = 0;
    virtual void set_mem_arg(sycl::kernel const &krnl, std::uint32_t arg_index, const void *value,
                             tinytc_mem_type_t type) const = 0;
};

class sycl_argument_handler_opencl_backend : public sycl_argument_handler {
  public:
    inline sycl_argument_handler_opencl_backend(sycl::platform const &plat)
        : cl_arg_(sycl::get_native<sycl::backend::opencl, sycl::platform>(plat)) {}

    inline void set_arg(sycl::kernel const &krnl, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) const override {
        auto native_krnl = sycl::get_native<sycl::backend::opencl, sycl::kernel>(krnl);
        cl_arg_.set_arg(native_krnl, arg_index, arg_size, arg_value);
        CL_CHECK_STATUS(clReleaseKernel(native_krnl));
    }

    inline void set_mem_arg(sycl::kernel const &krnl, std::uint32_t arg_index, const void *value,
                            tinytc_mem_type_t type) const override {
        auto native_krnl = sycl::get_native<sycl::backend::opencl, sycl::kernel>(krnl);
        cl_arg_.set_mem_arg(native_krnl, arg_index, value, type);
        CL_CHECK_STATUS(clReleaseKernel(native_krnl));
    }

  private:
    opencl_argument_handler cl_arg_;
};

class sycl_argument_handler_level_zero_backend : public sycl_argument_handler {
  public:
    inline void set_arg(sycl::kernel const &krnl, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) const override {
        auto native_krnl =
            sycl::get_native<sycl::backend::ext_oneapi_level_zero, sycl::kernel>(krnl);
        ZE_CHECK_STATUS(zeKernelSetArgumentValue(native_krnl, arg_index, arg_size, arg_value));
    }

    inline void set_mem_arg(sycl::kernel const &krnl, std::uint32_t arg_index, const void *value,
                            tinytc_mem_type_t) const override {
        auto native_krnl =
            sycl::get_native<sycl::backend::ext_oneapi_level_zero, sycl::kernel>(krnl);
        ZE_CHECK_STATUS(zeKernelSetArgumentValue(native_krnl, arg_index, sizeof(value), &value));
    }
};

} // namespace tinytc

#endif // SYCL_ARGUMENT_HANDLER_20240424_HPP
