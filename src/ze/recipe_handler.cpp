// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe_handler.hpp"
#include "../recipe.hpp"
#include "error.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc_ze.h"
#include "tinytc/tinytc_ze.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <level_zero/ze_api.h>
#include <memory>
#include <utility>

namespace tinytc {

ze_recipe_handler::ze_recipe_handler(ze_context_handle_t context, ze_device_handle_t device,
                                     shared_handle<tinytc_recipe_t> rec)
    : ::tinytc_recipe_handler(std::move(rec)) {

    module_ = make_kernel_bundle(context, device, get_binary(get_recipe()).get());

    auto const num_kernels = get_recipe()->num_kernels();
    kernels_.reserve(num_kernels);
    for (int num = 0; num < num_kernels; ++num) {
        kernels_.emplace_back(make_kernel(module_.get(), get_recipe()->kernel_name(num)));
    }
}

void ze_recipe_handler::active_kernel(int kernel_num) {
    if (kernel_num < 0 || static_cast<std::size_t>(kernel_num) >= kernels_.size()) {
        throw status::out_of_range;
    }
    active_kernel_ = kernel_num;
}
void ze_recipe_handler::arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) {
    ZE_CHECK_STATUS(
        zeKernelSetArgumentValue(kernels_[active_kernel_].get(), arg_index, arg_size, arg_value));
}
void ze_recipe_handler::mem_arg(std::uint32_t arg_index, const void *value, tinytc_mem_type_t) {
    arg(arg_index, sizeof(value), &value);
}

void ze_recipe_handler::howmany(std::int64_t num) {
    group_count_ = ze_group_count_t{static_cast<std::uint32_t>(num), 1u, 1u};
}

auto ze_recipe_handler::kernel() -> ze_kernel_handle_t { return kernels_[active_kernel_].get(); }
auto ze_recipe_handler::group_count() const -> ze_group_count_t const & { return group_count_; }

} // namespace tinytc

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_ze_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                ze_context_handle_t context,
                                                ze_device_handle_t device, tinytc_recipe_t rec) {
    if (handler == nullptr || rec == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code_ze([&] {
        *handler =
            std::make_unique<tinytc::ze_recipe_handler>(context, device, shared_handle{rec, true})
                .release();
    });
}

tinytc_status_t tinytc_ze_recipe_handler_submit(tinytc_recipe_handler_t handler,
                                                ze_command_list_handle_t list,
                                                ze_event_handle_t signal_event,
                                                uint32_t num_wait_events,
                                                ze_event_handle_t *wait_events) {
    if (handler == nullptr || (num_wait_events > 0 && wait_events == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    auto ze_handler = dynamic_cast<tinytc::ze_recipe_handler *>(handler);
    if (ze_handler == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    auto const &group_count = ze_handler->group_count();
    TINYTC_ZE_CHECK_STATUS(zeCommandListAppendLaunchKernel(
        list, ze_handler->kernel(), &group_count, signal_event, num_wait_events, wait_events));
    return tinytc_status_success;
}
}
