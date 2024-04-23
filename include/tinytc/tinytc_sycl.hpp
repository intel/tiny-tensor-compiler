// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_SYCL_20240403_HPP
#define TINYTC_SYCL_20240403_HPP

#include "tinytc.hpp"
#include "tinytc_cl.hpp"
#include "tinytc_sycl.h"
#include "tinytc_ze.hpp"

#include <cstdint>
#include <sycl/sycl.hpp>
#include <utility>

namespace tinytc {

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from SYCL runtime
 *
 * @param device [in] device handle
 *
 * @return core info
 */
inline auto create_core_info(::sycl::device dev) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(::tinytc_sycl_core_info_create(&info, &dev));
    return core_info{info};
}

////////////////////////////
////////// Kernel //////////
////////////////////////////

inline auto create_kernel_bundle(::sycl::context ctx, sycl::device dev, binary const &bin)
    -> ::sycl::kernel_bundle<::sycl::bundle_state::executable> {
    using namespace ::sycl;
    switch (dev.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_context = get_native<backend::ext_oneapi_level_zero, context>(ctx);
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        auto native_mod = create_module(native_context, native_device, bin, nullptr);
        return make_kernel_bundle<backend::ext_oneapi_level_zero, bundle_state::executable>(
            {native_mod.release(), ext::oneapi::level_zero::ownership::transfer}, ctx);
    }
    case backend::opencl: {
        auto native_context = get_native<backend::opencl, context>(ctx);
        auto native_device = get_native<backend::opencl, device>(dev);
        auto native_mod = create_program(native_context, native_device, bin);
        auto bundle =
            make_kernel_bundle<backend::opencl, bundle_state::executable>(native_mod.get(), ctx);
        CL_CHECK_STATUS(clReleaseDevice(native_device));
        CL_CHECK_STATUS(clReleaseContext(native_context));
        return bundle;
    }
    default:
        break;
    }
    throw status::unsupported_backend;
}

inline auto create_kernel(::sycl::kernel_bundle<::sycl::bundle_state::executable> bundle,
                          char const *name) -> ::sycl::kernel {
    using namespace ::sycl;
    switch (bundle.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_bundle =
            get_native<backend::ext_oneapi_level_zero, bundle_state::executable>(bundle);
        auto native_kernel = create_kernel(native_bundle.front(), name);
        return make_kernel<backend::ext_oneapi_level_zero>(
            {bundle, native_kernel.release(), ext::oneapi::level_zero::ownership::transfer},
            bundle.get_context());
    }
    case backend::opencl: {
        auto native_bundle = get_native<backend::opencl, bundle_state::executable>(bundle);
        auto native_kernel = opencl_runtime::make_kernel(native_bundle.front(), name);
        auto kernel = make_kernel<backend::opencl>(native_kernel.get(), bundle.get_context());
        for (auto &m : native_bundle) {
            CL_CHECK_STATUS(clReleaseProgram(m));
        }
        return kernel;
    }
    default:
        break;
    }
    throw status::unsupported_backend;
}

inline auto get_group_size(::sycl::kernel const &krnl) -> ::sycl::range<3u> {
    using namespace ::sycl;
    switch (krnl.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_krnl = get_native<backend::ext_oneapi_level_zero>(krnl);
        auto gs = get_group_size(native_krnl);
        return {static_cast<std::size_t>(gs[2]), static_cast<std::size_t>(gs[1]),
                static_cast<std::size_t>(gs[0])};
    }
    case backend::opencl: {
        auto native_krnl = get_native<backend::opencl>(krnl);
        auto gs = get_group_size(native_krnl);
        CL_CHECK_STATUS(clReleaseKernel(native_krnl));
        return {gs[2], gs[1], gs[0]};
    }
    default:
        break;
    }
    throw status::unsupported_backend;
}

inline auto get_global_size(std::uint32_t howmany, ::sycl::range<3u> const &local_size)
    -> ::sycl::range<3u> {
    return {howmany * local_size[0], local_size[1], local_size[2]};
}

inline auto get_execution_range(::sycl::kernel const &krnl, std::uint32_t howmany)
    -> ::sycl::nd_range<3u> {
    auto local_size = get_group_size(krnl);
    return {get_global_size(howmany, local_size), local_size};
}

////////////////////////////
////////// Runtime /////////
////////////////////////////

/**
 * @brief Wrapper for setting kernel arguments
 */
