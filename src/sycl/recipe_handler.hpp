// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_RECIPE_HANDLER_20240423_HPP
#define SYCL_RECIPE_HANDLER_20240423_HPP

#include "../recipe.hpp"
#include "tinytc/tinytc.hpp"

#include <sycl/sycl.hpp>
#include <vector>

namespace tinytc {

struct sycl_recipe_handler_impl : ::tinytc_recipe_handler {
  public:
    sycl_recipe_handler_impl(recipe rec, sycl::context const &context, sycl::device const &device);

    void active_kernel(std::uint32_t kernel_num) override;
    void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) override;
    void mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) override;
    void howmany(std::uint32_t num) override;

    // auto kernel() -> ze_kernel_handle_t;
    // auto group_count() const -> ze_group_count_t const &;

  private:
    ::sycl::kernel_bundle<::sycl::bundle_state::executable> module_;
    std::vector<::sycl::kernel> kernels_;
    std::uint32_t active_kernel_ = 0;
};

} // namespace tinytc

#endif // SYCL_RECIPE_HANDLER_20240423_HPP
