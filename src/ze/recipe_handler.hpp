// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_RECIPE_HANDLER_20240419_HPP
#define ZE_RECIPE_HANDLER_20240419_HPP

#include "../recipe.hpp"
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_ze.hpp>
#include <tinytc/types.h>

#include <cstdint>
#include <level_zero/ze_api.h>
#include <vector>

namespace tinytc {

struct ze_recipe_handler : ::tinytc_recipe_handler {
  public:
    ze_recipe_handler(ze_context_handle_t context, ze_device_handle_t device, recipe rec);

    void active_kernel(std::uint32_t kernel_num) override;
    void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) override;
    void mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) override;
    void howmany(std::uint32_t num) override;

    auto kernel() -> ze_kernel_handle_t;
    auto group_count() const -> ze_group_count_t const &;

  private:
    unique_handle<ze_module_handle_t> module_;
    std::vector<unique_handle<ze_kernel_handle_t>> kernels_;
    std::uint32_t active_kernel_ = 0;
    ze_group_count_t group_count_ = {};
};

} // namespace tinytc

#endif // ZE_RECIPE_HANDLER_20240419_HPP
