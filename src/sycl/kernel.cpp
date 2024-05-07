// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "dispatch.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/tinytc_sycl.hpp"
#include "tinytc/tinytc_ze.hpp"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <sycl/sycl.hpp>
#include <utility>
#include <vector>

using namespace sycl;

namespace tinytc {

template <backend B> struct kernel_bundle_dispatcher;
template <> struct kernel_bundle_dispatcher<backend::ext_oneapi_level_zero> {
    template <typename T>
    auto operator()(context const &ctx, device const &dev, T const &obj,
                    source_context source_ctx) {
        auto native_context = get_native<backend::ext_oneapi_level_zero, context>(ctx);
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        auto native_mod =
            make_kernel_bundle(native_context, native_device, obj, std::move(source_ctx));
        return make_kernel_bundle<backend::ext_oneapi_level_zero, bundle_state::executable>(
            {native_mod.release(), ext::oneapi::level_zero::ownership::transfer}, ctx);
    }
    auto operator()(context const &ctx, device const &dev, prog prg,
                    tinytc_core_feature_flags_t core_features, source_context source_ctx) {
        auto native_context = get_native<backend::ext_oneapi_level_zero, context>(ctx);
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        auto native_mod = make_kernel_bundle(native_context, native_device, std::move(prg),
                                             core_features, std::move(source_ctx));
        return make_kernel_bundle<backend::ext_oneapi_level_zero, bundle_state::executable>(
            {native_mod.release(), ext::oneapi::level_zero::ownership::transfer}, ctx);
    }
};
template <> struct kernel_bundle_dispatcher<backend::opencl> {
    template <typename T>
    auto operator()(context const &ctx, device const &dev, T const &obj,
                    source_context source_ctx) {
        auto native_context = get_native<backend::opencl, context>(ctx);
        auto native_device = get_native<backend::opencl, device>(dev);
        auto native_mod =
            make_kernel_bundle(native_context, native_device, obj, std::move(source_ctx));
        auto bundle =
            make_kernel_bundle<backend::opencl, bundle_state::executable>(native_mod.get(), ctx);
        CL_CHECK_STATUS(clReleaseDevice(native_device));
        CL_CHECK_STATUS(clReleaseContext(native_context));
        return bundle;
    }
    auto operator()(context const &ctx, device const &dev, prog prg,
                    tinytc_core_feature_flags_t core_features, source_context source_ctx) {
        auto native_context = get_native<backend::opencl, context>(ctx);
        auto native_device = get_native<backend::opencl, device>(dev);
        auto native_mod = make_kernel_bundle(native_context, native_device, std::move(prg),
                                             core_features, std::move(source_ctx));
        auto bundle =
            make_kernel_bundle<backend::opencl, bundle_state::executable>(native_mod.get(), ctx);
        CL_CHECK_STATUS(clReleaseDevice(native_device));
        CL_CHECK_STATUS(clReleaseContext(native_context));
        return bundle;
    }
};

auto make_kernel_bundle(context const &ctx, device const &dev, source const &src,
                        source_context source_ctx) -> kernel_bundle<bundle_state::executable> {
    return dispatch<kernel_bundle_dispatcher>(dev.get_backend(), ctx, dev, src,
                                              std::move(source_ctx));
}
auto make_kernel_bundle(context const &ctx, device const &dev, prog prg,
                        tinytc_core_feature_flags_t core_features, source_context source_ctx)
    -> kernel_bundle<bundle_state::executable> {
    return dispatch<kernel_bundle_dispatcher>(dev.get_backend(), ctx, dev, std::move(prg),
                                              core_features, std::move(source_ctx));
}
auto make_kernel_bundle(context const &ctx, device const &dev, binary const &bin,
                        source_context source_ctx) -> kernel_bundle<bundle_state::executable> {
    return dispatch<kernel_bundle_dispatcher>(dev.get_backend(), ctx, dev, bin,
                                              std::move(source_ctx));
}

template <backend B> struct kernel_dispatcher;
template <> struct kernel_dispatcher<backend::ext_oneapi_level_zero> {
    auto operator()(kernel_bundle<bundle_state::executable> const &bundle, char const *name) {
        auto native_bundle =
            get_native<backend::ext_oneapi_level_zero, bundle_state::executable>(bundle);
        auto native_kernel = make_kernel(native_bundle.front(), name);
        return make_kernel<backend::ext_oneapi_level_zero>(
            {bundle, native_kernel.release(), ext::oneapi::level_zero::ownership::transfer},
            bundle.get_context());
    }
};
template <> struct kernel_dispatcher<backend::opencl> {
    auto operator()(kernel_bundle<bundle_state::executable> const &bundle, char const *name) {
        auto native_bundle = get_native<backend::opencl, bundle_state::executable>(bundle);
        auto native_kernel = make_kernel(native_bundle.front(), name);
        auto kernel = make_kernel<backend::opencl>(native_kernel.get(), bundle.get_context());
        for (auto &m : native_bundle) {
            CL_CHECK_STATUS(clReleaseProgram(m));
        }
        return kernel;
    }
};

auto make_kernel(kernel_bundle<bundle_state::executable> const &bundle, char const *name)
    -> kernel {
    return dispatch<kernel_dispatcher>(bundle.get_backend(), bundle, name);
}

template <backend B> struct group_size_dispatcher {
    auto operator()(kernel const &krnl) {
        auto native_krnl = get_native<B>(krnl);
        auto gs = get_group_size(native_krnl);
        dispatch_traits<B>::release(native_krnl);
        return range<3u>{static_cast<std::size_t>(gs[2]), static_cast<std::size_t>(gs[1]),
                         static_cast<std::size_t>(gs[0])};
    }
};

auto get_group_size(kernel const &krnl) -> range<3u> {
    return dispatch<group_size_dispatcher>(krnl.get_backend(), krnl);
}

auto get_global_size(std::int64_t howmany, range<3u> const &local_size) -> range<3u> {
    return {howmany * local_size[0], local_size[1], local_size[2]};
}

auto get_execution_range(kernel const &krnl, std::int64_t howmany) -> nd_range<3u> {
    auto local_size = get_group_size(krnl);
    return {get_global_size(howmany, local_size), local_size};
}

} // namespace tinytc
