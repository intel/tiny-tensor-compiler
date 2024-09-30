// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/region_node.hpp"
#include "node/inst_node.hpp"

namespace tinytc {

auto ilist_callbacks<tinytc_inst>::get_parent_region() -> tinytc_region * {
    return reinterpret_cast<tinytc_region *>(reinterpret_cast<char *>(this) -
                                             tinytc_region::inst_list_offset());
}

void ilist_callbacks<tinytc_inst>::node_added(tinytc_inst_t node) {
    node->parent(get_parent_region());
}
void ilist_callbacks<tinytc_inst>::node_removed(tinytc_inst_t node) { tinytc_inst_destroy(node); }

} // namespace tinytc

using namespace tinytc;

tinytc_region::tinytc_region(array_view<tinytc_data_type_t> param_types, location const &lc)
    : kind_(region_kind::mixed) {
    loc(lc);

    params_.reserve(param_types.size());
    for (auto &param_ty : param_types) {
        params_.push_back(make_value(param_ty));
    }
}
tinytc_region::~tinytc_region() {}

void tinytc_region::add_param(tinytc_data_type_t param_type) {
    params_.push_back(make_value(param_type));
}