class sycl_argument_handler {
  public:
    //! ctor
    inline sycl_argument_handler(::sycl::platform plat) {
        using namespace ::sycl;
        switch (plat.get_backend()) {
        case backend::opencl: {
            auto native_plat = get_native<backend::opencl, platform>(std::move(plat));
            cl_arg_ = opencl_argument_handler(native_plat);
            break;
        }
        default:
            break;
        }
    }

    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    inline void set_arg(::sycl::kernel krnl, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) {
        using namespace ::sycl;
        switch (krnl.get_backend()) {
        case backend::ext_oneapi_level_zero: {
            auto native_krnl = get_native<backend::ext_oneapi_level_zero, kernel>(std::move(krnl));
            ze_arg_.set_arg(native_krnl, arg_index, arg_size, arg_value);
            return;
        }
        case backend::opencl: {
            auto native_krnl = get_native<backend::opencl, kernel>(std::move(krnl));
            cl_arg_.set_arg(native_krnl, arg_index, arg_size, arg_value);
            CL_CHECK_STATUS(clReleaseKernel(native_krnl));
            return;
        }
        default:
            break;
        }
        throw status::unsupported_backend;
    }

    /**
     * @brief Set single kernel argument
     *
     * @tparam T Argument type
     * @param kernel Kernel
     * @param arg_index Argument index
     * @param arg Argument value
     */
    template <typename T> void set_arg(sycl::kernel kernel, std::uint32_t arg_index, T const &arg) {
        set_arg(std::move(kernel), arg_index, sizeof(T), &arg);
    }

    /**
     * @brief Set multiple kernel arguments
     *
     * @tparam T Argument types
     * @param kernel Kernel
     * @param ...args Argument values
     */
    template <typename... T> void set_args(sycl::kernel kernel, T const &...args) {
        set_args_helper(std::move(kernel), 0, args...);
    }

  private:
    template <typename Head, typename... Tail>
    void set_args_helper(sycl::kernel kernel, std::uint32_t arg_index, Head &head,
                         Tail const &...tail) {
        if constexpr (sizeof...(Tail) > 0) {
            set_arg(kernel, arg_index, head);
            set_args_helper(std::move(kernel), arg_index + 1, tail...);
        } else {
            set_arg(std::move(kernel), arg_index, head);
        }
    }

    level_zero_argument_handler ze_arg_;
    opencl_argument_handler cl_arg_;
};

/**
 * @brief SYCL runtime
 */
class sycl_runtime {
  public:
    using context_t = ::sycl::context; ///< Context type
    using device_t = ::sycl::device;   ///< Device type
    using kernel_bundle_t =
        ::sycl::kernel_bundle<::sycl::bundle_state::executable>; ///< Kernel bundle type
    using kernel_t = ::sycl::kernel;                             ///< Kernel type
    using native_kernel_bundle_t = kernel_bundle_t;              ///< Kernel bundle type
    using native_kernel_t = kernel_t;                            ///< Kernel type
    using argument_handler_t = sycl_argument_handler;            ///< Argument handler type
    using command_list_t = ::sycl::queue;                        ///< Queue type
    using event_t = ::sycl::event;                               ///< Event type
    using native_event_t = event_t;                              ///< Event type
    using mem_t = void *;                                        ///< Memory object type
    using const_mem_t = const void *;                            ///< Const memory object type
    using work_group_size_t = ::sycl::range<3u>;                 ///< Work group size type
    constexpr static bool is_event_managed = true; ///< Determines submit function signature

    //! Identify function
    template <typename T> static auto get(T &&wrapped_obj) -> T && {
        return std::forward<T>(wrapped_obj);
    }
    //! Create argument handler
    inline static auto make_argument_handler(device_t dev) -> argument_handler_t {
        return {dev.get_platform()};
    }

