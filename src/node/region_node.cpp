// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/region_node.hpp"
#include "node/inst_node.hpp"
#include "tinytc/tinytc.h"

#include <utility>

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
    : kind_(region_kind::mixed), params_{param_types.size()} {
    loc(lc);

    set_params(std::move(param_types), lc);
}
tinytc_region::~tinytc_region() {}

void tinytc_region::set_params(array_view<tinytc_data_type_t> param_types, location const &lc) {
    params_.resize(param_types.size());
    for (std::size_t i = 0; i < param_types.size(); ++i) {
        params_[i] = tinytc_value{param_types[i], nullptr, lc};
    }
}

void tinytc_region::set_num_params(std::size_t num_params) { params_.resize(num_params); }
void tinytc_region::set_param(std::size_t idx, tinytc_data_type_t param_type, location const &lc) {
    params_[idx] = tinytc_value{param_type, nullptr, lc};
}
