// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/region.hpp"
#include "error.hpp"
#include "node/inst.hpp"
#include "node/inst.hpp" // IWYU pragma: keep
#include "tinytc/builder.h"
#include "tinytc/types.h"
#include "util/ilist.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace tinytc;

namespace tinytc {

auto ilist_callbacks<tinytc_inst>::get_parent_region() -> tinytc_region * {
    return reinterpret_cast<tinytc_region *>(reinterpret_cast<char *>(this) -
                                             tinytc_region::inst_list_offset());
}

void ilist_callbacks<tinytc_inst>::node_added(tinytc_inst_t node) {
    node->parent(get_parent_region());
}
void ilist_callbacks<tinytc_inst>::node_moved(tinytc_inst_t node) {
    node->parent(get_parent_region());
}
void ilist_callbacks<tinytc_inst>::node_removed(tinytc_inst_t node) { tinytc_inst_destroy(node); }

} // namespace tinytc

tinytc_region::tinytc_region(tinytc_inst_t def_inst)
    : def_inst_{def_inst}, kind_{region_kind::mixed} {}

tinytc_region::~tinytc_region() {}

void tinytc_region::loc(location const &loc) {
    loc_ = loc;
    for (auto &param : params_) {
        param.loc(loc_);
    }
}
void tinytc_region::defining_inst(tinytc_inst_t def_inst) {
    def_inst_ = def_inst;
    for (auto &param : params_) {
        param.defining_inst(def_inst_);
    }
}

void tinytc_region::set_params(array_view<tinytc_data_type_t> param_types) {
    params_.resize(param_types.size());
    for (std::size_t i = 0; i < param_types.size(); ++i) {
        set_param(i, param_types[i]);
    }
}

void tinytc_region::set_num_params(std::size_t num_params) { params_.resize(num_params); }
void tinytc_region::set_param(std::size_t idx, tinytc_data_type_t param_type) {
    params_[idx] = tinytc_value{param_type, def_inst_, loc_};
}

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

tinytc_status_t tinytc_region_get_parameters(tinytc_region_t reg, size_t *result_list_size,
                                             tinytc_value_t *result_list) {

    if (reg == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = reg->num_params();
        if (num_results < 0) {
            throw std::out_of_range("number of results must not be negative");
        }
        auto num = static_cast<std::size_t>(num_results);
        if (*result_list_size > 0) {
            auto results = reg->param_begin();
            num = std::min(num, *result_list_size);
            for (size_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}
}
