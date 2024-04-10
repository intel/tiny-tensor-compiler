// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef EQUAL_20240208_HPP
#define EQUAL_20240208_HPP

#include "ir/node/data_type_node.hpp"

namespace tinytc {

class equal {
  public:
    /* Data type nodes */
    bool operator()(::tinytc_data_type &a, ::tinytc_data_type &b);
    bool operator()(void_data_type &a, void_data_type &b);
    bool operator()(group_data_type &a, group_data_type &b);
    bool operator()(memref_data_type &a, memref_data_type &b);
    bool operator()(scalar_data_type &a, scalar_data_type &b);
};

} // namespace tinytc

#endif // EQUAL_20240208_HPP