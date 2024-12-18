// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/matrix_ext.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "matrix_ext_info.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace tinytc {

auto matrix_ext_analysis_result::get(::const_tinytc_value_t a) const -> bool {
    return mext_.find(a) != mext_.end();
}
auto matrix_ext_analysis_result::get(::tinytc_value const &a) const -> bool { return get(&a); }

class matrix_ext_helper {
  public:
    matrix_ext_helper(::tinytc_core_info const &info,
                      std::unordered_set<const_tinytc_value_t> &mext,
                      std::queue<const_tinytc_inst_t> &q)
        : info_{&info}, mext_{&mext}, q_{&q} {}
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(cooperative_matrix_load_inst const &in);
    void operator()(cooperative_matrix_mul_add_inst const &in);
    void operator()(cooperative_matrix_scale_inst const &in);
    void operator()(cooperative_matrix_store_inst const &in);
    void operator()(for_inst const &in);
    void operator()(if_inst const &in);

  private:
    auto have(value_node const &val) -> bool;
    void kill(value_node const &val);
    auto check_2d_block_io(value_node const &operand, std::int32_t alignment) -> bool;

    ::const_tinytc_core_info_t info_;
    std::unordered_set<const_tinytc_value_t> *mext_;
    std::queue<const_tinytc_inst_t> *q_;
};

auto matrix_ext_helper::have(value_node const &val) -> bool { return mext_->contains(&val); }

void matrix_ext_helper::kill(value_node const &val) {
    if (auto it = mext_->find(&val); it != mext_->end()) {
        if (val.defining_inst()) {
            q_->push(val.defining_inst());
        }
        for (auto const &use : val.uses()) {
            auto in = use.owner();
            if (isa<yield_inst>(*in)) {
                in = in->parent()->defining_inst();
                if (in) {
                    q_->push(in);
                }
            } else {
                q_->push(in);
            }
        }
        mext_->erase(it);
    }
}

