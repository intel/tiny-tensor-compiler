// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/value_node.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <memory>
#include <string>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_value_create(tinytc_value_t *vl, tinytc_data_type_t type,
                                    const tinytc_location_t *lc) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *vl = std::make_unique<val>(type, get_optional(lc)).release(); });
}

tinytc_status_t tinytc_value_release(tinytc_value_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_value_retain(tinytc_value_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_value_set_name(tinytc_value_t vl, char const *name) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->name(std::string(name)); });
}

tinytc_status_t tinytc_value_get_name(const_tinytc_value_t vl, char const **name) {
    if (vl == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { return vl->name(); });
}
}
