// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_20230908_HPP
#define REGION_20230908_HPP

#include "tinytc/export.hpp"

#include "clir/handle.hpp"

namespace tinytc::ir {

namespace internal {
class region_node;
}

//! Reference-counted region handle
class TINYTC_EXPORT region : public clir::handle<internal::region_node> {
  public:
    using clir::handle<internal::region_node>::handle;
};

} // namespace tinytc::ir

#endif // REGION_20230908_HPP
