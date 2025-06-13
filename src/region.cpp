// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/inst_node.hpp" // IWYU pragma: keep
#include "node/region_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "util/ilist.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_region_append(tinytc_region_t reg, tinytc_inst_t instr) {
    if (reg == nullptr || instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { reg->insts().push_back(instr); });
}

tinytc_status_t tinytc_region_begin(tinytc_region_t reg, tinytc_inst_iterator_t *iterator) {
    if (reg == nullptr || iterator == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *iterator = reg->insts().begin().get(); });
}

tinytc_status_t tinytc_region_end(tinytc_region_t reg, tinytc_inst_iterator_t *iterator) {
    if (reg == nullptr || iterator == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *iterator = reg->insts().end().get(); });
}

tinytc_status_t tinytc_region_erase(tinytc_region_t reg, tinytc_inst_iterator_t *iterator) {
    if (reg == nullptr || iterator == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *iterator = reg->insts().erase(*iterator).get(); });
}

tinytc_status_t tinytc_region_insert(tinytc_region_t reg, tinytc_inst_iterator_t *iterator,
                                     tinytc_inst_t instr) {
    if (reg == nullptr || iterator == nullptr || instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *iterator = reg->insts().insert(*iterator, instr).get(); });
}

tinytc_status_t tinytc_next_inst(tinytc_inst_iterator_t *iterator) {
    if (iterator == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *iterator = static_cast<tinytc_inst_iterator_t>((*iterator)->next()); });
}

tinytc_status_t tinytc_prev_inst(tinytc_inst_iterator_t *iterator) {
    if (iterator == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *iterator = static_cast<tinytc_inst_iterator_t>((*iterator)->prev()); });
}

tinytc_status_t tinytc_region_get_parameter(tinytc_region_t reg, uint32_t param_no,
                                            tinytc_value_t *result) {
    if (reg == nullptr || result == nullptr || param_no >= reg->num_params()) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *result = &reg->param(param_no); });
}

tinytc_status_t tinytc_region_get_parameters(tinytc_region_t reg, uint32_t *result_list_size,
                                             tinytc_value_t *result_list) {

    if (reg == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = reg->num_params();
        if (num_results > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many results");
        }
        auto num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            auto results = reg->param_begin();
            num = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}
}
