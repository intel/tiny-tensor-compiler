// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_RUNTIME_20240308_HPP
#define ZE_RUNTIME_20240308_HPP

#include "tinytc/ze/kernel.hpp"

#include "tinytc/export.hpp"
#include "tinytc/tensor_kernel.hpp"
#include "tinytc/ze/shared_handle.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <level_zero/ze_api.h>
#include <memory>

namespace tinytc {

enum class bundle_format;

class binary;

/**
 * @brief Level Zero runtime
 */
class TINYTC_EXPORT level_zero_runtime {
  public:
    using context_t = ze_context_handle_t; ///< Context handle type
    using device_t = ze_device_handle_t;   ///< Device handle type
    using kernel_bundle_t =
        shared_handle<ze_module_handle_t>;                  ///< Wrapped kernel bundle handle type
    using kernel_t = shared_handle<ze_kernel_handle_t>;     ///< Wrapped kernel handle type
    using native_kernel_bundle_t = ze_module_handle_t;      ///< Native kernel bundle handle type
    using native_kernel_t = ze_kernel_handle_t;             ///< Native kernel handle type
    using argument_handler_t = level_zero_argument_handler; ///< Argument handler type
    using command_list_t = ze_command_list_handle_t;        ///< Command list type
    using native_event_t = ze_event_handle_t;               ///< Event type
    using event_t = native_event_t;                         ///< Event type
    using mem_t = void *;                                   ///< Memory object type
    using const_mem_t = const void *;                       ///< Const memory object type
    constexpr static bool is_event_managed = false; ///< Determines submit function signature

    /**
     * @brief Get the native handle from a shared_handle
     *
     * @tparam T shared_handle type
     * @param wrapped_obj shared_handle
     *
     * @return native handle
     */
    template <typename T> static auto get(T &wrapped_obj) -> typename T::value_type {
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
    inline static auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size,
                                          bundle_format format, std::uint32_t core_features,
                                          ze_context_handle_t context, ze_device_handle_t device)
        -> kernel_bundle_t {
        return wrap(::tinytc::make_kernel_bundle(binary, binary_size, format, core_features,
                                                 context, device));
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
        return wrap(::tinytc::make_kernel(mod, name));
    }

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
    static void submit(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
                       native_kernel_t krnl, command_list_t q,
                       native_event_t signal_event = nullptr, std::uint32_t num_wait_events = 0,
                       native_event_t *wait_events = nullptr);

  private:
    static auto wrap(native_kernel_bundle_t mod) -> kernel_bundle_t;
    static auto wrap(native_kernel_t kernel) -> kernel_t;
};

tensor_kernel_bundle(std::shared_ptr<binary> bin, ze_context_handle_t ctx, ze_device_handle_t dev)
    ->tensor_kernel_bundle<level_zero_runtime>;

} // namespace tinytc

#endif // ZE_RUNTIME_20240308_HPP
