// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst_node.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <sstream>
#include <utility>

auto tinytc_inst::context() const -> tinytc_compiler_context_t {
    if (num_results() > 0) {
        return result(0).context();
    } else if (num_operands() > 0) {
        return op(0).context();
    }
    return nullptr;
}

void tinytc_inst::subs(tinytc_value_t old_value, tinytc_value_t new_value, bool recursive) {
    for (auto op = op_begin_; op != op_end_; ++op) {
        if (op->get() == old_value) {
            op->set(new_value);
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

auto tinytc_inst::kind() const -> tinytc::inst_execution_kind {
    switch (type_id()) {
    case tinytc::IK::alloca:
    case tinytc::IK::barrier:
    case tinytc::IK::lifetime_stop:
    case tinytc::IK::foreach_loop:
    case tinytc::IK::parallel:
    case tinytc::IK::blas_a2:
    case tinytc::IK::axpby_blas_a2:
    case tinytc::IK::sum_blas_a2:
    case tinytc::IK::last_blas_a2:
    case tinytc::IK::blas_a3:
    case tinytc::IK::gemm_blas_a3:
    case tinytc::IK::gemv_blas_a3:
    case tinytc::IK::ger_blas_a3:
    case tinytc::IK::hadamard_blas_a3:
    case tinytc::IK::last_blas_a3:
        return tinytc::inst_execution_kind::collective;
    case tinytc::IK::arith:
    case tinytc::IK::arith_unary:
    case tinytc::IK::cast:
    case tinytc::IK::compare:
    case tinytc::IK::constant:
    case tinytc::IK::expand:
    case tinytc::IK::fuse:
    case tinytc::IK::if_:
    case tinytc::IK::load:
    case tinytc::IK::size:
    case tinytc::IK::store:
    case tinytc::IK::subview:
    case tinytc::IK::work_group:
    case tinytc::IK::yield:
    case tinytc::IK::loop:
    case tinytc::IK::for_loop:
    case tinytc::IK::last_loop:
        return tinytc::inst_execution_kind::mixed;
    case tinytc::IK::cooperative_matrix_load:
    case tinytc::IK::cooperative_matrix_mul_add:
    case tinytc::IK::cooperative_matrix_scale:
    case tinytc::IK::cooperative_matrix_store:
    case tinytc::IK::subgroup_broadcast:
        return tinytc::inst_execution_kind::spmd;
    case tinytc::IK::builtin:
        return tinytc::dyn_cast<const tinytc::builtin_inst>(this)->kind();
    };
    throw tinytc::internal_compiler_error();
}

namespace tinytc {

coopmatrix_data_type *get_coopmatrix_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<coopmatrix_data_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_coopmatrix);
    }
    return m;
}

scalar_data_type *get_scalar_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<scalar_data_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_scalar);
    }
    return m;
}

memref_data_type *get_memref_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<memref_data_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_memref);
    }
    return m;
}

void check_index_ty(location const &loc, tinytc_value const &v) {
    if (auto sty = dyn_cast<scalar_data_type>(v.ty()); !sty || sty->ty() != scalar_type::index) {
        throw compilation_error(loc, {&v}, status::ir_expected_index);
    }
}

void check_memref_shape(memref_data_type *rt, std::int64_t ri, memref_data_type *ot,
                        std::int64_t oi, location const &loc) {
    if (rt->shape(ri) != ot->shape(oi)) {
        auto extra_info = std::ostringstream{} << "Size of mode " << ri
                                               << " does not match operand mode " << oi << " ["
                                               << rt->shape(ri) << "!=" << ot->shape(oi) << "]";
        throw compilation_error(loc, status::ir_invalid_shape, std::move(extra_info).str());
    }
}
void check_memref_stride(memref_data_type *rt, std::int64_t ri, memref_data_type *ot,
                         std::int64_t oi, location const &loc) {
    if (!is_dynamic_value(rt->stride(ri)) && rt->stride(ri) != ot->stride(oi)) {
        auto extra_info = std::ostringstream{} << "Stride of mode " << ri
                                               << " does not match operand stride " << oi << " ["
                                               << rt->stride(ri) << "!=" << ot->stride(oi) << "]";
        throw compilation_error(loc, status::ir_invalid_stride, std::move(extra_info).str());
    }
}

void check_memref_mode(memref_data_type *rt, std::int64_t ri, memref_data_type *ot, std::int64_t oi,
                       location const &loc) {
    check_memref_shape(rt, ri, ot, oi, loc);
    check_memref_stride(rt, ri, ot, oi, loc);
}

auto get_and_check_memref_type_addrspace(tinytc_value const &operand, tinytc_data_type_t ty,
                                         location const &loc)
    -> std::pair<memref_data_type *, memref_data_type *> {
    auto rt = dyn_cast<memref_data_type>(ty);
    if (!rt) {
        throw compilation_error(loc, status::ir_expected_memref);
    }
    auto ot = get_memref_type(loc, operand);
    if (rt->element_data_ty() != ot->element_data_ty()) {
        throw compilation_error(loc, {&operand}, status::ir_scalar_mismatch);
    }
    if (rt->addrspace() != ot->addrspace()) {
        throw compilation_error(loc, {&operand}, status::ir_address_space_mismatch);
    }
    return {ot, rt};
}

