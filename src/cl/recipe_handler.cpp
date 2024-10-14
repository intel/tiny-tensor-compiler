// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe_handler.hpp"
#include "../recipe.hpp"
#include "../reference_counted.hpp"
#include "error.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.h"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <CL/cl_platform.h>
#include <memory>
#include <utility>

namespace tinytc {

cl_recipe_handler::cl_recipe_handler(cl_context context, cl_device_id device, recipe rec)
    : ::tinytc_recipe_handler(std::move(rec)) {

    module_ = make_kernel_bundle(context, device, get_recipe().get_source());

    auto const num_kernels = get_recipe()->num_kernels();
    kernels_.reserve(num_kernels);
    local_size_.reserve(num_kernels);
    for (int num = 0; num < num_kernels; ++num) {
        kernels_.emplace_back(make_kernel(module_.get(), get_recipe()->kernel_name(num)));
        local_size_.emplace_back(get_group_size(kernels_.back().get()));
    }

    cl_platform_id platform;
    CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(platform), &platform, nullptr));
    arg_handler_ = opencl_argument_handler(platform);
}

void cl_recipe_handler::active_kernel(int kernel_num) {
    if (kernel_num < 0 || static_cast<std::size_t>(kernel_num) >= kernels_.size()) {
        throw status::out_of_range;
    }
    active_kernel_ = kernel_num;
}
void cl_recipe_handler::arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) {
    arg_handler_.set_arg(kernel(), arg_index, arg_size, arg_value);
}
void cl_recipe_handler::mem_arg(std::uint32_t arg_index, const void *value,
                                tinytc_mem_type_t type) {
    arg_handler_.set_mem_arg(kernel(), arg_index, value, type);
}

void cl_recipe_handler::howmany(std::int64_t num) {
    global_size_ = get_global_size(num, local_size());
}

auto cl_recipe_handler::kernel() -> cl_kernel { return kernels_[active_kernel_].get(); }

auto cl_recipe_handler::local_size() const -> std::array<std::size_t, 3u> const & {
    return local_size_[active_kernel_];
}

} // namespace tinytc

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_cl_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                cl_context context, cl_device_id device,
                                                tinytc_recipe_t rec) {
    if (handler == nullptr || rec == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code_cl([&] {
        *handler = std::make_unique<tinytc::cl_recipe_handler>(context, device, recipe(rec, true))
                       .release();
    });
}

tinytc_status_t tinytc_cl_recipe_handler_submit(tinytc_recipe_handler_t handler,
                                                cl_command_queue queue, cl_uint num_wait_events,
                                                const cl_event *wait_events, cl_event *event) {
    if (handler == nullptr || (num_wait_events > 0 && wait_events == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    auto cl_handler = dynamic_cast<tinytc::cl_recipe_handler *>(handler);
    if (cl_handler == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    TINYTC_CL_CHECK_STATUS(clEnqueueNDRangeKernel(queue, cl_handler->kernel(), 3u, nullptr,
                                                  cl_handler->global_size().data(),
                                                  cl_handler->local_size().data(), num_wait_events,
                                                  wait_events, event););
    return tinytc_status_success;
}
}
