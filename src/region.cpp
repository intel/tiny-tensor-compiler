// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/region_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_region_create(tinytc_region_t *reg, const tinytc_location_t *loc) {
    if (reg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *reg = std::make_unique<region_node>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_region_add_instruction(tinytc_region_t reg, tinytc_inst_t instruction) {
    if (reg == nullptr || instruction == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { reg->push_back(instruction); });
}

tinytc_status_t tinytc_region_release(tinytc_region_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_region_retain(tinytc_region_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