blas_a2_inst::blas_a2_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t beta,
                           tinytc_value_t B, bool atomic, location const &lc)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha, alpha);
    op(op_A, A);
    op(op_beta, beta);
    op(op_B, B);
    loc(lc);

    auto At = get_memref_type(loc(), op(op_A));
    auto Bt = get_memref_type(loc(), op(op_B));
    auto alphat = get_scalar_type(loc(), op(op_alpha));
    auto betat = get_scalar_type(loc(), op(op_beta));

    if (!promotable(alphat->ty(), At->element_ty())) {
        throw compilation_error(loc(), {&op(op_alpha), &op(op_A)}, status::ir_forbidden_promotion);
    }
    if (!promotable(At->element_ty(), Bt->element_ty())) {
        throw compilation_error(loc(), {&op(op_A), &op(op_B)}, status::ir_forbidden_promotion);
    }
    if (!promotable(betat->ty(), Bt->element_ty())) {
        throw compilation_error(loc(), {&op(op_beta), &op(op_B)}, status::ir_forbidden_promotion);
    }
}

blas_a3_inst::blas_a3_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                           tinytc_value_t beta, tinytc_value_t C, bool atomic, location const &lc)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha, alpha);
    op(op_A, A);
    op(op_B, B);
    op(op_beta, beta);
    op(op_C, C);
    loc(lc);

    auto At = get_memref_type(loc(), op(op_A));
    auto Bt = get_memref_type(loc(), op(op_B));
    auto Ct = get_memref_type(loc(), op(op_C));
    auto alphat = get_scalar_type(loc(), op(op_alpha));
    auto betat = get_scalar_type(loc(), op(op_beta));

    const auto AB_ty = promote(At->element_ty(), Bt->element_ty());
    if (!AB_ty) {
        throw compilation_error(loc(), {&op(op_A), &op(op_B)}, status::ir_forbidden_promotion);
    }
    if (!promotable(alphat->ty(), *AB_ty)) {
        throw compilation_error(loc(), {&op(op_alpha), &op(op_A), &op(op_B)},
                                status::ir_forbidden_promotion);
    }
    if (!promotable(*AB_ty, Ct->element_ty())) {
        throw compilation_error(loc(), {&op(op_A), &op(op_B), &op(op_C)},
                                status::ir_forbidden_promotion);
    }
    if (!promotable(betat->ty(), Ct->element_ty())) {
        throw compilation_error(loc(), {&op(op_beta), &op(op_C)}, status::ir_forbidden_promotion);
    }
}

alloca_inst::alloca_inst(tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::alloca}, stack_ptr_{-1} {
    loc(lc);

    result(0) = value_node{ty, this, lc};
    auto memref = dyn_cast<memref_data_type>(result(0).ty());
    if (memref == nullptr) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    if (memref->addrspace() != address_space::local) {
        throw compilation_error(loc(), status::ir_expected_local_address_space);
    }
}

axpby_inst::axpby_inst(transpose tA, tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t beta0,
                       tinytc_value_t B0, bool atomic, location const &lc)
    : blas_a2_inst(IK::axpby_blas_a2, std::move(alpha0), std::move(A0), std::move(beta0),
                   std::move(B0), atomic, lc),
      tA_(tA) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    if (b->dim() < 0 || b->dim() > 2) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_0_1_or_2);
    }

    bool shape_equal = false;
    if (tA_ == transpose::T && a->dim() == 2 && b->dim() == 2) {
        shape_equal = a->shape()[1] == b->shape()[0] && a->shape()[0] == b->shape()[1];
    } else {
        shape_equal = a->shape() == b->shape();
    }

    if (!shape_equal) {
        throw compilation_error(loc(), {&A(), &B()}, status::ir_incompatible_shapes);
    }
}

arith_inst::arith_inst(arithmetic operation, tinytc_value_t a0, tinytc_value_t b0,
                       tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::arith}, operation_(operation) {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    if (a().ty() != ty) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }
    if (b().ty() != ty) {
        throw compilation_error(loc(), {&b()}, status::ir_operand_type_must_match_return_type);
    }

    if (isa<boolean_data_type>(*ty)) {
        auto const inst_supports_bool = [&] {
            switch (operation) {
            case arithmetic::and_:
            case arithmetic::or_:
            case arithmetic::xor_:
                return true;
            default:
                return false;
            }
        }();
        if (!inst_supports_bool) {
            throw compilation_error(loc(), status::ir_boolean_unsupported);
        }
    } else if (isa<coopmatrix_data_type>(*ty)) {
        bool inst_supports_coopmatrix = false;
        switch (operation) {
        case arithmetic::add:
        case arithmetic::sub:
        case arithmetic::mul:
        case arithmetic::div:
            inst_supports_coopmatrix = true;
            break;
        default:
            break;
        }
        if (!inst_supports_coopmatrix) {
            throw compilation_error(loc(), status::ir_coopmatrix_unsupported);
        }
    } else {
        auto sty = get_scalar_type(loc(), ty)->ty();

        bool inst_supports_fp = true;
        bool inst_supports_complex = true;
        switch (operation) {
        case arithmetic::add:
        case arithmetic::sub:
        case arithmetic::mul:
        case arithmetic::div:
            break;
        case arithmetic::min:
        case arithmetic::max:
        case arithmetic::rem:
            inst_supports_complex = false;
            break;
        case arithmetic::and_:
        case arithmetic::or_:
        case arithmetic::xor_:
            inst_supports_fp = false;
            inst_supports_complex = false;
            break;
        case arithmetic::shl:
        case arithmetic::shr:
            inst_supports_fp = false;
            inst_supports_complex = false;
            break;
        }
        if (!inst_supports_fp && is_floating_type(sty)) {
            throw compilation_error(loc(), status::ir_fp_unsupported);
        }
        if (!inst_supports_complex && is_complex_type(sty)) {
            throw compilation_error(loc(), status::ir_complex_unsupported);
        }
    }

    result(0) = value_node{ty, this, lc};
}

