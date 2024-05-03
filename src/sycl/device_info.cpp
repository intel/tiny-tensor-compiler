// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "dispatch.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/tinytc_sycl.hpp"
#include "tinytc/tinytc_ze.hpp"

#include <sycl/sycl.hpp>

using namespace sycl;

namespace tinytc {

template <sycl::backend B> struct support_level_dispatcher {
    auto operator()(device const &dev) {
        auto native_device = get_native<B, device>(dev);
        auto level = get_support_level(native_device);
        dispatch_traits<B>::release(native_device);
        return level;
    }
};

template <sycl::backend B> struct core_info_dispatcher {
    auto operator()(device const &dev) {
        auto native_device = get_native<B, device>(dev);
        auto info = make_core_info(native_device);
        dispatch_traits<B>::release(native_device);
        return info;
    }
};

auto get_support_level(device const &dev) -> support_level {
    return dispatch<support_level_dispatcher>(dev.get_backend(), dev);
}

auto make_core_info(device const &dev) -> core_info {
    return dispatch<core_info_dispatcher>(dev.get_backend(), dev);
}

} // namespace tinytc
