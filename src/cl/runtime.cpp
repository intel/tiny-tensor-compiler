// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/cl/runtime.hpp"
#include "tinytc/cl/error.hpp"

namespace tinytc {

auto opencl_runtime::make_argument_handler(device_t dev) -> argument_handler_t {
    cl_platform_id plat;
    CL_CHECK(clGetDeviceInfo(dev, CL_DEVICE_PLATFORM, sizeof(plat), &plat, nullptr));
    return {plat};
}

auto opencl_runtime::submit(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
                            native_kernel_t krnl, command_list_t q,
                            std::vector<native_event_t> const &dep_events) -> event_t {
    auto range = get_opencl_nd_range(work_group_size, howmany);
    cl_event ev;
    CL_CHECK(clEnqueueNDRangeKernel(q, krnl, range.dim, nullptr, range.global_work_size.data(),
                                    range.local_work_size.data(), dep_events.size(),
                                    dep_events.data(), &ev));
    return opencl_object_wrapper<cl_event>(ev);
}

} // namespace tinytc
