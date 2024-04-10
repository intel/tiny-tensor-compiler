// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_KERNEL_20240307_HPP
#define CL_KERNEL_20240307_HPP

#include "tinytc/bundle_format.hpp"
#include "tinytc/export.h"

#include <CL/cl.h>
#include <array>
#include <cstdint>

namespace tinytc {

//! Provide ND range type for OpenCL analogue to sycl::nd_range
struct TINYTC_EXPORT opencl_nd_range {
    constexpr static cl_uint dim = 3u;             ///< Work-group dimension
    std::array<std::size_t, dim> global_work_size, ///< Global work-group size
        local_work_size;                           ///< Local work-group size
};

/**
 * @brief Create a kernel bundle ("program" in OpenCL terminology) from a binary
 *
 * @param binary Pointer to binary
 * @param binary_size Size of binary
 * @param format Binary format (SPIR-V or native)
 * @param core_features Required core features
 * @param context Context
 * @param device Device
 *
 * @return Kernel bundle handle
 */
TINYTC_EXPORT auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size,
                                      bundle_format format, std::uint32_t core_features,
                                      cl_context context, cl_device_id device) -> cl_program;
/**
 * @brief Create a kernel
 *
 * @param mod Native kernel bundle handle
 * @param name Kernel name
 *
 * @return Kernel handle
 */
TINYTC_EXPORT auto make_kernel(cl_program mod, char const *name) -> cl_kernel;
/**
 * @brief Get ND range for work-group size and group size
 *
 * @param work_group_size Local work-group size
 * @param howmany Group size
 *
 * @return ND range
 */
TINYTC_EXPORT auto get_opencl_nd_range(std::array<std::uint32_t, 2> work_group_size,
                                       std::size_t howmany) -> opencl_nd_range;

/**
 * @brief Check whether T is of pointer type but not of cl_mem type
 *
 * @tparam T value type
 */
template <typename T>
concept pointer_kernel_argument = std::is_pointer_v<std::decay_t<T>> && !
std::is_same_v<std::decay_t<T>, cl_mem>;

/**
 * @brief Check whether T is a regular (non-pointer) argument
 *
 * @tparam T value type
 */
template <typename T>
concept regular_kernel_argument = !
pointer_kernel_argument<T>;

/**
 * @brief Wrapper for setting kernel arguments
 */
class TINYTC_EXPORT opencl_argument_handler {
  public:
    using clSetKernelArgMemPointerINTEL_t =
        cl_int (*)(cl_kernel kernel, cl_uint arg_index,
                   const void *arg_value); ///< Signature of clSetKernelArgMemPointerINTEL function
    //! ctor
    opencl_argument_handler();
    //! ctor; checks whether cl_intel_unified_shared_memory is available and gets
    //! clSetKernelArgMemPointerINTEL
    opencl_argument_handler(cl_platform_id plat);

    /**
     * @brief Set single pointer kernel argument
     *
     * Throws opencl_error if cl_intel_unified_shared_memory extension is unavailable.
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_value Argument value
     */
    void set_arg_mem_pointer(cl_kernel kernel, std::uint32_t arg_index, void const *arg_value);
    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    void set_arg(cl_kernel kernel, std::uint32_t arg_index, std::size_t arg_size,
                 void const *arg_value);

    /**
     * @brief Set single pointer kernel argument
     *
     * Throws opencl_error if cl_intel_unified_shared_memory extension is unavailable.
     *
     * @tparam T Pointer argument type
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg Argument value
     */
    template <pointer_kernel_argument T>
    void set_arg(cl_kernel kernel, std::uint32_t arg_index, T const &arg) {
        set_arg_mem_pointer(kernel, arg_index, arg);
    }
    /**
     * @brief Set single kernel argument
     *
     * @tparam T Argument type
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg Argument value
     */
    template <regular_kernel_argument T>
    void set_arg(cl_kernel kernel, std::uint32_t arg_index, T const &arg) {
        set_arg(kernel, arg_index, sizeof(T), &arg);
    }

    /**
     * @brief Set multiple kernel arguments
     *
     * @tparam T Argument types
     * @param kernel Kernel handle
     * @param ...args Argument values
     */
    template <typename... T> void set_args(cl_kernel kernel, T const &...args) {
        set_args_helper(kernel, 0, args...);
    }

  private:
    template <typename Head, typename... Tail>
    void set_args_helper(cl_kernel kernel, std::uint32_t arg_index, Head &head,
                         Tail const &...tail) {
        set_arg(kernel, arg_index, head);
        if constexpr (sizeof...(Tail) > 0) {
            set_args_helper(kernel, arg_index + 1, tail...);
        }
    }

    clSetKernelArgMemPointerINTEL_t clSetKernelArgMemPointerINTEL_;
};

} // namespace tinytc

#endif // CL_KERNEL_20240307_HPP
