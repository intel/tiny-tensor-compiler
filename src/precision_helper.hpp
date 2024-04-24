// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PRECISION_HELPER_20230214_HPP
#define PRECISION_HELPER_20230214_HPP

#include "tinytc/types.hpp"

#include "clir/builtin_type.hpp"
#include "clir/data_type.hpp"
#include "clir/expr.hpp"

namespace tinytc {

class precision_helper {
  public:
    precision_helper(scalar_type ty);
    clir::builtin_type base_type() const;
    clir::builtin_type block_rw_base_type() const;
    clir::expr as_type(clir::builtin_type ty, clir::expr e) const;
    short bits() const;
    clir::data_type type(clir::address_space as = clir::address_space::generic_t) const;
    clir::data_type type(short size, clir::address_space as = clir::address_space::generic_t) const;
    clir::expr constant(double value) const;
    clir::expr zero() const;
    clir::expr sub_group_block_read(clir::expr address,
                                    clir::address_space as = clir::address_space::generic_t) const;
    clir::expr sub_group_block_write(clir::expr address, clir::expr data,
                                     clir::address_space as = clir::address_space::generic_t) const;

  private:
    scalar_type ty_;
};

} // namespace tinytc

#endif // PRECISION_HELPER_20230214_HPP
