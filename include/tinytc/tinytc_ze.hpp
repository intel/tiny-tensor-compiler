// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_ZE_20240416_HPP
#define TINYTC_ZE_20240416_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_ze.h"

#include <exception>
#include <level_zero/ze_api.h>

////////////////////////////
////////// Macros //////////
////////////////////////////

//! Capture ze result and throw error if unsuccessful
#define TINYTC_ZE_CHECK(X)                                                                         \
    [](ze_result_t result) {                                                                       \
        if (result != ZE_RESULT_SUCCESS) {                                                         \
            throw ::tinytc::level_zero_error(result);                                              \
        }                                                                                          \
    }(X)

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

class level_zero_error : public std::exception {
  public:
    inline level_zero_error(ze_result_t result) : result_(result) {}

    inline char const *what() const noexcept override { return ::tinytc_ze_result_string(result_); }

  private:
    ze_result_t result_;
};

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from level zero runtime
 *
 * @param info [out] pointer to the core_info object created
 * @param device [in] device handle
 *
 * @return ZE_RESULT_SUCCESS on success and error otherwise
 */
inline auto get_core_info(ze_device_handle_t device) -> core_info {
    tinytc_core_info_t info;
    TINYTC_ZE_CHECK(::tinytc_ze_core_info_create(&info, device));
    return core_info{info};
}

////////////////////////////
////////// Runtime /////////
////////////////////////////

template <> struct unique_handle_traits<ze_kernel_handle_t> {
    static void destroy(ze_kernel_handle_t obj) { zeKernelDestroy(obj); }
};
template <> struct unique_handle_traits<ze_module_handle_t> {
    static void destroy(ze_module_handle_t obj) { zeModuleDestroy(obj); }
};

/**
 * @brief Wrapper for setting kernel arguments
 */
class level_zero_argument_handler {
  public:
    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    inline void set_arg(ze_kernel_handle_t kernel, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) {
        TINYTC_ZE_CHECK(zeKernelSetArgumentValue(kernel, arg_index, arg_size, arg_value));
    }

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

/**
 * @brief Level Zero runtime
 */
class level_zero_runtime {
  public:
    struct work_group_size_dummy {};
    using context_t = ze_context_handle_t; ///< Context handle type
    using device_t = ze_device_handle_t;   ///< Device handle type
    using kernel_bundle_t =
        unique_handle<ze_module_handle_t>;                  ///< Wrapped kernel bundle handle type
    using kernel_t = unique_handle<ze_kernel_handle_t>;     ///< Wrapped kernel handle type
    using native_kernel_bundle_t = ze_module_handle_t;      ///< Native kernel bundle handle type
    using native_kernel_t = ze_kernel_handle_t;             ///< Native kernel handle type
    using argument_handler_t = level_zero_argument_handler; ///< Argument handler type
    using command_list_t = ze_command_list_handle_t;        ///< Command list type
    using native_event_t = ze_event_handle_t;               ///< Event type
    using event_t = native_event_t;                         ///< Event type
    using work_group_size_t = work_group_size_dummy;        ///< Work group size type
    constexpr static bool is_event_managed = false; ///< Determines submit function signature

    /**
     * @brief Get the native handle from a shared_handle
     *
     * @tparam T shared_handle type
     * @param wrapped_obj shared_handle
     *
     * @return native handle
     */
    template <typename T> static auto get(T &wrapped_obj) -> typename T::native_type {
        return wrapped_obj.get();
    }

    //! Create argument handler
    inline static auto make_argument_handler(device_t) -> argument_handler_t { return {}; }

    /**
     * @brief Create a kernel bundle for a binary
     *
     * @param binary Pointer to binary
     * @param binary_size Size of binary
     * @param format Binary format (SPIR-V or native)
     * @param core_features Required core features
     * @param context Context
     * @param device Device
     *
     * @return Kernel bundle
     */
    inline static auto make_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                                          binary const &bin) -> kernel_bundle_t {
        ze_module_handle_t mod;
        TINYTC_ZE_CHECK(::tinytc_ze_module_create(&mod, context, device, bin.get(), nullptr));
        return {mod};
    }
    /**
     * @brief Create a kernel
     *
     * @param mod Native kernel bundle handle
     * @param name Kernel name
     *
     * @return Kernel
     */
    inline static auto make_kernel(ze_module_handle_t mod, char const *name) -> kernel_t {
        ze_kernel_handle_t kernel;
        std::uint32_t x, y, z;
        ze_kernel_desc_t kernel_desc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, name};
        TINYTC_ZE_CHECK(zeKernelCreate(mod, &kernel_desc, &kernel));
        TINYTC_ZE_CHECK(::tinytc_ze_get_group_size(kernel, &x, &y, &z));
        TINYTC_ZE_CHECK(zeKernelSetGroupSize(kernel, x, y, z));
        return {kernel};
    }

    /**
     * @brief Work group size is set on kernel creation; therefore only a dummy is returned
     *
     * @return work group size dummy
     */
    inline static auto work_group_size(ze_kernel_handle_t) -> work_group_size_t { return {}; }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * @param work_group_size Work-group size
     * @param howmany Group size
     * @param krnl Native kernel handle
     * @param q Command list
     * @param signal_event Event that is signalled on kernel completion
     * @param num_wait_events Number of events that need to be waited on before execution
     * @param wait_events Pointer to num_wait_events event handles
     */
    inline static void submit(work_group_size_t const &, std::uint32_t howmany,
                              native_kernel_t krnl, command_list_t q,
                              native_event_t signal_event = nullptr,
                              std::uint32_t num_wait_events = 0,
                              native_event_t *wait_events = nullptr) {
        auto group_count = ::tinytc_ze_get_group_count(howmany);
        TINYTC_ZE_CHECK(zeCommandListAppendLaunchKernel(q, krnl, &group_count, signal_event,
                                                        num_wait_events, wait_events));
    }
};

tensor_kernel_bundle(binary const &bin, ze_context_handle_t ctx, ze_device_handle_t dev)
    ->tensor_kernel_bundle<level_zero_runtime>;

} // namespace tinytc

#endif // TINYTC_ZE_20240416_HPP
