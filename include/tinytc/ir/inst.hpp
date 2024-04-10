// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_20230327_HPP
#define INST_20230327_HPP

#include "tinytc/export.h"

#include "clir/handle.hpp"

namespace tinytc {

class inst_node;

//! Binary operations
enum class TINYTC_EXPORT binary_op {
    add, ///< add
    sub, ///< subtract
    mul, ///< multiply
    div, ///< divide
    rem  ///< division remainder
};
//! Compare operation
enum class TINYTC_EXPORT cmp_condition {
    eq, ///< equals
    ne, ///< not equal
    gt, ///< greater than
    ge, ///< greather or equal than
    lt, ///< less than
    le  ///< less or equal than
};
//! Instruction classification
enum class TINYTC_EXPORT inst_kind {
    replicated, ///< replicated instruction executed in every work-item
    collective  ///< collective instruction distributed among work-items
};
//! Transpose
enum class TINYTC_EXPORT transpose {
    N, ///< no transpose
    T  ///< transpose
};

//! Convert binary op to string
TINYTC_EXPORT char const *to_string(binary_op op);
//! Convert cmp condition to string
TINYTC_EXPORT char const *to_string(cmp_condition cond);
//! Convert transpose to string
TINYTC_EXPORT char const *to_string(transpose t);

//! Reference-counted instruction handle
class TINYTC_EXPORT inst : public clir::handle<inst_node> {
  public:
    using clir::handle<inst_node>::handle;
};

} // namespace tinytc

#endif // INST_20230327_HPP
