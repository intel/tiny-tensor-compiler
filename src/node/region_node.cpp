// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/region_node.hpp"
#include "node/inst_node.hpp"
#include "tinytc/tinytc.h"

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

using namespace tinytc;

tinytc_region::tinytc_region(tinytc_inst_t def_inst)
    : def_inst_{def_inst}, kind_{tinytc::region_kind::mixed} {}

tinytc_region::~tinytc_region() {}

void tinytc_region::loc(tinytc::location const &loc) {
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
