// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_OPENCL_20240403_HPP
#define TINYTC_OPENCL_20240403_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.h"

#include <CL/cl.h>
#include <array>
#include <type_traits>

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

//! Throw exception for unsuccessful call to C-API and convert result code to tinytc status
inline void CL_CHECK_STATUS(cl_int stat) {
    if (stat != CL_SUCCESS) {
        throw status{std::underlying_type_t<status>(::tinytc_cl_convert_status(stat))};
    }
}

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from OpenCL runtime
 *
 * @param device [in] device handle
 *
 * @return core info
 */
inline auto create_core_info(cl_device_id device) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(::tinytc_cl_core_info_create(&info, device));
    return core_info{info};
}

////////////////////////////
////////// Runtime /////////
////////////////////////////

template <> struct shared_handle_traits<cl_program> {
    static auto retain(cl_program handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clRetainProgram(handle));
    }
    static auto release(cl_program handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clReleaseProgram(handle));
    }
};

template <> struct shared_handle_traits<cl_kernel> {
    static auto retain(cl_kernel handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clRetainKernel(handle));
    }
    static auto release(cl_kernel handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clReleaseKernel(handle));
    }
};

template <> struct shared_handle_traits<cl_event> {
    static auto retain(cl_event handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clRetainEvent(handle));
    }
    static auto release(cl_event handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clReleaseEvent(handle));
    }
};

/**
 * @brief Check whether T is of pointer type but not of cl_mem type
 *
 * @tparam T value type
 */
template <typename T>
concept pointer_kernel_argument =
    std::is_pointer_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, cl_mem>;

/**
 * @brief Check whether T is a regular (non-pointer) argument
 *
 * @tparam T value type
 */
template <typename T>
concept regular_kernel_argument = !pointer_kernel_argument<T>;

/**
 * @brief Wrapper for setting kernel arguments
 */
class opencl_argument_handler {
  public:
    using clSetKernelArgMemPointerINTEL_t =
        cl_int (*)(cl_kernel kernel, cl_uint arg_index,
                   const void *arg_value); ///< Signature of clSetKernelArgMemPointerINTEL function
    //! ctor
    inline opencl_argument_handler() : clSetKernelArgMemPointerINTEL_(nullptr) {}
    //! ctor; checks whether cl_intel_unified_shared_memory is available and gets
    //! clSetKernelArgMemPointerINTEL
    inline opencl_argument_handler(cl_platform_id plat)
        : clSetKernelArgMemPointerINTEL_(
              (clSetKernelArgMemPointerINTEL_t)clGetExtensionFunctionAddressForPlatform(
                  plat, "clSetKernelArgMemPointerINTEL")) {}

    /**
     * @brief Set single pointer kernel argument
     *
     * Throws unavailable_extension status if cl_intel_unified_shared_memory extension is
     * unavailable.
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_value Argument value
     */
    inline void set_arg_mem_pointer(cl_kernel kernel, std::uint32_t arg_index,
                                    void const *arg_value) {
        if (clSetKernelArgMemPointerINTEL_ == nullptr) {
            throw status::unavailable_extension;
        }
        CL_CHECK_STATUS(clSetKernelArgMemPointerINTEL_(kernel, arg_index, arg_value));
    }
    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    inline void set_arg(cl_kernel kernel, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) {
        CL_CHECK_STATUS(clSetKernelArg(kernel, arg_index, arg_size, arg_value));
    }

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

/**
 * @brief OpenCL runtime
 */
class opencl_runtime {
  public:
    using context_t = cl_context;                          ///< Context handle type
    using device_t = cl_device_id;                         ///< Device handle type
    using kernel_bundle_t = shared_handle<cl_program>;     ///< Wrapped kernel bundle handle type
    using kernel_t = shared_handle<cl_kernel>;             ///< Wrapped kernel handle type
    using native_kernel_bundle_t = cl_program;             ///< Native kernel bundle handle type
    using native_kernel_t = cl_kernel;                     ///< Native kernel handle type
    using argument_handler_t = opencl_argument_handler;    ///< Argument handler type
    using command_list_t = cl_command_queue;               ///< Queue type
    using event_t = shared_handle<cl_event>;               ///< Wrapped event handle type
    using native_event_t = cl_event;                       ///< Native event handle type
    using mem_t = cl_mem;                                  ///< Memory object type
    using const_mem_t = const cl_mem;                      ///< Const memory object type
    using work_group_size_t = std::array<std::size_t, 3u>; ///< Work group size type
    constexpr static bool is_event_managed = true;         ///< Determines submit function signature

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
    inline static auto make_argument_handler(device_t dev) -> argument_handler_t {
        cl_platform_id plat;
        CL_CHECK_STATUS(clGetDeviceInfo(dev, CL_DEVICE_PLATFORM, sizeof(plat), &plat, nullptr));
        return {plat};
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
    inline static auto make_kernel_bundle(context_t ctx, device_t dev, binary const &bin)
        -> kernel_bundle_t {
        cl_program mod;
        CHECK_STATUS(::tinytc_cl_program_create(&mod, ctx, dev, bin.get()));
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
    inline static auto make_kernel(native_kernel_bundle_t mod, char const *name) -> kernel_t {
        cl_int err;
        cl_kernel kernel = clCreateKernel(mod, name, &err);
        CL_CHECK_STATUS(err);
        return {kernel};
    }

    //! @brief Get work group size
    inline static auto work_group_size(native_kernel_t kernel, device_t dev) -> work_group_size_t {
        std::size_t x, y, z;
        CHECK_STATUS(tinytc_cl_get_group_size(kernel, dev, &x, &y, &z));
        return {x, y, z};
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
    inline static auto submit(work_group_size_t const &lws, std::size_t howmany,
                              native_kernel_t krnl, command_list_t q,
                              std::vector<native_event_t> const &dep_events = {}) -> event_t {
        std::array<std::size_t, 3u> gws;
        tinytc_cl_get_global_size(howmany, lws[0], lws[1], lws[2], &gws[0], &gws[1], &gws[2]);
        cl_event ev;
        CL_CHECK_STATUS(clEnqueueNDRangeKernel(q, krnl, 3, nullptr, gws.data(), lws.data(),
                                               dep_events.size(), dep_events.data(), &ev));
        return {ev};
    }
};

tensor_kernel_bundle(binary const &bin, cl_context ctx, cl_device_id dev)
    -> tensor_kernel_bundle<opencl_runtime>;

} // namespace tinytc

#endif // TINYTC_OPENCL_20240403_HPP