arith_unary_inst::arith_unary_inst(arithmetic_unary operation, tinytc_value_t a0,
                                   tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::arith_unary}, operation_(operation) {
    op(op_a, a0);
    loc(lc);

    result(0) = value_node{ty, this, lc};

    // Check if inst is supported for combination of a type and result type
    switch (operation_) {
    case arithmetic_unary::abs:
    case arithmetic_unary::im:
    case arithmetic_unary::re: {
        auto a_ty = get_scalar_type(a().loc(), a());
        auto r_ty = get_scalar_type(loc(), result(0));
        if (r_ty->ty() != component_type(a_ty->ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
        }
        break;
    }
    default:
        if (a().ty() != ty) {
            throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
        }
        break;
    }

    if (isa<boolean_data_type>(*ty)) {
        if (operation_ != arithmetic_unary::not_) {
            throw compilation_error(loc(), status::ir_boolean_unsupported);
        }
    } else if (isa<coopmatrix_data_type>(*ty)) {
        if (operation_ != arithmetic_unary::neg) {
            throw compilation_error(loc(), status::ir_coopmatrix_unsupported);
        }
    } else {
        auto a_ty = get_scalar_type(loc(), a());

        bool inst_supports_int = true;
        bool inst_supports_fp = true;
        bool inst_supports_complex = true;
        switch (operation_) {
        case arithmetic_unary::abs:
        case arithmetic_unary::neg:
            break;
        case arithmetic_unary::not_:
            inst_supports_fp = false;
            inst_supports_complex = false;
            break;
        case arithmetic_unary::conj:
        case arithmetic_unary::im:
        case arithmetic_unary::re:
            inst_supports_int = false;
            inst_supports_fp = false;
            break;
        }
        if (!inst_supports_int && is_integer_type(a_ty->ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_int_unsupported);
        }
        if (!inst_supports_fp && is_floating_type(a_ty->ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_fp_unsupported);
        }
        if (!inst_supports_complex && is_complex_type(a_ty->ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_complex_unsupported);
        }
    }
}

builtin_inst::builtin_inst(builtin btype, tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::builtin}, btype_{btype} {
    loc(lc);

    auto rt = dyn_cast<scalar_data_type>(ty);
    if (!rt) {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }

    switch (builtin_type()) {
    case builtin::group_id:
    case builtin::group_size:
        if (rt->ty() != scalar_type::index) {
            throw compilation_error(loc(), status::ir_expected_index);
        }
        break;
    case builtin::num_subgroups:
    case builtin::subgroup_size:
    case builtin::subgroup_id:
    case builtin::subgroup_local_id:
        if (rt->ty() != scalar_type::i32) {
            throw compilation_error(loc(), status::ir_expected_i32);
        }
        break;
    }

    result(0) = value_node{ty, this, lc};
}

auto builtin_inst::kind() const -> tinytc::inst_execution_kind {
    switch (builtin_type()) {
    case builtin::group_id:
    case builtin::group_size:
    case builtin::num_subgroups:
    case builtin::subgroup_size:
        return tinytc::inst_execution_kind::mixed;
    case builtin::subgroup_id:
    case builtin::subgroup_local_id:
        return tinytc::inst_execution_kind::spmd;
    }
    return tinytc::inst_execution_kind::spmd;
}

cast_inst::cast_inst(tinytc_value_t a0, tinytc_data_type_t to_ty, location const &lc)
    : standard_inst{IK::cast} {
    op(op_a, a0);
    loc(lc);

    if (auto rt = dyn_cast<coopmatrix_data_type>(to_ty); rt) {
        auto ct = dyn_cast<coopmatrix_data_type>(a().ty());
        if (!ct) {
            throw compilation_error(loc(), {&a()}, status::ir_expected_coopmatrix);
        }
        if (ct->rows() != rt->rows() || ct->cols() != rt->cols() || ct->use() != rt->use()) {
            throw compilation_error(lc, {&a()}, status::ir_forbidden_cast);
        }
        if (!is_cast_allowed(ct->component_ty(), rt->component_ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
    } else {
        auto to_ty_scalar = dyn_cast<scalar_data_type>(to_ty);
        if (to_ty_scalar == nullptr) {
            throw compilation_error(lc, status::ir_expected_scalar);
        }

        auto at = get_scalar_type(loc(), a());
        if (!is_cast_allowed(at->ty(), to_ty_scalar->ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
    }

    result(0) = value_node{to_ty, this, loc()};
}

compare_inst::compare_inst(cmp_condition cond, tinytc_value_t a0, tinytc_value_t b0,
                           tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::compare}, cond_(cond) {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    if (!isa<boolean_data_type>(*ty)) {
        throw compilation_error(loc(), status::ir_expected_boolean);
    }

    auto at = get_scalar_type(loc(), a());
    auto bt = get_scalar_type(loc(), b());

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_scalar_mismatch);
    }

    bool inst_supports_complex = true;
    switch (cond_) {
    case cmp_condition::eq:
    case cmp_condition::ne:
        break;
    case cmp_condition::gt:
    case cmp_condition::ge:
    case cmp_condition::lt:
    case cmp_condition::le:
        inst_supports_complex = false;
        break;
    }
    if (!inst_supports_complex && is_complex_type(at->ty())) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_complex_unsupported);
    }

    result(0) = value_node{ty, this, lc};
}

constant_inst::constant_inst(value_type const &value, tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::constant}, value_(value) {
    loc(lc);

    const auto type_ok = [](value_type const &val, scalar_type ty) {
        return (is_integer_type(ty) && std::holds_alternative<std::int64_t>(val)) ||
               (is_floating_type(ty) && std::holds_alternative<double>(val)) ||
               (is_complex_type(ty) && std::holds_alternative<std::complex<double>>(val));
    };

    if (auto bt = dyn_cast<boolean_data_type>(ty); bt) {
        if (!std::holds_alternative<bool>(value_)) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else if (auto st = dyn_cast<scalar_data_type>(ty); st) {
        if (!type_ok(value_, st->ty())) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(ty); ct) {
        if (!type_ok(value_, ct->component_ty())) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else {
        throw compilation_error(loc(), status::ir_expected_coopmatrix_scalar_or_boolean);
    }

    result(0) = value_node{ty, this, lc};
}

auto constant_inst::is_zero() const -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){0}; }, value_);
}
auto constant_inst::is_identity() const -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){1}; }, value_);
}

