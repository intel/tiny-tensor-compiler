// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROG_20240208_HPP
#define PROG_20240208_HPP

#include "tinytc/export.h"

#include <clir/handle.hpp>

#include <memory>
#include <vector>

namespace tinytc {

class func;
class program_node;

//! Reference-counted program handle
class TINYTC_EXPORT prog : public clir::handle<program_node> {
  public:
    using clir::handle<program_node>::handle;

    //! Create program containing single function
    prog(func fun);
    //! Create program containting multiple functions
    prog(std::vector<func> funs);
};

} // namespace tinytc

#endif // PROG_20240208_HPP