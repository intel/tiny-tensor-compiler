// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef EQUAL_20240208_HPP
#define EQUAL_20240208_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/internal/data_type_node.hpp"

namespace tinytc::ir::internal {

class TINYTC_EXPORT equal {
  public:
    /* Data type nodes */
    bool operator()(data_type_node &a, data_type_node &b);
    bool operator()(void_data_type &a, void_data_type &b);
    bool operator()(group_data_type &a, group_data_type &b);
    bool operator()(memref_data_type &a, memref_data_type &b);
    bool operator()(scalar_data_type &a, scalar_data_type &b);
};

} // namespace tinytc::ir::internal

#endif // EQUAL_20240208_HPP