cooperative_matrix_load_inst::cooperative_matrix_load_inst(transpose t, checked_flag flag,
                                                           tinytc_value_t op0, tinytc_value_t p0,
                                                           tinytc_value_t p1,
                                                           tinytc_data_type_t to_ty,
                                                           location const &lc)
    : standard_inst{IK::cooperative_matrix_load}, t_(t), flag_(flag) {
    op(op_operand, op0);
    op(op_pos0, p0);
    op(op_pos1, p1);
    loc(lc);

    auto rt = dyn_cast<coopmatrix_data_type>(to_ty);
    if (!rt) {
        throw compilation_error(loc(), status::ir_expected_coopmatrix);
    }

    auto ot = get_memref_type(loc(), operand());
    if (ot->element_ty() != rt->component_ty()) {
        throw compilation_error(loc(), {&operand()}, status::ir_scalar_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), {&operand()}, status::ir_expected_memref_order_2);
    }

    check_index_ty(lc, pos0());
    check_index_ty(lc, pos1());

    result(0) = value_node{to_ty, this, lc};
}

cooperative_matrix_mul_add_inst::cooperative_matrix_mul_add_inst(tinytc_value_t a0,
                                                                 tinytc_value_t b0,
                                                                 tinytc_value_t c0,
                                                                 tinytc_data_type_t to_ty,
                                                                 location const &lc)
    : standard_inst{IK::cooperative_matrix_mul_add} {
    op(op_a, a0);
    op(op_b, b0);
    op(op_c, c0);
    loc(lc);

    auto rt = dyn_cast<coopmatrix_data_type>(to_ty);
    if (!rt) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    if (rt->use() != matrix_use::acc) {
        throw compilation_error(loc(), status::ir_invalid_matrix_use);
    }

    auto at = get_coopmatrix_type(loc(), a());
    auto bt = get_coopmatrix_type(loc(), b());
    auto ct = get_coopmatrix_type(loc(), c());
    if (at->use() != matrix_use::a) {
        throw compilation_error(loc(), {&a()}, status::ir_invalid_matrix_use);
    }
    if (bt->use() != matrix_use::b) {
        throw compilation_error(loc(), {&b()}, status::ir_invalid_matrix_use);
    }
    if (ct->use() != matrix_use::acc) {
        throw compilation_error(loc(), {&c()}, status::ir_invalid_matrix_use);
    }

    auto M = rt->rows();
    auto N = rt->cols();
    auto K = at->cols();
    if (rt->rows() != M || rt->cols() != N || ct->rows() != M || ct->cols() != N ||
        at->rows() != M || bt->rows() != K || bt->cols() != N) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "A=" << at->rows() << "x" << at->cols() << ", ";
        oss << "B=" << bt->rows() << "x" << bt->cols() << ", ";
        oss << "C=" << ct->rows() << "x" << ct->cols() << ", ";
        oss << "result=" << rt->rows() << "x" << rt->cols();
        throw compilation_error(loc(), {&a(), &b(), &c()}, status::ir_incompatible_shapes,
                                oss.str());
    }

    const auto AB_ty = promote(at->component_ty(), bt->component_ty());
    if (!AB_ty) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_forbidden_promotion);
    }
    if (!promotable(*AB_ty, ct->component_ty())) {
        throw compilation_error(loc(), {&a(), &b(), &c()}, status::ir_forbidden_promotion);
    }
    if (!is_cast_allowed(ct->component_ty(), rt->component_ty())) {
        throw compilation_error(loc(), {&c()}, status::ir_forbidden_cast);
    }

    result(0) = value_node{to_ty, this, lc};
}

cooperative_matrix_scale_inst::cooperative_matrix_scale_inst(tinytc_value_t a0, tinytc_value_t b0,
                                                             tinytc_data_type_t ty,
                                                             location const &lc)
    : standard_inst{IK::cooperative_matrix_scale} {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    if (b().ty() != ty) {
        throw compilation_error(loc(), {&b()}, status::ir_operand_type_must_match_return_type);
    }

    auto at = get_scalar_type(loc(), a());
    auto bt = get_coopmatrix_type(loc(), b());

    if (at->ty() != bt->component_ty()) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_scalar_mismatch);
    }

    result(0) = value_node{ty, this, lc};
}

