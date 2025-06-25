// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "tinytc/builder.h"
#include "tinytc/types.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>

using namespace tinytc;

extern "C" {

void tinytc_inst_destroy(tinytc_inst_t obj) { tinytc_inst::destroy(obj); }

tinytc_status_t tinytc_inst_get_parent_region(tinytc_inst_t instr, tinytc_region_t *parent) {
    if (instr == nullptr || parent == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *parent = instr->parent(); });
}

tinytc_status_t tinytc_inst_get_values(tinytc_inst_t instr, uint32_t *result_list_size,
                                       tinytc_value_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_results();
        if (num_results > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many results");
        }
        auto num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            num = std::min(num, *result_list_size);
            auto results = instr->result_begin();
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}

tinytc_status_t tinytc_inst_get_regions(tinytc_inst_t instr, uint32_t *result_list_size,
                                        tinytc_region_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_child_regions();
        if (num_results > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many results");
        }
        auto num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            auto results = instr->child_regions_begin();
            num = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}

tinytc_status_t tinytc_inst_set_attr(tinytc_inst_t instr, tinytc_attr_t a) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { instr->attr(a); });
}
}
