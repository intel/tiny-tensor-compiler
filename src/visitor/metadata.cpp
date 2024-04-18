// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/metadata.hpp"

#include <clir/visit.hpp>

using clir::visit;

namespace tinytc {

/* Function nodes */
void metadata::operator()(prototype const &) {}

void metadata::operator()(function const &fn) {
    auto m = kernel_metadata{};
    m.subgroup_size = fn.subgroup_size();
    m.work_group_size = fn.work_group_size();
    metadata_[std::string(fn.name())] = m;
}

/* Program nodes */
void metadata::operator()(program const &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