cooperative_matrix_store_inst::cooperative_matrix_store_inst(checked_flag cflag, store_flag sflag,
                                                             tinytc_value_t val0,
                                                             tinytc_value_t op0, tinytc_value_t p0,
                                                             tinytc_value_t p1, location const &lc)
    : standard_inst{IK::cooperative_matrix_store}, cflag_(cflag), sflag_(sflag) {
    op(op_val, val0);
    op(op_operand, op0);
    op(op_pos0, p0);
    op(op_pos1, p1);
    loc(lc);

    auto vt = get_coopmatrix_type(loc(), val());
    auto ot = get_memref_type(loc(), operand());
    if (vt->component_ty() != ot->element_ty()) {
        throw compilation_error(loc(), {&val(), &operand()}, status::ir_scalar_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), {&operand()}, status::ir_expected_memref_order_2);
    }

    check_index_ty(lc, pos0());
    check_index_ty(lc, pos1());
}

expand_inst::expand_inst(tinytc_value_t op0, std::int64_t expanded_mode,
                         array_view<std::int64_t> static_expand_shape0,
                         array_view<tinytc_value_t> expand_shape0, tinytc_data_type_t ty,
                         location const &lc)
    : standard_inst{IK::expand, static_cast<std::int64_t>(1 + expand_shape0.size())},
      expanded_mode_(expanded_mode), static_expand_shape_(std::move(static_expand_shape0)) {
    op(0, op0);
    for (std::size_t i = 0; i < expand_shape0.size(); ++i) {
        check_index_ty(loc(), *expand_shape0[i]);
        op(1 + i, expand_shape0[i]);
    }
    loc(lc);

    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    bool const range_ok = 0 <= expanded_mode_ && expanded_mode_ < ot->dim();
    if (!range_ok) {
        throw compilation_error(loc(), {&operand()}, status::ir_out_of_bounds);
    }

    if (static_expand_shape_.size() < 2) {
        throw compilation_error(loc(), status::ir_expand_shape_order_too_small);
    }
    if (std::count(static_expand_shape_.begin(), static_expand_shape_.end(), dynamic) !=
        num_operands() - 1) {
        throw compilation_error(loc(), status::ir_expand_shape_mismatch);
    }

    for (std::int64_t i = 0; i < expanded_mode_; ++i) {
        check_memref_mode(rt, i, ot, i, loc());
    }
    auto stride = ot->stride(expanded_mode_);
    for (std::size_t i = 0; i < static_expand_shape_.size(); ++i) {
        const auto mode = expanded_mode_ + i;
        if (rt->shape(mode) != static_expand_shape()[i]) {
            auto extra_info = std::ostringstream{}
                              << "Size of mode " << mode << " does not match static expand shape ("
                              << rt->shape(mode) << "!=" << static_expand_shape()[i] << ")";
            throw compilation_error(loc(), status::ir_invalid_shape, std::move(extra_info).str());
        }
        if (!is_dynamic_value(rt->stride(mode)) && rt->stride(mode) != stride) {
            auto extra_info = std::ostringstream{} << "Stride of mode " << mode << " is invalid ("
                                                   << rt->stride(mode) << "!=" << stride << ")";
            throw compilation_error(loc(), status::ir_invalid_stride, std::move(extra_info).str());
        }
        stride = is_dynamic_value(stride) || is_dynamic_value(rt->shape(mode))
                     ? dynamic
                     : stride * rt->shape(mode);
    }
    for (std::int64_t i = expanded_mode_ + 1; i < ot->dim(); ++i) {
        check_memref_mode(rt, i + static_expand_shape_.size() - 1, ot, i, loc());
    }

    result(0) = value_node{ty, this, lc};
}

for_inst::for_inst(tinytc_data_type_t loop_var_type, tinytc_value_t from0, tinytc_value_t to0,
                   tinytc_value_t step0, array_view<tinytc_value_t> init_values,
                   array_view<tinytc_data_type_t> return_types, location const &lc)
    : loop_inst{IK::for_loop, (step0 ? 3 : 2) + static_cast<std::int64_t>(init_values.size()),
                static_cast<std::int64_t>(init_values.size())} {
    op(op_from, from0);
    op(op_to, to0);
    if (step0) {
        op(op_step, step0);
    }
    loc(lc);

    body().loc(lc);
    body().defining_inst(this);
    body().set_num_params(1 + init_values.size());
    body().set_param(0, loop_var_type);
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        if (!isa<boolean_data_type>(*return_types[i]) && !isa<scalar_data_type>(*return_types[i]) &&
            !isa<coopmatrix_data_type>(*return_types[i])) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_scalar_or_boolean);
        }
        body().set_param(1 + i, return_types[i]);
        result(i) = value_node{return_types[i], this, lc};
    }
    if (init_values.size() != return_types.size()) {
        throw compilation_error(loc(), status::ir_init_return_type_mismatch);
    }
    for (std::size_t i = 0; i < init_values.size(); ++i) {
        if (init_values[i]->ty() != return_types[i]) {
            throw compilation_error(loc(), {init_values[i]}, status::ir_init_return_type_mismatch);
        }
        op(op_init() + i, init_values[i]);
    }

    auto lvt = get_scalar_type(loc(), loop_var());
    auto fromt = get_scalar_type(loc(), from());
    auto tot = get_scalar_type(loc(), to());

    if (!is_integer_type(lvt->ty())) {
        throw compilation_error(loc(), status::ir_expected_int);
    }
    if (lvt->ty() != fromt->ty()) {
        throw compilation_error(loc(), {&from()}, status::ir_scalar_mismatch);
    }
    if (lvt->ty() != tot->ty()) {
        throw compilation_error(loc(), {&to()}, status::ir_scalar_mismatch);
    }
    if (has_step()) {
        auto stept = get_scalar_type(loc(), step());
        if (lvt->ty() != stept->ty()) {
            throw compilation_error(loc(), {&step()}, status::ir_scalar_mismatch);
        }
    }
}

