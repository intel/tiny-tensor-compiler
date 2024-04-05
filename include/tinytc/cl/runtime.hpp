// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_RUNTIME_20240314_HPP
#define CL_RUNTIME_20240314_HPP

#include "tinytc/cl/kernel.hpp"
#include "tinytc/cl/object_wrapper.hpp"

#include "tinytc/export.hpp"
#include "tinytc/tensor_kernel.hpp"

#include <CL/cl.h>
#include <array>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace tinytc {

class binary;

/**
 * @brief OpenCL runtime
 */
class TINYTC_EXPORT opencl_runtime {
  public:
    using context_t = cl_context;  ///< Context handle type
    using device_t = cl_device_id; ///< Device handle type
    using kernel_bundle_t =
        opencl_object_wrapper<cl_program>;              ///< Wrapped kernel bundle handle type
    using kernel_t = opencl_object_wrapper<cl_kernel>;  ///< Wrapped kernel handle type
    using native_kernel_bundle_t = cl_program;          ///< Native kernel bundle handle type
    using native_kernel_t = cl_kernel;                  ///< Native kernel handle type
    using argument_handler_t = opencl_argument_handler; ///< Argument handler type
    using command_list_t = cl_command_queue;            ///< Queue type
    using event_t = opencl_object_wrapper<cl_event>;    ///< Wrapped event handle type
    using native_event_t = cl_event;                    ///< Native event handle type
    using mem_t = cl_mem;                               ///< Memory object type
    using const_mem_t = const cl_mem;                   ///< Const memory object type
    constexpr static bool is_event_managed = true;      ///< Determines submit function signature

    /**
     * @brief Get the native handle from an object wrapper
     *
     * @tparam T opencl_object_wrapper type
     * @param wrapped_obj opencl_object_wrapper
     *
     * @return native handle
     */
    template <typename T> static auto get(T &wrapped_obj) -> typename T::native_type {
        return wrapped_obj.get();
    }
    //! Create argument handler
    static auto make_argument_handler(device_t dev) -> argument_handler_t;

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
                                          context_t ctx, device_t dev) -> kernel_bundle_t {
        return wrap(
            ::tinytc::make_kernel_bundle(binary, binary_size, format, core_features, ctx, dev));
    }
    /**
     * @brief Create a kernel
     *
     * @param mod Native kernel bundle handle
     * @param name Kernel name
     *
     * @return Kernel
     */
    inline static auto make_kernel(native_kernel_bundle_t mod, char const *name) -> kernel_t {
        return wrap(::tinytc::make_kernel(mod, name));
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

  private:
    template <typename T> static auto wrap(T obj) -> opencl_object_wrapper<T> {
        return opencl_object_wrapper<T>(obj);
    }
};

tensor_kernel_bundle(std::shared_ptr<binary> bin, cl_context ctx, cl_device_id dev)
    ->tensor_kernel_bundle<opencl_runtime>;

} // namespace tinytc

#endif // CL_RUNTIME_20240314_HPP
