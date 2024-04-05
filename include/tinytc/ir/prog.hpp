// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROG_20240208_HPP
#define PROG_20240208_HPP

#include "tinytc/export.hpp"

#include <clir/handle.hpp>

#include <memory>
#include <vector>

namespace tinytc::ir {

class func;

namespace internal {
class program_node;
}

//! Reference-counted program handle
class TINYTC_EXPORT prog : public clir::handle<internal::program_node> {
  public:
    using clir::handle<internal::program_node>::handle;

    //! Create program containing single function
    prog(func fun);
    //! Create program containting multiple functions
    prog(std::vector<func> funs);
};

} // namespace tinytc::ir

#endif // PROG_20240208_HPP