foreach_inst::foreach_inst(tinytc_data_type_t loop_var_type, array_view<tinytc_value_t> from,
                           array_view<tinytc_value_t> to, location const &lc)
    : loop_inst{IK::foreach_loop, static_cast<std::int64_t>(from.size() + to.size()),
                std::int64_t{0}} {
    std::int64_t op_no = 0;
    for (auto &v : from) {
        op(op_no++, v);
    }
    for (auto &v : to) {
        op(op_no++, v);
    }
    body().loc(lc);
    body().defining_inst(this);
    body().set_num_params(from.size());
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(from.size()); ++i) {
        body().set_param(i, loop_var_type);
    }
    child_region(0).kind(region_kind::spmd);
    loc(lc);

    if (from.size() == 0 || from.size() != to.size()) {
        throw compilation_error(loc(), status::ir_from_to_mismatch);
    }

    if (auto lv_ty = dyn_cast<scalar_data_type>(loop_var_type); lv_ty) {
        if (!is_integer_type(lv_ty->ty()) ||
            std::any_of(op_begin(), op_end(), [&loop_var_type](tinytc_value &val) {
                return val.ty() != loop_var_type;
            })) {
            throw compilation_error(loc(), status::ir_scalar_mismatch);
        }
    } else {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }
}

fuse_inst::fuse_inst(tinytc_value_t op0, std::int64_t from, std::int64_t to, tinytc_data_type_t ty,
                     location const &lc)
    : standard_inst{IK::fuse}, from_(from), to_(to) {
    op(0, op0);
    loc(lc);

    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    bool const range_ok = 0 <= from_ && from_ < to_ && to_ < ot->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    for (std::int64_t i = 0; i < from_; ++i) {
        check_memref_mode(rt, i, ot, i, loc());
    }

    std::int64_t prod = 1;
    for (std::int64_t i = from_; i <= to_; ++i) {
        if (is_dynamic_value(ot->shape(i))) {
            prod = dynamic;
            break;
        }
        prod *= ot->shape(i);
    }
    if (rt->shape(from_) != prod) {
        auto extra_info = std::ostringstream{} << "Size of mode " << from_
                                               << " does not match shape product ("
                                               << rt->shape(from_) << "!=" << prod << ")";
        throw compilation_error(loc(), status::ir_invalid_shape, std::move(extra_info).str());
    }
    check_memref_stride(rt, from_, ot, from_, loc());

    for (std::int64_t i = to_ + 1; i < ot->dim(); ++i) {
        check_memref_mode(rt, i - to_ + from_, ot, i, loc());
    }

    result(0) = value_node{ty, this, lc};
}

load_inst::load_inst(tinytc_value_t op0, array_view<tinytc_value_t> index_list0,
                     tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::load, static_cast<std::int64_t>(1 + index_list0.size())} {
    op(0, op0);
    for (std::size_t i = 0; i < index_list0.size(); ++i) {
        check_index_ty(lc, *index_list0[i]);
        op(1 + i, index_list0[i]);
    }
    loc(lc);

    visit(overloaded{
              [&](group_data_type &g) {
                  if (g.ty() != ty) {
                      throw compilation_error(loc(), {&operand()},
                                              status::ir_operand_type_must_match_return_type);
                  }
                  if (static_cast<std::int64_t>(index_list().size()) != 1) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result(0) = value_node{ty, this, lc};
              },
              [&](memref_data_type &m) {
                  if (m.element_data_ty() != ty) {
                      throw compilation_error(loc(), {&operand()},
                                              status::ir_operand_type_must_match_return_type);
                  }
                  if (m.dim() != static_cast<std::int64_t>(index_list().size())) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result(0) = value_node{ty, this, lc};
              },
              [&](auto &) { throw compilation_error(loc(), status::ir_expected_memref_or_group); }},
          *operand().ty());
}

gemm_inst::gemm_inst(transpose tA, transpose tB, tinytc_value_t alpha0, tinytc_value_t A0,
                     tinytc_value_t B0, tinytc_value_t beta0, tinytc_value_t C0, bool atomic,
                     location const &lc)
    : blas_a3_inst(IK::gemm_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic, lc),
      tA_(tA), tB_(tB) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 2) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_2);
    }
    if (b->dim() != 2) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_2);
    }
    if (c->dim() != 2) {
        throw compilation_error(loc(), {&C()}, status::ir_expected_memref_order_2);
    }

    auto ak = tA_ == transpose::T ? 0 : 1;
    auto bk = tB_ == transpose::T ? 1 : 0;
    auto M = c->shape(0);
    auto N = c->shape(1);
    auto K = a->shape(ak);
    if (a->shape(1 - ak) != M || b->shape(bk) != K || b->shape(1 - bk) != N) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "A=" << a->shape(0) << "x" << a->shape(1) << ", ";
        oss << "B=" << b->shape(0) << "x" << b->shape(1) << ", ";
        oss << "C=" << c->shape(0) << "x" << c->shape(1);
        throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes,
                                oss.str());
    }
}

