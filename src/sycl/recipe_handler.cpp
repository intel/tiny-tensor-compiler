// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe_handler.hpp"
#include "dispatch.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/tinytc_sycl.hpp"
#include "tinytc/tinytc_ze.hpp"

#include <utility>

namespace tinytc {

template <sycl::backend B> struct arg_handler_dispatcher;
template <> struct arg_handler_dispatcher<sycl::backend::ext_oneapi_level_zero> {
    auto operator()(sycl::device const &) -> std::unique_ptr<sycl_argument_handler> {
        return std::make_unique<sycl_argument_handler_level_zero_backend>();
    }
};
template <> struct arg_handler_dispatcher<sycl::backend::opencl> {
    auto operator()(sycl::device const &dev) -> std::unique_ptr<sycl_argument_handler> {
        return std::make_unique<sycl_argument_handler_opencl_backend>(dev.get_platform());
    }
};

sycl_recipe_handler_impl::sycl_recipe_handler_impl(sycl::context const &context,
                                                   sycl::device const &device, recipe rec)
    : ::tinytc_recipe_handler(std::move(rec)),
      module_(make_kernel_bundle(context, device, get_recipe().get_binary())) {

    auto const num_kernels = get_recipe()->num_kernels();
    kernels_.reserve(num_kernels);
    local_size_.reserve(num_kernels);
    for (int num = 0; num < num_kernels; ++num) {
        kernels_.emplace_back(make_kernel(module_, get_recipe()->kernel_name(num)));
        local_size_.emplace_back(get_group_size(kernels_.back()));
    }

    arg_handler_ = dispatch<arg_handler_dispatcher>(device.get_backend(), device);
}

void sycl_recipe_handler_impl::active_kernel(int kernel_num) {
    if (kernel_num < 0 || static_cast<std::size_t>(kernel_num) >= kernels_.size()) {
        throw status::out_of_range;
    }
    active_kernel_ = kernel_num;
}

void sycl_recipe_handler_impl::arg(std::uint32_t arg_index, std::size_t arg_size,
                                   const void *arg_value) {
    arg_handler_->set_arg(kernel(), arg_index, arg_size, arg_value);
}
void sycl_recipe_handler_impl::mem_arg(std::uint32_t arg_index, const void *value,
                                       tinytc_mem_type_t type) {
    arg_handler_->set_mem_arg(kernel(), arg_index, value, type);
}

void sycl_recipe_handler_impl::howmany(std::int64_t num) {
    const auto ls = local_size();
    execution_range_ = sycl::nd_range{
        get_global_size(sycl::range<3u>(1u, 1u, static_cast<std::size_t>(num)), ls), ls};
}

auto sycl_recipe_handler_impl::kernel() const -> sycl::kernel const & {
    return kernels_[active_kernel_];
}
auto sycl_recipe_handler_impl::local_size() const -> sycl::range<3u> const & {
    return local_size_[active_kernel_];
}

auto make_recipe_handler(sycl::context const &ctx, sycl::device const &dev, recipe const &rec)
    -> sycl_recipe_handler {
    tinytc_recipe_handler_t handler =
        std::make_unique<sycl_recipe_handler_impl>(ctx, dev, rec).release();
    return sycl_recipe_handler{handler};
}

auto make_recipe_handler(sycl::queue const &q, recipe const &rec) -> sycl_recipe_handler {
    tinytc_recipe_handler_t handler =
        std::make_unique<sycl_recipe_handler_impl>(q.get_context(), q.get_device(), rec).release();
    return sycl_recipe_handler{handler};
}

void sycl_recipe_handler::parallel_for(sycl::handler &h) {
    auto recipe_handler = dynamic_cast<tinytc::sycl_recipe_handler_impl *>(obj_);
    if (recipe_handler == nullptr) {
        throw status::invalid_arguments;
    }

    h.parallel_for(recipe_handler->execution_range(), recipe_handler->kernel());
}

auto sycl_recipe_handler::submit(sycl::queue q) -> sycl::event {
    return q.submit([&](sycl::handler &h) { parallel_for(h); });
}

auto sycl_recipe_handler::submit(sycl::queue q, sycl::event const &dep_event) -> sycl::event {
    return q.submit([&](sycl::handler &h) {
        h.depends_on(dep_event);
        parallel_for(h);
    });
}

auto sycl_recipe_handler::submit(sycl::queue q, std::vector<sycl::event> const &dep_events)
    -> sycl::event {
    return q.submit([&](sycl::handler &h) {
        h.depends_on(dep_events);
        parallel_for(h);
    });
}

} // namespace tinytc
