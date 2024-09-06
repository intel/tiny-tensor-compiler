// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/work_group_size.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc {

auto get_memref_type(value_node &v) {
    auto t = dyn_cast<memref_data_type>(v.ty().get());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

work_group_size::work_group_size(::tinytc_core_info const *info) : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

/* Stmt nodes */
void work_group_size::operator()(inst_node &) {}

void work_group_size::operator()(blas_a2_inst &in) {
    auto b = get_memref_type(*in.B());
    if (b->dim() == 1) {
        shapes_.insert({b->element_ty(), {b->shape(0), 0}});
    } else if (b->dim() >= 2) {
        shapes_.insert({b->element_ty(), {b->shape(0), b->shape(1)}});
    }
}
void work_group_size::operator()(blas_a3_inst &in) {
    auto c = get_memref_type(*in.C());
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
void work_group_size::operator()(parallel_inst &p) { visit(*this, *p.body()); }

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
            subgroup_size = suggest_subgroup_size(shapes, *info_);
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
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    if (work_group_size[0] == 0 || work_group_size[1] == 0) {
        throw compilation_error(fn.loc(), status::unsupported_work_group_size);
    }
    if (work_group_size[0] % subgroup_size != 0) {
        throw compilation_error(fn.loc(), status::unsupported_work_group_size,
                                "First work-group size mode must be divisible by subgroup size");
    }

    core_config cfg = {};
    try {
        cfg = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    if (work_group_size[0] * work_group_size[1] > cfg.max_work_group_size) {
        throw compilation_error(fn.loc(), status::unsupported_work_group_size);
    }
}

/* Program nodes */
void work_group_size::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