void matrix_ext_helper::operator()(inst_node const &) {}
void matrix_ext_helper::operator()(arith_inst const &in) {
    // Missing: OpIAdd, OpFAdd, OpISub, OpFSub, OpFMul, OpIMul, OpFDiv, OpSDiv
    kill(in.a());
    kill(in.b());
    kill(in.result(0));
}
void matrix_ext_helper::operator()(arith_unary_inst const &in) {
    // Missing: OpSNegate, OpFNegate
    kill(in.a());
    kill(in.result(0));
}
void matrix_ext_helper::operator()(cast_inst const &in) {
    // Missing: OpConvertFToS, OpConvertSToF, OpConvertUToF, OpSConvert, OpFConvert
    kill(in.a());
    kill(in.result(0));
}
auto matrix_ext_helper::check_2d_block_io(value_node const &operand,
                                          std::int32_t alignment) -> bool {
    auto const &block_io = info_->matrix().block_io();
    auto ot = get_memref_type(operand);
    const auto element_size = static_cast<std::int32_t>(size(ot->element_ty()));

    const bool base_address_alignment_ok = alignment >= block_io.base_address_alignment;
    const bool stride_ok = ot->stride(0) == 1 &&
                           (ot->stride(1) * element_size) >= block_io.min_stride &&
                           (ot->stride(1) * element_size) <= block_io.max_stride &&
                           (ot->stride(1) * element_size) % block_io.stride_alignment == 0;
    const bool addrspace_ok = ot->addrspace() == address_space::global;

    return base_address_alignment_ok && stride_ok && addrspace_ok;
}
void matrix_ext_helper::operator()(cooperative_matrix_load_inst const &in) {
    const bool transpose_ok = in.t() == transpose::N;
    if (!transpose_ok || !check_2d_block_io(in.operand(), in.align())) {
        kill(in.result(0));
    }
}
void matrix_ext_helper::operator()(cooperative_matrix_mul_add_inst const &in) {
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result(0));
    const bool have_gemm =
        have(in.a()) && have(in.b()) && have(in.c()) && have(in.result(0)) &&
        info_->matrix().have_gemm(at->component_ty(), bt->component_ty(), ct->component_ty(),
                                  rt->component_ty(), rt->rows(), rt->cols(), at->cols());
    if (!have_gemm) {
        kill(in.a());
        kill(in.b());
        kill(in.c());
        kill(in.result(0));
    }
}
void matrix_ext_helper::operator()(cooperative_matrix_scale_inst const &in) {
    // Missing: OpMatrixTimesScalar
    kill(in.b());
    kill(in.result(0));
}
void matrix_ext_helper::operator()(cooperative_matrix_store_inst const &in) {
    auto vt = get_coopmatrix_type(in.val());
    const bool store_flag_ok = in.flag() == store_flag::regular;
    const bool use_ok = vt->use() == matrix_use::acc;
    if (!store_flag_ok || !use_ok || !check_2d_block_io(in.operand(), in.align())) {
        kill(in.val());
    }
}
void matrix_ext_helper::operator()(for_inst const &in) {
    if (in.num_results() > 0) {
        auto yield = get_yield(in.loc(), in.body());
        if (yield->num_operands() != in.num_results()) {
            throw compilation_error(in.loc(), status::ir_yield_mismatch);
        }
        for (std::int64_t i = 0; i < in.num_results(); ++i) {
            if (isa<coopmatrix_data_type>(*in.iter_arg(i).ty())) {
                if (!have(in.result(i)) || !have(in.iter_arg(i)) || !have(in.iter_init(i)) ||
                    !have(yield->op(i))) {
                    kill(in.result(i));
                    kill(in.iter_arg(i));
                    kill(in.iter_init(i));
                    kill(yield->op(i));
                }
            }
        }
    }
}
void matrix_ext_helper::operator()(if_inst const &in) {
    if (in.num_results() > 0) {
        auto then_yield = get_yield(in.loc(), in.then());
        auto otherwise_yield = get_yield(in.loc(), in.otherwise());
        if (then_yield->num_operands() != in.num_results() ||
            otherwise_yield->num_operands() != in.num_results()) {
            throw compilation_error(in.loc(), status::ir_yield_mismatch);
        }
        for (std::int64_t i = 0; i < in.num_results(); ++i) {
            if (isa<coopmatrix_data_type>(*in.result(i).ty())) {
                if (!have(in.result(i)) || !have(then_yield->op(i)) ||
                    !have(otherwise_yield->op(i))) {
                    kill(in.result(i));
                    kill(then_yield->op(i));
                    kill(otherwise_yield->op(i));
                }
            }
        }
    }
}

auto matrix_ext_analysis::run_on_function(function_node const &fn, ::tinytc_core_info const &info)
    -> matrix_ext_analysis_result {
    // mext = coopmatrix values that are mapped to matrix extension
    auto mext = std::unordered_set<const_tinytc_value_t>{};
    auto q = std::queue<const_tinytc_inst_t>{};

    // Insert all coopmatrix values that could be mapped to matrix extension
    for (auto const &in0 : fn.body()) {
        walk<walk_order::pre_order>(in0, [&](inst_node const &in) {
            bool has_at_least_one = false;
            auto const add_if = [&](value_node const &v) {
                if (auto rt = dyn_cast<coopmatrix_data_type>(v.ty()); rt) {
                    if (info.matrix().have_type(rt)) {
                        mext.insert(&v);
                        has_at_least_one = true;
                    }
                }
            };
            for (auto const &res : in.results()) {
                add_if(res);
            }
            for (auto const &subreg : in.child_regions()) {
                for (auto const &p : subreg.params()) {
                    add_if(p);
                }
            }
            if (has_at_least_one) {
                q.push(&in);
            }
        });
    }

    auto helper = matrix_ext_helper{info, mext, q};

    // kill all values from mext that cannot be mapped to the matrix extension
    while (!q.empty()) {
        auto n = q.front();
        q.pop();

        visit(helper, *n);
    }

    return matrix_ext_analysis_result{std::move(mext)};
}

} // namespace tinytc
