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

inline auto make_kernel_bundle(cl_context context, cl_device_id device, binary const &bin)
    -> shared_handle<cl_program> {
    cl_program obj;
    CHECK_STATUS(tinytc_cl_program_create(&obj, context, device, bin.get()));
    return {obj};
}

inline auto make_kernel(cl_program mod, char const *name) -> shared_handle<cl_kernel> {
    cl_int err;
    cl_kernel obj = clCreateKernel(mod, name, &err);
    CL_CHECK_STATUS(err);
    return {obj};
}

inline auto get_group_size(cl_kernel kernel) -> std::array<std::size_t, 3u> {
    auto group_size = std::array<std::size_t, 3u>{};
    CHECK_STATUS(tinytc_cl_get_group_size(kernel, group_size.data()));
    return group_size;
}

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

template <> struct auto_mem_type<cl_mem> {
    constexpr static mem_type value = mem_type::buffer;
};

class opencl_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    inline auto submit(cl_command_queue queue, uint32_t num_wait_events = 0,
                       cl_event *wait_events = nullptr) -> shared_handle<cl_event> {
        cl_event evt;
        CHECK_STATUS(
            tinytc_cl_recipe_handler_submit(obj_, queue, num_wait_events, wait_events, &evt));
        return {evt};
    }
    inline void submit_no_event(cl_command_queue queue, uint32_t num_wait_events = 0,
                                cl_event *wait_events = nullptr) {
        CHECK_STATUS(
            tinytc_cl_recipe_handler_submit(obj_, queue, num_wait_events, wait_events, NULL));
    }
};

inline auto make_recipe_handler(cl_context context, cl_device_id device, recipe const &rec)
    -> opencl_recipe_handler {
    tinytc_recipe_handler_t handler;
    CHECK_STATUS(tinytc_cl_recipe_handler_create(&handler, context, device, rec.get()));
    return {handler};
}

} // namespace tinytc

#endif // TINYTC_OPENCL_20240403_HPP
