// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_MATRIX_EXT_20241205_HPP
#define DUMP_MATRIX_EXT_20241205_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

#include <iosfwd>

namespace tinytc {

class dump_matrix_ext_pass {
  public:
    dump_matrix_ext_pass(std::ostream &os, ::tinytc_core_info const *info);

    void run_on_function(function_node const &fn);

  private:
    std::ostream *os_;
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // DUMP_MATRIX_EXT_20241205_HPP
