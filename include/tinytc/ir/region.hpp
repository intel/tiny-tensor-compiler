// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_20230908_HPP
#define REGION_20230908_HPP

#include "tinytc/export.h"

#include "clir/handle.hpp"

namespace tinytc {

class region_node;

//! Reference-counted region handle
class TINYTC_EXPORT region : public clir::handle<region_node> {
  public:
    using clir::handle<region_node>::handle;
};

} // namespace tinytc

#endif // REGION_20230908_HPP
