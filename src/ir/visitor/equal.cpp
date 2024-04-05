// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/visitor/equal.hpp"
#include "tinytc/ir/data_type.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <algorithm>
#include <vector>

using clir::visit;

namespace tinytc::ir::internal {

bool equal::operator()(data_type_node &, data_type_node &) { return false; }
bool equal::operator()(void_data_type &, void_data_type &) { return true; }
bool equal::operator()(group_data_type &a, group_data_type &b) {
    return visit(*this, *a.ty(), *b.ty());
}
bool equal::operator()(memref_data_type &a, memref_data_type &b) {
    return a.element_ty() == b.element_ty() && a.shape() == b.shape() && a.stride() == b.stride();
}
bool equal::operator()(scalar_data_type &a, scalar_data_type &b) { return a.ty() == b.ty(); }

} // namespace tinytc::ir::internal
