// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNC_20230310_HPP
#define FUNC_20230310_HPP

#include "tinytc/export.h"

#include "clir/handle.hpp"

namespace tinytc {

class function_node;

//! Reference-counted function handle
class TINYTC_EXPORT func : public clir::handle<function_node> {
  public:
    using clir::handle<function_node>::handle;
};

} // namespace tinytc

#endif // FUNC_20230310_HPP
