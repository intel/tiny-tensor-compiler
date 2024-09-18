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

tinytc_status_t tinytc_region_create(tinytc_region_t *reg, uint32_t instruction_list_size,
                                     tinytc_inst_t *instruction_list,
                                     const tinytc_location_t *loc) {
    if (reg == nullptr || (instruction_list_size > 0 && instruction_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto inst_vec = std::vector<inst>();
        inst_vec.reserve(instruction_list_size);
        for (uint32_t i = 0; i < instruction_list_size; ++i) {
            inst_vec.emplace_back(inst(instruction_list[i], true));
        }
        *reg = std::make_unique<region_node>(std::move(inst_vec), get_optional(loc)).release();
    });
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
