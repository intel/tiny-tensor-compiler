// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef EQUAL_20240208_HPP
#define EQUAL_20240208_HPP

#include "node/data_type_node.hpp"

namespace tinytc {

class equal {
  public:
    /* Data type nodes */
    bool operator()(data_type_node const &a, data_type_node const &b);
    bool operator()(void_data_type const &a, void_data_type const &b);
    bool operator()(group_data_type const &a, group_data_type const &b);
    bool operator()(memref_data_type const &a, memref_data_type const &b);
    bool operator()(scalar_data_type const &a, scalar_data_type const &b);
};

bool is_equal(tinytc_data_type const &a, tinytc_data_type const &b);

} // namespace tinytc

#endif // EQUAL_20240208_HPP
