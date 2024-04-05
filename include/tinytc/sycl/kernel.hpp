// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_KERNEL_20240307_HPP
#define SYCL_KERNEL_20240307_HPP

#include "tinytc/bundle_format.hpp"
#include "tinytc/cl/kernel.hpp"
#include "tinytc/export.hpp"
#include "tinytc/ze/kernel.hpp"

#include <array>
#include <cstdint>
#include <sycl/sycl.hpp>

namespace tinytc {

/**
 * @brief Create a kernel bundle from a binary
 *
 * @param binary Pointer to binary
 * @param binary_size Size of binary
 * @param format Binary format (SPIR-V or native)
 * @param core_features Required core features
 * @param ctx Context
 * @param dev Device
 *
 * @return Kernel bundle
 */
TINYTC_EXPORT auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size,
                                      bundle_format format, std::uint32_t core_features,
                                      sycl::context ctx, sycl::device dev)
    -> sycl::kernel_bundle<sycl::bundle_state::executable>;
/**
 * @brief Create a kernel
 *
 * @param mod Kernel bundle
 * @param name Kernel name
 *
 * @return Kernel
 */
TINYTC_EXPORT auto make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> mod,
                               char const *name) -> sycl::kernel;
/**
 * @brief Get ND range for work-group size and group size
 *
 * @param work_group_size Local work-group size
 * @param howmany Group size
 *
 * @return ND range
 */
TINYTC_EXPORT auto get_sycl_nd_range(std::array<std::uint32_t, 2> work_group_size,
                                     std::size_t howmany) -> sycl::nd_range<3u>;

/**
 * @brief Wrapper for setting kernel arguments
 */
class TINYTC_EXPORT sycl_argument_handler {
  public:
    //! ctor
    sycl_argument_handler(sycl::platform plat);

    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    void set_arg(sycl::kernel kernel, std::uint32_t arg_index, std::size_t arg_size,
                 void const *arg_value);

    /**
     * @brief Set single kernel argument
     *
     * @tparam T Argument type
     * @param kernel Kernel
     * @param arg_index Argument index
     * @param arg Argument value
     */
    template <typename T> void set_arg(sycl::kernel kernel, std::uint32_t arg_index, T const &arg) {
        set_arg(std::move(kernel), arg_index, sizeof(T), &arg);
    }

    /**
     * @brief Set multiple kernel arguments
     *
     * @tparam T Argument types
     * @param kernel Kernel
     * @param ...args Argument values
     */
    template <typename... T> void set_args(sycl::kernel kernel, T const &...args) {
        set_args_helper(std::move(kernel), 0, args...);
    }

  private:
    template <typename Head, typename... Tail>
    void set_args_helper(sycl::kernel kernel, std::uint32_t arg_index, Head &head,
                         Tail const &...tail) {
        if constexpr (sizeof...(Tail) > 0) {
            set_arg(kernel, arg_index, head);
            set_args_helper(std::move(kernel), arg_index + 1, tail...);
        } else {
            set_arg(std::move(kernel), arg_index, head);
        }
    }

    level_zero_argument_handler ze_arg_;
    opencl_argument_handler cl_arg_;
};

} // namespace tinytc

#endif // SYCL_KERNEL_20240307_HPP
