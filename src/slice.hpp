// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SLICE_20240412_HPP
#define SLICE_20240412_HPP

#include "tinytc/tinytc.hpp"

#include <utility>

namespace tinytc {

//! Slice storing offset:size
class slice : public std::pair<value, value> {
  public:
    //! ctor
    inline slice(value offset = nullptr, value size = nullptr)
        : std::pair<value, value>{std::move(offset), std::move(size)} {}
};

} // namespace tinytc

#endif // SLICE_20240412_HPP
