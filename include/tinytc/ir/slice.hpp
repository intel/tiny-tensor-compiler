// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SLICE_20230327_HPP
#define SLICE_20230327_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/value.hpp"

#include <utility>

namespace tinytc::ir {

//! Slice storing offset:size
class TINYTC_EXPORT slice : public std::pair<value, value> {
  public:
    /**
     * ctor; first gives the offset and second gives the size
     */
    inline slice(value first = nullptr, value second = nullptr)
        : std::pair<value, value>{std::move(first), std::move(second)} {}
};

} // namespace tinytc::ir

#endif // SLICE_20230327_HPP
