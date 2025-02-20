// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TEMP_COUNTER_20250220_HPP
#define TEMP_COUNTER_20250220_HPP

#include <cstdint>
#include <string>

namespace tinytc {

class temp_counter {
  public:
    auto operator()(char const *prefix = "") -> std::string;

  private:
    std::int64_t tmp_counter_ = 0;
};

} // namespace tinytc

#endif // TEMP_COUNTER_20250220_HPP
