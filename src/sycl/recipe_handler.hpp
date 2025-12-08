// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_RECIPE_HANDLER_20240423_HPP
#define SYCL_RECIPE_HANDLER_20240423_HPP

#include "../recipe.hpp"
#include "argument_handler.hpp"
#include "tinytc/core.hpp"

#include <memory>
#include <sycl/sycl.hpp>
#include <vector>

namespace tinytc {

struct sycl_recipe_handler_impl : ::tinytc_recipe_handler {
  public:
    sycl_recipe_handler_impl(sycl::context const &context, sycl::device const &device,
                             shared_handle<tinytc_recipe_t> rec);

    void active_kernel(int kernel_num) override;
    void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) override;
    void mem_arg(std::uint32_t arg_index, const void *value, tinytc_mem_type_t ty) override;
    void howmany(std::int64_t num) override;

    auto kernel() const -> sycl::kernel const &;
    auto local_size() const -> sycl::range<3u> const &;
    inline auto execution_range() const -> sycl::nd_range<3u> const & { return execution_range_; }

  private:
    sycl::kernel_bundle<sycl::bundle_state::executable> module_;
    std::vector<sycl::range<3u>> local_size_;
    std::vector<sycl::kernel> kernels_;
    int active_kernel_ = 0;
    sycl::nd_range<3u> execution_range_;
    std::unique_ptr<sycl_argument_handler> arg_handler_;
};

} // namespace tinytc

#endif // SYCL_RECIPE_HANDLER_20240423_HPP
