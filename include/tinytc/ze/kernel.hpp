// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_KERNEL_20240307_HPP
#define ZE_KERNEL_20240307_HPP

#include "tinytc/export.h"

#include <cstddef>
#include <cstdint>
#include <level_zero/ze_api.h>

namespace tinytc {

enum class bundle_format;

/**
 * @brief Create a kernel bundle ("module" in Level Zero terminology) from a binary
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
                                      ze_context_handle_t context, ze_device_handle_t device)
    -> ze_module_handle_t;
/**
 * @brief Create a kernel
 *
 * @param mod Native kernel bundle handle
 * @param name Kernel name
 *
 * @return Kernel handle
 */
TINYTC_EXPORT auto make_kernel(ze_module_handle_t mod, char const *name) -> ze_kernel_handle_t;
/**
 * @brief Get Level Zero group count for group size
 *
 * @param howmany Group size
 *
 * @return Group count
 */
TINYTC_EXPORT auto get_group_count(std::uint32_t howmany) -> ze_group_count_t;

/**
 * @brief Wrapper for setting kernel arguments
 */
class TINYTC_EXPORT level_zero_argument_handler {
  public:
    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    void set_arg(ze_kernel_handle_t kernel, std::uint32_t arg_index, std::size_t arg_size,
                 void const *arg_value);

    /**
     * @brief Set single kernel argument
     *
     * @tparam T Argument type
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg Argument value
     */
    template <typename T>
    void set_arg(ze_kernel_handle_t kernel, std::uint32_t arg_index, T const &arg) {
        set_arg(kernel, arg_index, sizeof(T), &arg);
    }

    /**
     * @brief Set multiple kernel arguments
     *
     * @tparam T Argument types
     * @param kernel Kernel handle
     * @param ...args Argument values
     */
    template <typename... T> void set_args(ze_kernel_handle_t kernel, T const &...args) {
        set_args_helper(kernel, 0, args...);
    }

  private:
    template <typename Head, typename... Tail>
    void set_args_helper(ze_kernel_handle_t kernel, std::uint32_t arg_index, Head &head,
                         Tail const &...tail) {
        set_arg(kernel, arg_index, head);
        if constexpr (sizeof...(Tail) > 0) {
            set_args_helper(kernel, arg_index + 1, tail...);
        }
    }
};

} // namespace tinytc

#endif // ZE_KERNEL_20240307_HPP
