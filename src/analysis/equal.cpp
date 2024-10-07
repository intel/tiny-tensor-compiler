// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/equal.hpp"
#include "support/visit.hpp"

#include <vector>

namespace tinytc {

bool equal::operator()(data_type_node const &, data_type_node const &) { return false; }
bool equal::operator()(void_data_type const &, void_data_type const &) { return true; }
bool equal::operator()(group_data_type const &a, group_data_type const &b) {
    return visit(*this, *a.ty(), *b.ty()) && a.offset() == b.offset();
}
bool equal::operator()(memref_data_type const &a, memref_data_type const &b) {
    return a.element_ty() == b.element_ty() && a.shape() == b.shape() && a.stride() == b.stride() &&
           a.addrspace() == b.addrspace();
}
bool equal::operator()(scalar_data_type const &a, scalar_data_type const &b) {
    return a.ty() == b.ty();
}

bool is_equal(tinytc_data_type const &a, tinytc_data_type const &b) { return visit(equal{}, a, b); }

} // namespace tinytc
