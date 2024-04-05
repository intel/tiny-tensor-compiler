// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ze/runtime.hpp"
#include "tinytc/ze/error.hpp"

#include <utility>

namespace tinytc {

auto level_zero_runtime::wrap(native_kernel_bundle_t mod) -> kernel_bundle_t {
    return shared_handle<native_kernel_bundle_t>(
        std::move(mod), [](native_kernel_bundle_t mod) { zeModuleDestroy(mod); });
}

auto level_zero_runtime::wrap(native_kernel_t kernel) -> kernel_t {
    return shared_handle<native_kernel_t>(std::move(kernel),
                                          [](native_kernel_t kernel) { zeKernelDestroy(kernel); });
}

void level_zero_runtime::submit(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
                                native_kernel_t krnl, command_list_t q, native_event_t signal_event,
                                std::uint32_t num_wait_events, native_event_t *wait_events) {
    ZE_CHECK(zeKernelSetGroupSize(krnl, work_group_size[0], work_group_size[1], 1u));
    auto group_count = get_group_count(howmany);
    ZE_CHECK(zeCommandListAppendLaunchKernel(q, krnl, &group_count, signal_event, num_wait_events,
                                             wait_events));
}

} // namespace tinytc
