// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RECIPE_20240419_HPP
#define RECIPE_20240419_HPP

#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <utility>

namespace tinytc {
auto is_argument_zero(scalar_type type, std::size_t arg_size, const void *arg_value) -> bool;

} // namespace tinytc

struct tinytc_recipe : tinytc::reference_counted {
  public:
    inline tinytc_recipe(tinytc::prog prg, tinytc::binary bin)
        : prg_(std::move(prg)), bin_(std::move(bin)) {}
    virtual ~tinytc_recipe() = default;

    inline auto get_program() const -> tinytc::prog const & { return prg_; }
    inline auto get_binary() const -> tinytc::binary const & { return bin_; }

    virtual auto num_kernels() const -> std::uint32_t = 0;
    virtual auto kernel_name(std::uint32_t kernel_num) const -> char const * = 0;

  private:
    tinytc::prog prg_;
    tinytc::binary bin_;
};

struct tinytc_recipe_handler : tinytc::reference_counted {
  public:
    inline tinytc_recipe_handler(tinytc::recipe recipe) : recipe_(std::move(recipe)) {}
    virtual ~tinytc_recipe_handler() = default;

    inline auto get_recipe() const -> tinytc::recipe const & { return recipe_; }

    virtual void active_kernel(std::uint32_t kernel_num) = 0;
    virtual void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) = 0;
    virtual void mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) = 0;
    virtual void howmany(std::uint32_t num) = 0;

  private:
    tinytc::recipe recipe_;
};

#endif // RECIPE_20240419_HPP