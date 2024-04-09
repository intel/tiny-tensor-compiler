// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/work_group_size.hpp"
#include "ir/node/data_type_node.hpp"
#include "ir/node/value_node.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/value.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using clir::visit;

namespace tinytc::ir {

auto get_memref_type(value &v) {
    auto t = dynamic_cast<memref_data_type *>(v->ty().get());
    if (t == nullptr) {
        throw compilation_error(v->loc(), "Expected matrix type: " + std::string(v->name()));
    }
    return t;
}

work_group_size::work_group_size(std::shared_ptr<core_info> info) : info_(std::move(info)) {}

/* Stmt nodes */
void work_group_size::operator()(inst_node &) {}

void work_group_size::operator()(blas_a2_inst &in) {
    auto b = get_memref_type(in.B());
    if (b->dim() == 1) {
        shapes_.insert({b->element_ty(), {b->shape(0), 0}});
    } else if (b->dim() >= 2) {
        shapes_.insert({b->element_ty(), {b->shape(0), b->shape(1)}});
    }
}
void work_group_size::operator()(blas_a3_inst &in) {
    auto c = get_memref_type(in.C());
    if (c->dim() == 1) {
        shapes_.insert({c->element_ty(), {c->shape(0), 0}});
    } else if (c->dim() >= 2) {
        shapes_.insert({c->element_ty(), {c->shape(0), c->shape(1)}});
    }
}

void work_group_size::operator()(if_inst &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}
void work_group_size::operator()(loop_inst &in) { visit(*this, *in.body()); }

/* Region nodes */
void work_group_size::operator()(rgn &b) {
    for (auto &i : b.insts()) {
        visit(*this, *i);
    }
}

/* Function nodes */
void work_group_size::operator()(prototype &) {}

void work_group_size::operator()(function &fn) {
    auto subgroup_size = fn.subgroup_size();
    auto work_group_size = fn.work_group_size();

    shapes_.clear();
    if (subgroup_size == 0 || work_group_size[0] == 0 || work_group_size[1] == 0) {
        visit(*this, *fn.prototype());
        visit(*this, *fn.body());

        auto const shapes = std::vector<blas_shape>(shapes_.begin(), shapes_.end());

        if (subgroup_size == 0) {
            subgroup_size = suggest_subgroup_size(shapes, info_->subgroup_sizes());
            fn.subgroup_size(subgroup_size);
        }

        if (work_group_size[0] == 0 || work_group_size[1] == 0) {
            auto const core_cfg = info_->get_core_config(subgroup_size);

            auto tiling = suggest_local_tiling(shapes, core_cfg);
            work_group_size[0] = tiling[0] * subgroup_size;
            work_group_size[1] = tiling[1];
            fn.work_group_size(work_group_size);
        }
    }

    if (subgroup_size == 0) {
        throw compilation_error(fn.loc(), "Subgroup size must be non-zero");
    }
    if (work_group_size[0] == 0 || work_group_size[1] == 0) {
        throw compilation_error(fn.loc(), "All work-group size modes must be non-zero");
    }
    if (work_group_size[0] % subgroup_size != 0) {
        throw compilation_error(fn.loc(),
                                "First work-group size mode must be divisible by subgroup size");
    }

    core_config cfg = {};
    try {
        cfg = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), "Unsupported subgroup size");
    }
    if (work_group_size[0] * work_group_size[1] > cfg.max_number_of_work_items) {
        throw compilation_error(
            fn.loc(), "Work group size is larger than maximum work group size supported by device");
    }
}

/* Program nodes */
void work_group_size::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc::ir
