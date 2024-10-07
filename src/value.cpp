// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

#include <cstdint>
#include <string>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_value_set_name(tinytc_value_t vl, char const *name) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->name(std::string(name)); });
}

tinytc_status_t tinytc_value_set_name_n(tinytc_value_t vl, uint32_t name_length, char const *name) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->name(std::string(name, name_length)); });
}

tinytc_status_t tinytc_value_get_name(const_tinytc_value_t vl, char const **name) {
    if (vl == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { return vl->name(); });
}
}