    /**
     * @brief Create a kernel bundle for a binary
     *
     * @param binary Pointer to binary
     * @param binary_size Size of binary
     * @param format Binary format (SPIR-V or native)
     * @param core_features Required core features
     * @param ctx Context
     * @param dev Device
     *
     * @return Kernel bundle
     */
    inline static auto make_kernel_bundle(context_t ctx, device_t dev, binary const &bin) {
        using namespace ::sycl;
        switch (dev.get_backend()) {
        case backend::ext_oneapi_level_zero: {
            auto native_context = get_native<backend::ext_oneapi_level_zero, context>(ctx);
            auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
            auto mod = level_zero_runtime::make_kernel_bundle(native_context, native_device, bin);
            return make_kernel_bundle<backend::ext_oneapi_level_zero, bundle_state::executable>(
                {mod.release(), ext::oneapi::level_zero::ownership::transfer}, ctx);
        }
        case backend::opencl: {
            auto native_context = get_native<backend::opencl, context>(ctx);
            auto native_device = get_native<backend::opencl, device>(dev);
            auto mod = opencl_runtime::make_kernel_bundle(native_context, native_device, bin);
            auto bundle =
                make_kernel_bundle<backend::opencl, bundle_state::executable>(mod.get(), ctx);
            CL_CHECK_STATUS(clReleaseDevice(native_device));
            CL_CHECK_STATUS(clReleaseContext(native_context));
            return bundle;
        }
        default:
            break;
        }
        throw status::unsupported_backend;
    }
    /**
     * @brief Create a kernel
     *
     * @param mod Native kernel bundle handle
     * @param name Kernel name
     *
     * @return Kernel
     */
    inline static auto make_kernel(native_kernel_bundle_t mod, char const *name) {
        using namespace ::sycl;
        switch (mod.get_backend()) {
        case backend::ext_oneapi_level_zero: {
            auto native_mod =
                get_native<backend::ext_oneapi_level_zero, bundle_state::executable>(mod);
            auto native_kernel = level_zero_runtime::make_kernel(native_mod.front(), name);
            return make_kernel<backend::ext_oneapi_level_zero>(
                {mod, native_kernel.release(), ext::oneapi::level_zero::ownership::transfer},
                mod.get_context());
        }
        case backend::opencl: {
            auto native_mod = get_native<backend::opencl, bundle_state::executable>(mod);
            auto native_kernel = opencl_runtime::make_kernel(native_mod.front(), name);
            auto kernel = make_kernel<backend::opencl>(native_kernel.get(), mod.get_context());
            for (auto &m : native_mod) {
                CL_CHECK_STATUS(clReleaseProgram(m));
            }
            return kernel;
        }
        default:
            break;
        }
        throw status::unsupported_backend;
    }

    //! @brief Get work group size
    inline static auto work_group_size(native_kernel_t krnl, device_t) -> work_group_size_t {
        using namespace ::sycl;
        switch (krnl.get_backend()) {
        case backend::ext_oneapi_level_zero: {
            auto native_krnl = get_native<backend::ext_oneapi_level_zero, kernel>(std::move(krnl));
            uint32_t x, y, z;
            CHECK_STATUS(tinytc_ze_get_group_size(native_krnl, &x, &y, &z));
            return range{z, y, x};
        }
        case backend::opencl: {
            auto native_krnl = get_native<backend::opencl, kernel>(std::move(krnl));
            auto group_size = get_group_size(native_krnl);
            CL_CHECK_STATUS(clReleaseKernel(native_krnl));
            return range{group_size[2], group_size[1], group_size[0]};
        }
        default:
            break;
        }
        throw status::unsupported_backend;
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * @param work_group_size Work-group size
     * @param howmany Group size
     * @param krnl Native kernel handle
     * @param q Queue
     * @param dep_events Vector of events that need to be waited on before execution
     *
     * @return Event
     */
    static auto submit(work_group_size_t const &lws, std::size_t howmany, native_kernel_t krnl,
                       command_list_t q, std::vector<native_event_t> const &dep_events = {})
        -> event_t {
        return q.submit([&](::sycl::handler &h) {
            h.depends_on(dep_events);
            h.parallel_for(get_global_size(howmany, lws), std::move(krnl));
        });
    }
};

tensor_kernel_bundle(binary const &bin, sycl::context ctx, sycl::device dev)
    -> tensor_kernel_bundle<sycl_runtime>;

////////////////////////////
////////// Recipe //////////
////////////////////////////

class sycl_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    inline sycl_recipe_handler(recipe const &rec, ::sycl::context const &ctx,
                               ::sycl::device const &dev) {
        CHECK_STATUS(tinytc_sycl_recipe_handler_create(&obj_, rec.get(), &ctx, &dev));
    }

    // inline auto submit(::sycl::queue q, std::vector<::sycl::queue> const &dep_events)
    //-> ::sycl::event {
    // return q.submit([&](::sycl::handler &h) {
    // h.depends_on(dep_events);
    // h.parallel_for(get_execution_range(howmany, lws), std::move(krnl));
    //});
    //}
};

} // namespace tinytc

#endif // TINYTC_SYCL_20240403_HPP
