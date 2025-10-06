// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RECIPE_20240419_HPP
#define RECIPE_20240419_HPP

#include "reference_counted.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace tinytc {
auto is_argument_zero(tinytc_type_t ty, std::size_t arg_size, const void *arg_value) -> bool;
} // namespace tinytc

struct tinytc_recipe : tinytc::reference_counted {
  public:
    inline tinytc_recipe(tinytc::shared_handle<tinytc_prog_t> prg,
                         tinytc::shared_handle<tinytc_binary_t> bin)
        : prg_(std::move(prg)), bin_(std::move(bin)) {}
    virtual ~tinytc_recipe() = default;

    inline auto get_program() const -> tinytc_prog_t { return prg_.get(); }
    inline auto get_binary() const -> tinytc_binary_t { return bin_.get(); }

    virtual auto num_kernels() const -> int = 0;
    virtual auto kernel_name(int kernel_num) const -> char const * = 0;

  private:
    tinytc::shared_handle<tinytc_prog_t> prg_;
    tinytc::shared_handle<tinytc_binary_t> bin_;
};

struct tinytc_recipe_handler : tinytc::reference_counted {
  public:
    inline tinytc_recipe_handler(tinytc::shared_handle<tinytc_recipe_t> recipe)
        : recipe_(std::move(recipe)) {}
    virtual ~tinytc_recipe_handler() = default;

    inline auto get_recipe() const -> tinytc_recipe_t { return recipe_.get(); }

    virtual void active_kernel(int kernel_num) = 0;
    virtual void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) = 0;
    virtual void mem_arg(std::uint32_t arg_index, const void *value, tinytc_mem_type_t ty) = 0;
    virtual void howmany(std::int64_t num) = 0;

  private:
    tinytc::shared_handle<tinytc_recipe_t> recipe_;
};

#endif // RECIPE_20240419_HPP
