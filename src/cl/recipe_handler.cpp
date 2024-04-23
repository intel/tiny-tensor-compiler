// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe_handler.hpp"
#include "error.hpp"
#include "tinytc/tinytc.hpp"

#include <utility>

namespace tinytc {

cl_recipe_handler::cl_recipe_handler(recipe rec, cl_context context, cl_device_id device)
    : ::tinytc_recipe_handler(std::move(rec)) {

    module_ = create_program(context, device, get_recipe().get_binary());

    auto const num_kernels = get_recipe()->num_kernels();
    local_size_.reserve(num_kernels);
    kernels_.reserve(num_kernels);
    for (std::uint32_t num = 0; num < num_kernels; ++num) {
        kernels_.emplace_back(create_kernel(module_.get(), get_recipe()->kernel_name(num)));
        local_size_.emplace_back(get_group_size(kernels_.back().get()));
    }

    cl_platform_id platform;
    CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(platform), &platform, nullptr));
    clSetKernelArgMemPointerINTEL_ =
        (clSetKernelArgMemPointerINTEL_t)clGetExtensionFunctionAddressForPlatform(
            platform, "clSetKernelArgMemPointerINTEL");
}

void cl_recipe_handler::active_kernel(std::uint32_t kernel_num) {
    if (kernel_num >= kernels_.size()) {
        throw status::out_of_range;
    }
    active_kernel_ = kernel_num;
}
void cl_recipe_handler::arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) {
    CL_CHECK_STATUS(clSetKernelArg(kernel(), arg_index, arg_size, arg_value));
}
void cl_recipe_handler::mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) {
    switch (mem.type) {
    case tinytc_mem_type_buffer:
        arg(arg_index, sizeof(mem.value), &mem.value);
        break;
    case tinytc_mem_type_usm_pointer:
        if (clSetKernelArgMemPointerINTEL_ == nullptr) {
            throw status::unavailable_extension;
        }
        CL_CHECK_STATUS(clSetKernelArgMemPointerINTEL_(kernel(), arg_index, mem.value));
        arg(arg_index, sizeof(mem.value), &mem.value);
        break;
    case tinytc_mem_type_svm_pointer:
        CL_CHECK_STATUS(clSetKernelArgSVMPointer(kernel(), arg_index, mem.value));
        break;
    default:
        break;
    }
    throw status::invalid_arguments;
}

void cl_recipe_handler::howmany(std::uint32_t num) {
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
                                                tinytc_recipe_t rec, cl_context context,
                                                cl_device_id device) {
    if (handler == nullptr || rec == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code_cl([&] {
        *handler = std::make_unique<tinytc::cl_recipe_handler>(recipe(rec, true), context, device)
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
