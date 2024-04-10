// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_RUNTIME_20240308_HPP
#define SYCL_RUNTIME_20240308_HPP

#include "tinytc/sycl/kernel.hpp"

#include "tinytc/export.h"
#include "tinytc/tensor_kernel.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <sycl/sycl.hpp>
#include <utility>
#include <vector>

namespace tinytc {

class binary;

/**
 * @brief SYCL runtime
 */
class TINYTC_EXPORT sycl_runtime {
  public:
    using context_t = sycl::context; ///< Context type
    using device_t = sycl::device;   ///< Device type
    using kernel_bundle_t =
        sycl::kernel_bundle<sycl::bundle_state::executable>; ///< Kernel bundle type
    using kernel_t = sycl::kernel;                           ///< Kernel type
    using native_kernel_bundle_t = kernel_bundle_t;          ///< Kernel bundle type
    using native_kernel_t = kernel_t;                        ///< Kernel type
    using argument_handler_t = sycl_argument_handler;        ///< Argument handler type
    using command_list_t = sycl::queue;                      ///< Queue type
    using event_t = sycl::event;                             ///< Event type
    using native_event_t = event_t;                          ///< Event type
    using mem_t = void *;                                    ///< Memory object type
    using const_mem_t = const void *;                        ///< Const memory object type
    constexpr static bool is_event_managed = true; ///< Determines submit function signature

    //! Identify function
    template <typename T> static auto get(T &&wrapped_obj) -> T && {
        return std::forward<T>(wrapped_obj);
    }
    //! Create argument handler
    inline static auto make_argument_handler(device_t dev) -> argument_handler_t {
        return {dev.get_platform()};
    }

    /**
     * @brief Create a kernel bundle for a binary
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
    inline static auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size,
                                          bundle_format format, std::uint32_t core_features,
                                          sycl::context ctx, sycl::device dev) {
        return ::tinytc::make_kernel_bundle(binary, binary_size, format, core_features,
                                            std::move(ctx), std::move(dev));
    }
    /**
     * @brief Create a kernel
     *
     * @param mod Native kernel bundle handle
     * @param name Kernel name
     *
     * @return Kernel
     */
    inline static auto make_kernel(native_kernel_bundle_t mod, char const *name) {
        return ::tinytc::make_kernel(std::move(mod), name);
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * @param work_group_size Work-group size
     * @param howmany Group size
     * @param krnl Native kernel handle
     * @param q Queue
     * @param dep_events Vector of events that need to be waited on before execution
     *
     * @return Event
     */
    static auto submit(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
                       native_kernel_t krnl, command_list_t q,
                       std::vector<native_event_t> const &dep_events = {}) -> event_t;
};

tensor_kernel_bundle(std::shared_ptr<binary> bin, sycl::context ctx, sycl::device dev)
    ->tensor_kernel_bundle<sycl_runtime>;

} // namespace tinytc

#endif // SYCL_RUNTIME_20240308_HPP
