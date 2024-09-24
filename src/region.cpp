// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "support/ilist.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

#include <memory>

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
    return exception_to_status_code([&] { reg->insts().push_back(instruction); });
}

void tinytc_region_destroy(tinytc_region_t obj) { delete obj; }
}
