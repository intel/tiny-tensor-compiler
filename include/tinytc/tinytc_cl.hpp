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
 * @brief Get support level of OpenCL device
 *
 * @param device Device handle
 *
 * @return Support level
 */
inline auto get_support_level(cl_device_id device) -> support_level {
    tinytc_support_level_t level;
    CHECK_STATUS(::tinytc_cl_get_support_level(device, &level));
    return support_level{std::underlying_type_t<support_level>(level)};
}

/**
 * @brief Query core info from OpenCL runtime
 *
 * @param device device handle
 *
 * @return core info
 */
inline auto make_core_info(cl_device_id device) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(::tinytc_cl_core_info_create(&info, device));
    return core_info{info};
}

////////////////////////////
////////// Kernel //////////
////////////////////////////

namespace internal {
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
} // namespace internal

/**
 * @brief Make an OpenCL program from a tinytc source
 *
 * @param context Context
 * @param device Device
 * @param src Source
 * @param source_ctx Source context for improved error reporting
 *
 * @return cl_program (shared handle)
 */
inline auto make_kernel_bundle(cl_context context, cl_device_id device, source const &src,
                               source_context source_ctx = {}) -> shared_handle<cl_program> {
    cl_program obj;
    CHECK_STATUS(tinytc_cl_kernel_bundle_create_with_source(&obj, context, device, src.get(),
                                                            source_ctx.get()));
    return shared_handle<cl_program>{obj};
}

/**
 * @brief Make an OpenCL program from a tinytc program
 *
 * @param context Context
 * @param device Device
 * @param prg Program
 * @param core_features requested core features; must be 0 (default) or a combination of
 * tinytc_core_feature_flag_t
 * @param source_ctx Source context for improved error reporting
 *
 * @return cl_program (shared handle)
 */
inline auto make_kernel_bundle(cl_context context, cl_device_id device, prog prg,
                               tinytc_core_feature_flags_t core_features = 0,
                               source_context source_ctx = {}) -> shared_handle<cl_program> {
    cl_program obj;
    CHECK_STATUS(tinytc_cl_kernel_bundle_create_with_program(&obj, context, device, prg.get(),
                                                             core_features, source_ctx.get()));
    return shared_handle<cl_program>{obj};
}

/**
 * @brief Make an OpenCL program from a tinytc binary
 *
 * @param context Context
 * @param device Device
 * @param bin Binary
 * @param source_ctx Source context for improved error reporting
 *
 * @return cl_program (shared handle)
 */
inline auto make_kernel_bundle(cl_context context, cl_device_id device, binary const &bin,
                               source_context source_ctx = {}) -> shared_handle<cl_program> {
    cl_program obj;
    CHECK_STATUS(tinytc_cl_kernel_bundle_create_with_binary(&obj, context, device, bin.get(),
                                                            source_ctx.get()));
    return shared_handle<cl_program>{obj};
}

/**
 * @brief Make a cl_kernel from a cl_program
 *
 * @param mod Program
 * @param name Kernel name
 *
 * @return cl_kernel (shared handle)
 */
inline auto make_kernel(cl_program mod, char const *name) -> shared_handle<cl_kernel> {
    cl_int err;
    cl_kernel obj = clCreateKernel(mod, name, &err);
    CL_CHECK_STATUS(err);
    return shared_handle<cl_kernel>{obj};
}

/**
 * @brief Get work group size
 *
 * @param kernel Kernel
 *
 * @return Work-group size
 */
inline auto get_group_size(cl_kernel kernel) -> std::array<std::size_t, 3u> {
    auto group_size = std::array<std::size_t, 3u>{};
    CHECK_STATUS(tinytc_cl_get_group_size(kernel, group_size.data()));
    return group_size;
}

/**
 * @brief Convert group size to opencl global range
 *
 * @param howmany Group size
 * @param local_size Work-group size
 *
 * @return Global size
 */
inline auto get_global_size(std::uint32_t howmany, std::array<std::size_t, 3u> const &local_size)
    -> std::array<std::size_t, 3u> {
    auto global_size = std::array<std::size_t, 3u>{};
    tinytc_cl_get_global_size(howmany, local_size.data(), global_size.data());
    return global_size;
}

////////////////////////////
////////// Recipe //////////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<cl_event> {
    static auto retain(cl_event handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clRetainEvent(handle));
    }
    static auto release(cl_event handle) -> tinytc_status_t {
        return ::tinytc_cl_convert_status(clReleaseEvent(handle));
    }
};
} // namespace internal

/**
 * @brief Specialize auto_mem_type for cl_mem
 */
template <> struct auto_mem_type<cl_mem> {
    constexpr static mem_type value = mem_type::buffer; ///< cl_mem maps to buffer type
};

/**
 * @brief Recipe handler for the OpenCL runtime
 */
class opencl_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    /**
     * @brief Submit recipe to queue
     *
     * @param queue Command queue
     * @param num_wait_events Number of events to wait
     * @param wait_events Array of num_wait_events events to wait on
     *
     * @return Event (cl_event wrapped in shared_handle -> cleans up automatically)
     */
    inline auto submit(cl_command_queue queue, uint32_t num_wait_events = 0,
                       cl_event *wait_events = nullptr) -> shared_handle<cl_event> {
        cl_event evt;
        CHECK_STATUS(
            tinytc_cl_recipe_handler_submit(obj_, queue, num_wait_events, wait_events, &evt));
        return shared_handle<cl_event>{evt};
    }
    /**
     * @brief Submit recipe to queue; does not return event
     *
     * @param queue Command queue
     * @param num_wait_events Number of events to wait
     * @param wait_events Array of num_wait_events events to wait on
     */
    inline void submit_no_event(cl_command_queue queue, uint32_t num_wait_events = 0,
                                cl_event *wait_events = nullptr) {
        CHECK_STATUS(
            tinytc_cl_recipe_handler_submit(obj_, queue, num_wait_events, wait_events, NULL));
    }
};

/**
 * @brief Make recipe handler
 *
 * @param context Context
 * @param device Device
 * @param rec Recipe
 * @param source_ctx Source context for improved error reporting
 *
 * @return OpenCL recipe handler
 */
inline auto make_recipe_handler(cl_context context, cl_device_id device, recipe const &rec,
                                source_context source_ctx = {}) -> opencl_recipe_handler {
    tinytc_recipe_handler_t handler;
    CHECK_STATUS(
        tinytc_cl_recipe_handler_create(&handler, context, device, rec.get(), source_ctx.get()));
    return opencl_recipe_handler{handler};
}

} // namespace tinytc

#endif // TINYTC_OPENCL_20240403_HPP