gemv_inst::gemv_inst(transpose tA, tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                     tinytc_value_t beta0, tinytc_value_t C0, bool atomic, location const &lc)
    : blas_a3_inst(IK::gemv_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic, lc),
      tA_(tA) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 2) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_2);
    }
    if (b->dim() != 1) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_1);
    }
    if (c->dim() != 1) {
        throw compilation_error(loc(), {&C()}, status::ir_expected_memref_order_1);
    }

    auto ak = tA_ == transpose::T ? 0 : 1;
    auto M = c->shape(0);
    auto K = a->shape(ak);
    if (a->shape(1 - ak) != M || b->shape(0) != K) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "A=" << a->shape(0) << "x" << a->shape(1) << ", ";
        oss << "b=" << b->shape(0) << ", ";
        oss << "c=" << c->shape(0);
        throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes,
                                oss.str());
    }
}

ger_inst::ger_inst(tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                   tinytc_value_t beta0, tinytc_value_t C0, bool atomic, location const &lc)
    : blas_a3_inst(IK::ger_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic, lc) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 1) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_1);
    }
    if (b->dim() != 1) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_1);
    }
    if (c->dim() != 2) {
        throw compilation_error(loc(), {&C()}, status::ir_expected_memref_order_2);
    }

    auto M = c->shape(0);
    auto N = c->shape(1);
    if (a->shape(0) != M || b->shape(0) != N) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "a=" << a->shape(0) << ", ";
        oss << "b=" << b->shape(0) << ", ";
        oss << "C=" << c->shape(0) << "x" << c->shape(1);
        throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes,
                                oss.str());
    }
}

hadamard_inst::hadamard_inst(tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                             tinytc_value_t beta0, tinytc_value_t C0, bool atomic,
                             location const &lc)
    : blas_a3_inst(IK::hadamard_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic, lc) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 1 && a->dim() != 2) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_1_or_2);
    }
    if (b->dim() != 1 && b->dim() != 2) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_1_or_2);
    }
    if (c->dim() != 1 && c->dim() != 2) {
        throw compilation_error(loc(), {&C()}, status::ir_expected_memref_order_1_or_2);
    }
    if (c->dim() != a->dim() || c->dim() != b->dim()) {
        throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes);
    }

    auto M = c->shape(0);
    if (c->dim() == 1) {
        if (a->shape(0) != M || b->shape(0) != M) {
            std::ostringstream oss;
            oss << "Got ";
            oss << "a=" << a->shape(0) << ", ";
            oss << "b=" << b->shape(0) << ", ";
            oss << "c=" << c->shape(0);
            throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes,
                                    oss.str());
        }
    } else if (c->dim() == 2) {
        auto N = c->shape(1);
        if (a->shape(0) != M || a->shape(1) != N || b->shape(0) != M || b->shape(1) != N) {
            std::ostringstream oss;
            oss << "Got ";
            oss << "A=" << a->shape(0) << "x" << a->shape(1) << ", ";
            oss << "B=" << b->shape(0) << "x" << b->shape(1) << ", ";
            oss << "C=" << c->shape(0) << "x" << c->shape(1);
            throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_incompatible_shapes,
                                    oss.str());
        }
    }
}

if_inst::if_inst(tinytc_value_t condition, array_view<tinytc_data_type_t> return_types,
                 location const &lc)
    : standard_inst{IK::if_, 1, static_cast<int64_t>(return_types.size())} {
    op(0, condition);
    loc(lc);
    then().loc(lc);
    then().defining_inst(this);
    otherwise().loc(lc);
    otherwise().defining_inst(this);
    if (!isa<boolean_data_type>(*condition->ty())) {
        throw compilation_error(loc(), {condition}, status::ir_expected_boolean);
    }
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        if (!isa<boolean_data_type>(*return_types[i]) && !isa<scalar_data_type>(*return_types[i]) &&
            !isa<coopmatrix_data_type>(*return_types[i])) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_scalar_or_boolean);
        }
        result(i) = value_node{return_types[i], this, lc};
    }
}

parallel_inst::parallel_inst(location const &lc) : standard_inst{IK::parallel} {
    loc(lc);

    child_region(0).kind(region_kind::spmd);
    child_region(0).loc(lc);
    child_region(0).defining_inst(this);
}

size_inst::size_inst(tinytc_value_t op0, std::int64_t mode, tinytc_data_type_t ty,
                     location const &lc)
    : standard_inst{IK::size}, mode_(mode) {
    op(0, op0);
    loc(lc);

    auto rt = dyn_cast<scalar_data_type>(ty);
    if (!rt || rt->ty() != scalar_type::index) {
        throw compilation_error(loc(), status::ir_expected_index);
    }

    const bool range_ok =
        visit(overloaded{[&](group_data_type &) -> bool { return 0 <= mode_ && mode_ < 1; },
                         [&](memref_data_type &m) -> bool { return 0 <= mode_ && mode_ < m.dim(); },
                         [&](auto &) -> bool {
                             throw compilation_error(loc(), status::ir_expected_memref_or_group);
                         }},
              *operand().ty());
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    result(0) = value_node{ty, this, lc};
}

