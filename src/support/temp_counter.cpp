// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "support/temp_counter.hpp"

#include <sstream>

namespace tinytc {

auto temp_counter::operator()(char const *prefix) -> std::string {
    return (std::ostringstream{} << prefix << tmp_counter_++).str();
}

} // namespace tinytc
