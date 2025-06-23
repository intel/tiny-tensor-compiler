// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst_node.hpp"
#include "error.hpp"
#include "node/inst_view.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "tinytc/types.hpp"
#include "util/overloaded.hpp"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>

static_assert(alignof(tinytc_value) == alignof(tinytc_inst));
static_assert(alignof(tinytc::use) == alignof(tinytc_inst));
static_assert(alignof(tinytc_region) == alignof(tinytc_inst));
static_assert(alignof(tinytc_inst) <= alignof(std::max_align_t));

auto tinytc_inst::create(tinytc::IK tid, tinytc::inst_layout layout, tinytc_location const &lc)
    -> tinytc_inst_t {
    std::size_t size = 0;
    size += sizeof(tinytc_value) * layout.num_results;
    size += sizeof(tinytc_inst);
    size += sizeof(tinytc::use) * layout.num_operands;
    size += layout.sizeof_properties;
    size += sizeof(tinytc_region) * layout.num_child_regions;

    struct del {
        void operator()(std::uint8_t *p) const { std::free(p); }
    };
    auto raw_mem =
        std::unique_ptr<std::uint8_t, del>(static_cast<std::uint8_t *>(std::malloc(size)));
    if (raw_mem.get() == nullptr) {
        throw tinytc::status::bad_alloc;
    }

    // initialize results
    tinytc_value_t first_result = reinterpret_cast<tinytc_value_t>(raw_mem.get());
    tinytc_value_t last_result = first_result + layout.num_results;
    for (; first_result != last_result; ++first_result) {
        new (first_result) tinytc_value();
    }

    // initialize inst
    tinytc_inst_t in = reinterpret_cast<tinytc_inst_t>(last_result);
    new (in) tinytc_inst(tid, layout, lc);

    // initialize uses
    tinytc::use *first_use = reinterpret_cast<tinytc::use *>(in + 1);
    tinytc::use *last_use = first_use + layout.num_operands;
    for (; first_use != last_use; ++first_use) {
        new (first_use) tinytc::use(in);
    }

    // properties
    std::uint8_t *first_prop = reinterpret_cast<std::uint8_t *>(last_use);
    std::uint8_t *last_prop = first_prop + layout.sizeof_properties;
    if (layout.sizeof_properties > 0) {
        tinytc::visit(tinytc::overloaded{[&](auto view) {
                          std::construct_at(
                              reinterpret_cast<decltype(view)::properties *>(first_prop));
                      }},
                      *in);
    }

    // child regions
    tinytc_region_t first_region = reinterpret_cast<tinytc_region_t>(last_prop);
    tinytc_region_t last_region = first_region + layout.num_child_regions;
    for (; first_region != last_region; ++first_region) {
        new (first_region) tinytc_region(in);
    }

    raw_mem.release();
    return in;
}

void tinytc_inst::destroy(tinytc_inst_t in) {
    void *raw_mem = reinterpret_cast<tinytc_value_t>(in) - in->layout_.num_results;
    in->~tinytc_inst();
    std::free(raw_mem);
}

tinytc_inst::~tinytc_inst() {
    // child regions
    for (tinytc_region_t r = child_region_ptr(0); r != child_region_ptr(layout_.num_child_regions);
         ++r) {
        std::destroy_at(r);
    }

    // properties
    if (layout_.sizeof_properties > 0) {
        tinytc::visit(tinytc::overloaded{[&](auto view) {
                          std::destroy_at(static_cast<decltype(view)::properties *>(props()));
                      }},
                      *this);
    }

    // uses
    for (tinytc::use *u = use_ptr(0); u != use_ptr(layout_.num_operands); ++u) {
        std::destroy_at(u);
    }

    // results
    for (tinytc_value_t r = result_ptr(0); r != result_ptr(layout_.num_results); --r) {
        std::destroy_at(r);
    }
}

auto tinytc_inst::context() -> tinytc_compiler_context_t {
    if (num_results() > 0) {
        return result(0).context();
    } else if (num_operands() > 0) {
        return op(0).context();
    }
    return nullptr;
}

void tinytc_inst::subs(tinytc_value_t old_value, tinytc_value_t new_value, bool recursive) {
    for (auto u = use_ptr(0); u != use_ptr(layout_.num_operands); ++u) {
        if (u->get() == old_value) {
            u->set(new_value);
        }
    }
    if (recursive) {
        for (auto &reg : child_regions()) {
            for (auto &in : reg) {
                in.subs(old_value, new_value, true);
            }
        }
    }
}

auto tinytc_inst::kind() -> tinytc::inst_execution_kind {
    switch (type_id()) {
    case tinytc::IK::IK_alloca:
    case tinytc::IK::IK_lifetime_stop:
    case tinytc::IK::IK_foreach:
    case tinytc::IK::IK_parallel:
    case tinytc::IK::IK_blas_a2:
    case tinytc::IK::IK_axpby:
    case tinytc::IK::IK_cumsum:
    case tinytc::IK::IK_sum:
    case tinytc::IK::IKEND_blas_a2:
    case tinytc::IK::IK_blas_a3:
    case tinytc::IK::IK_gemm:
    case tinytc::IK::IK_gemv:
    case tinytc::IK::IK_ger:
    case tinytc::IK::IK_hadamard:
    case tinytc::IK::IKEND_blas_a3:
        return tinytc::inst_execution_kind::collective;
    case tinytc::IK::IK_arith:
    case tinytc::IK::IK_arith_unary:
    case tinytc::IK::IK_barrier:
    case tinytc::IK::IK_cast:
    case tinytc::IK::IK_compare:
    case tinytc::IK::IK_constant:
    case tinytc::IK::IK_expand:
    case tinytc::IK::IK_fuse:
    case tinytc::IK::IK_if:
    case tinytc::IK::IK_load:
    case tinytc::IK::IK_math_unary:
    case tinytc::IK::IK_size:
    case tinytc::IK::IK_store:
    case tinytc::IK::IK_subview:
    case tinytc::IK::IK_yield:
    case tinytc::IK::IK_loop:
    case tinytc::IK::IK_for:
    case tinytc::IK::IKEND_loop:
        return tinytc::inst_execution_kind::mixed;
    case tinytc::IK::IK_cooperative_matrix_apply:
    case tinytc::IK::IK_cooperative_matrix_extract:
    case tinytc::IK::IK_cooperative_matrix_insert:
    case tinytc::IK::IK_cooperative_matrix_load:
    case tinytc::IK::IK_cooperative_matrix_mul_add:
    case tinytc::IK::IK_cooperative_matrix_prefetch:
    case tinytc::IK::IK_cooperative_matrix_reduce:
    case tinytc::IK::IK_cooperative_matrix_scale:
    case tinytc::IK::IK_cooperative_matrix_store:
    case tinytc::IK::IK_subgroup_broadcast:
    case tinytc::IK::IK_subgroup_operation:
        return tinytc::inst_execution_kind::spmd;
    case tinytc::IK::IK_builtin:
        return tinytc::dyn_cast<tinytc::builtin_inst>(this).kind();
    };
    throw tinytc::internal_compiler_error();
}

