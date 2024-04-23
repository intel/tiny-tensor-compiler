// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe_handler.hpp"
#include "error.hpp"
#include "tinytc/tinytc_sycl.hpp"

#include <utility>

namespace tinytc {

sycl_recipe_handler_impl::sycl_recipe_handler_impl(recipe rec, sycl::context const &context,
                                                   sycl::device const &device)
    : ::tinytc_recipe_handler(std::move(rec)),
      module_(create_kernel_bundle(context, device, get_recipe().get_binary())) {

    auto const num_kernels = get_recipe()->num_kernels();
    kernels_.reserve(num_kernels);
    for (std::uint32_t num = 0; num < num_kernels; ++num) {
        kernels_.emplace_back(create_kernel(module_, get_recipe()->kernel_name(num)));
    }
}

void sycl_recipe_handler_impl::active_kernel(std::uint32_t kernel_num) {
    if (kernel_num >= kernels_.size()) {
        throw status::out_of_range;
    }
    active_kernel_ = kernel_num;
}
void sycl_recipe_handler_impl::arg(std::uint32_t arg_index, std::size_t arg_size,
                                   const void *arg_value) {
    /*ZE_CHECK_STATUS(
        zeKernelSetArgumentValue(kernels_[active_kernel_].get(), arg_index, arg_size, arg_value));

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
    throw status::unsupported_backend;*/
}
void sycl_recipe_handler_impl::mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) {
    arg(arg_index, sizeof(mem.value), &mem.value);
}

void sycl_recipe_handler_impl::howmany(std::uint32_t num) {
    // group_count_ = ::tinytc_ze_get_group_count(num);
}

// auto sycl_recipe_handler_impl::kernel() -> ze_kernel_handle_t { return
// kernels_[active_kernel_].get(); } auto sycl_recipe_handler_impl::group_count() const ->
// ze_group_count_t const & { return group_count_; }

} // namespace tinytc

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_sycl_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                  tinytc_recipe_t rec, const void *ctx,
                                                  const void *dev) {
    if (handler == nullptr || rec == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code_sycl([&] {
        auto const &sycl_ctx = *static_cast<sycl::context const *>(ctx);
        auto const &sycl_dev = *static_cast<sycl::device const *>(dev);
        *handler = std::make_unique<tinytc::sycl_recipe_handler_impl>(recipe(rec, true), sycl_ctx,
                                                                      sycl_dev)
                       .release();
    });
}
}