subgroup_broadcast_inst::subgroup_broadcast_inst(tinytc_value_t a0, tinytc_value_t idx0,
                                                 tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::subgroup_broadcast} {
    op(0, a0);
    op(1, idx0);
    loc(lc);

    if (!isa<scalar_data_type>(*ty)) {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }

    if (a().ty() != ty) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }

    if (auto idxt = dyn_cast<scalar_data_type>(idx().ty());
        !idxt || idxt->ty() != scalar_type::i32) {
        throw compilation_error(loc(), {&idx()}, status::ir_expected_i32);
    }

    result(0) = value_node{ty, this, lc};
}

subview_inst::subview_inst(tinytc_value_t op0, array_view<std::int64_t> static_offsets0,
                           array_view<std::int64_t> static_sizes0,
                           array_view<tinytc_value_t> offsets0, array_view<tinytc_value_t> sizes0,
                           tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::subview, static_cast<std::int64_t>(1 + offsets0.size() + sizes0.size())},
      static_offsets_(std::move(static_offsets0)), static_sizes_(std::move(static_sizes0)) {
    op(0, op0);
    {
        std::size_t i = 1;
        for (auto const &val : offsets0) {
            check_index_ty(loc(), *val);
            op(i++, val);
        }
        num_dyn_offsets_ = i - 1;
        for (auto const &val : sizes0) {
            check_index_ty(loc(), *val);
            op(i++, val);
        }
    }
    loc(lc);

    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    if (ot->dim() != static_cast<std::int64_t>(static_offsets_.size()) ||
        ot->dim() != static_cast<std::int64_t>(static_sizes_.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }
    if (std::count(static_offsets_.begin(), static_offsets_.end(), dynamic) != num_dyn_offsets_ ||
        std::count(static_sizes_.begin(), static_sizes_.end(), dynamic) !=
            num_operands() - num_dyn_offsets_ - 1) {
        throw compilation_error(loc(), status::ir_subview_mismatch);
    }

    std::int64_t ri = 0;
    for (std::int64_t i = 0; i < ot->dim(); ++i) {
        auto offset = static_offsets_[i];
        auto size = static_sizes_[i];
        if ((offset < 0 && !is_dynamic_value(offset)) || (size < 0 && !is_dynamic_value(size))) {
            throw compilation_error(loc(), status::ir_invalid_slice);
        }
        if (size > 0 || is_dynamic_value(size)) {
            if (rt->shape(ri) != size) {
                auto extra_info = std::ostringstream{} << "Size of mode " << ri
                                                       << " does not match slice size ["
                                                       << rt->shape(ri) << "!=" << size << "]";
                throw compilation_error(loc(), status::ir_invalid_shape,
                                        std::move(extra_info).str());
            }
            check_memref_stride(rt, ri, ot, i, loc());
            ++ri;
        }
    }

    result(0) = value_node{ty, this, lc};
}

store_inst::store_inst(store_flag flag, tinytc_value_t val0, tinytc_value_t op0,
                       array_view<tinytc_value_t> index_list0, location const &lc)
    : standard_inst{IK::store, static_cast<std::int64_t>(2 + index_list0.size())}, flag_{flag} {
    op(op_val, val0);
    op(op_operand, op0);
    {
        std::size_t i = op_operand;
        for (auto const &val : index_list0) {
            check_index_ty(lc, *val);
            op(++i, val);
        }
    }
    loc(lc);

    auto v = get_scalar_type(loc(), val());
    auto o = get_memref_type(loc(), operand());

    if (v->ty() != o->element_ty()) {
        throw compilation_error(loc(), {&val(), &operand()}, status::ir_scalar_mismatch);
    }

    if (o->dim() != static_cast<std::int64_t>(index_list0.size())) {
        throw compilation_error(loc(), {&operand()}, status::ir_invalid_number_of_indices);
    }
}

sum_inst::sum_inst(transpose tA, tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t beta0,
                   tinytc_value_t B0, bool atomic, location const &lc)
    : blas_a2_inst(IK::sum_blas_a2, std::move(alpha0), std::move(A0), std::move(beta0),
                   std::move(B0), atomic, lc),
      tA_(tA) {
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    if (b->dim() == 1 && a->dim() != 2) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_2);
    }
    if (b->dim() == 0 && a->dim() != 1) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_memref_order_1);
    }
    if (b->dim() != 0 && b->dim() != 1) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_0_or_1);
    }

    if (a->dim() == 2) {
        if (a->shape(tA_ == transpose::T ? 1 : 0) != b->shape(0)) {
            throw compilation_error(loc(), {&A(), &B()}, status::ir_incompatible_shapes);
        }
    }
}

work_group_inst::work_group_inst(work_group_operation operation, tinytc_value_t operand0,
                                 tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::work_group}, operation_(operation) {
    loc(lc);
    op(0, operand0);

    if (!isa<scalar_data_type>(*ty)) {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }

    if (operand().ty() != ty) {
        throw compilation_error(loc(), {&operand()},
                                status::ir_operand_type_must_match_return_type);
    }

    result(0) = value_node{ty, this, lc};
}

yield_inst::yield_inst(array_view<tinytc_value_t> vals, location const &lc)
    : standard_inst{IK::yield, static_cast<std::int64_t>(vals.size())} {
    loc(lc);
    for (std::size_t i = 0; i < vals.size(); ++i) {
        op(i, vals[i]);
    }
}

} // namespace tinytc
