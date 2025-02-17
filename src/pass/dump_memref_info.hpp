// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_MEMREF_INFO_20250214_HPP
#define DUMP_MEMREF_INFO_20250214_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

#include <iosfwd>

namespace tinytc {

class dump_memref_info_pass {
  public:
    dump_memref_info_pass(std::ostream &os, ::tinytc_core_info const *info);

    void run_on_function(function_node const &fn);

  private:
    std::ostream *os_;
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // DUMP_MEMREF_INFO_20250214_HPP
