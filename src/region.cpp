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

tinytc_status_t tinytc_region_add_instruction(tinytc_region_t reg, tinytc_inst_t instruction) {
    if (reg == nullptr || instruction == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { reg->insts().push_back(instruction); });
}

tinytc_status_t tinytc_region_get_parameter(tinytc_region_t reg, uint32_t param_no,
                                            tinytc_value_t *result) {
    if (reg == nullptr || result == nullptr || param_no >= reg->num_params()) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *result = reg->param(param_no).get(); });
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
        auto const num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            auto results = reg->param_begin();
            auto const limit = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < limit; ++i) {
                result_list[i] = results[i].get();
            }
        }
        *result_list_size = num;
    });
}
}
