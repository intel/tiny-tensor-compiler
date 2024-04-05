// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/sycl/runtime.hpp"

using namespace sycl;

namespace tinytc {

auto sycl_runtime::submit(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
                          kernel_t krnl, command_list_t q,
                          std::vector<native_event_t> const &dep_events) -> event_t {
    return q.submit([&](handler &h) {
        h.depends_on(dep_events);
        h.parallel_for(get_sycl_nd_range(work_group_size, howmany), krnl);
    });
}

} // namespace tinytc
