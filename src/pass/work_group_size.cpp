// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/work_group_size.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <functional>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tinytc {

work_group_size_pass::work_group_size_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void work_group_size_pass::run_on_function(function_node &fn) {
    auto subgroup_size = fn.subgroup_size();
    auto work_group_size = fn.work_group_size();

    auto shape_set = std::unordered_set<blas_shape>{};

    if (subgroup_size == 0 || work_group_size[0] == 0 || work_group_size[1] == 0) {
        walk<walk_order::pre_order>(fn, [&shape_set](inst_node &i) {
            visit(
                overloaded{[&](blas_a2_inst &in) {
                               auto aty = get_memref_type(in.A())->element_ty();
                               auto b = get_memref_type(in.B());
                               if (b->dim() == 1) {
                                   shape_set.insert({aty, aty, b->element_ty(), {b->shape(0), 0}});
                               } else if (b->dim() >= 2) {
                                   shape_set.insert(
                                       {aty, aty, b->element_ty(), {b->shape(0), b->shape(1)}});
                               }
                           },
                           [&](blas_a3_inst &in) {
                               auto aty = get_memref_type(in.A())->element_ty();
                               auto bty = get_memref_type(in.B())->element_ty();
                               auto c = get_memref_type(in.C());
                               if (c->dim() == 1) {
                                   shape_set.insert({aty, bty, c->element_ty(), {c->shape(0), 0}});
                               } else if (c->dim() >= 2) {
                                   shape_set.insert({aty,
                                                     bty,
                                                     c->element_ty(),
                                                     {c->shape(0), c->shape(1)},
                                                     isa<gemm_inst>(in)});
                               }
                           },
                           [](inst_node &) {}},
                i);
        });

        auto const shapes = std::vector<blas_shape>(shape_set.begin(), shape_set.end());

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

} // namespace tinytc
