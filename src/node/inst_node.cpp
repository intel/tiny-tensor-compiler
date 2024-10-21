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
#include <sstream>

namespace tinytc {

scalar_data_type *get_scalar_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<scalar_data_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    return m;
}

memref_data_type *get_memref_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<memref_data_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_memref);
    }
    return m;
}

void check_index_ty(location const &loc, tinytc_data_type_t ty) {
    if (auto sty = dyn_cast<scalar_data_type>(ty); !sty || sty->ty() != scalar_type::index) {
        throw compilation_error(loc, status::ir_expected_index);
    }
}

blas_a2_inst::blas_a2_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t beta,
                           tinytc_value_t B, bool atomic)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha, alpha);
    op(op_A, A);
    op(op_beta, beta);
    op(op_B, B);
}

blas_a3_inst::blas_a3_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                           tinytc_value_t beta, tinytc_value_t C, bool atomic)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha, alpha);
    op(op_A, A);
    op(op_B, B);
    op(op_beta, beta);
    op(op_C, C);
}

loop_inst::loop_inst(IK tid, tinytc_value_t from0, tinytc_value_t to0, tinytc_value_t step0,
                     array_view<tinytc_value_t> init_values,
                     array_view<tinytc_data_type_t> return_types, tinytc_data_type_t loop_var_type,
                     location const &lc)
    : standard_inst{tid, (step0 ? 3 : 2) + static_cast<std::int64_t>(init_values.size()),
                    static_cast<std::int64_t>(return_types.size())} {
    if (init_values.size() != return_types.size()) {
        throw compilation_error(loc(), status::ir_init_return_mismatch);
    }

    op(op_from, from0);
    op(op_to, to0);
    if (step0) {
        op(op_step, step0);
    }

    body().set_num_params(1 + return_types.size());
    body().set_param(0, loop_var_type, lc);
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        body().set_param(1 + i, return_types[i], lc);
        result(i) = value_node{return_types[i], this, lc};
    }
    for (std::size_t i = 0; i < init_values.size(); ++i) {
        if (!isa<scalar_data_type>(*return_types[i]) &&
            !isa<coopmatrix_data_type>(*return_types[i])) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_or_scalar);
        }
        if (init_values[i]->ty() != return_types[i]) {
            throw compilation_error(loc(), status::ir_init_return_mismatch);
        }
        op(op_init() + i, init_values[i]);
    }
    loc(lc);

    auto lvt = get_scalar_type(loc(), loop_var());
    auto fromt = get_scalar_type(loc(), from());
    auto tot = get_scalar_type(loc(), to());
    bool step_ok = true;
    if (has_step()) {
        auto stept = get_scalar_type(loc(), step());
        step_ok = lvt->ty() == stept->ty();
    }

    if (!is_integer_type(lvt->ty()) || lvt->ty() != fromt->ty() || lvt->ty() != tot->ty() ||
        !step_ok) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
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
                   std::move(B0), atomic),
      tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    bool shape_equal = false;
    if (tA_ == transpose::T && a->dim() == 2 && b->dim() == 2) {
        shape_equal = a->shape()[1] == b->shape()[0] && a->shape()[0] == b->shape()[1];
    } else {
        shape_equal = a->shape() == b->shape();
    }

    if (!shape_equal) {
        throw compilation_error(loc(), status::ir_incompatible_shapes);
    }

    if (b->dim() > 2) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix);
    }
}

arith_inst::arith_inst(arithmetic operation, tinytc_value_t a0, tinytc_value_t b0,
                       location const &lc)
    : standard_inst{IK::arith}, operation_(operation) {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    if (isa<coopmatrix_data_type>(*a().ty())) {
        if (!isa<coopmatrix_data_type>(*b().ty())) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix);
        }
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
        auto a_ty = get_scalar_type(loc(), a())->ty();
        auto b_ty = get_scalar_type(loc(), b())->ty();

        if (a_ty != b_ty) {
            throw compilation_error(loc(), status::ir_scalar_mismatch);
        }
        bool inst_supports_fp = true;
        bool inst_supports_complex = true;
        bool inst_supports_i1 = true;
        switch (operation) {
        case arithmetic::add:
        case arithmetic::sub:
        case arithmetic::mul:
        case arithmetic::div:
            break;
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
            inst_supports_i1 = false;
            inst_supports_fp = false;
            inst_supports_complex = false;
            break;
        }
        if (!inst_supports_i1 && a_ty == scalar_type::i1) {
            throw compilation_error(loc(), status::ir_i1_unsupported);
        }
        if (!inst_supports_fp && is_floating_type(a_ty)) {
            throw compilation_error(loc(), status::ir_fp_unsupported);
        }
        if (!inst_supports_complex && is_complex_type(a_ty)) {
            throw compilation_error(loc(), status::ir_complex_unsupported);
        }
    }

    result(0) = value_node{a().ty(), this, lc};
}

arith_unary_inst::arith_unary_inst(arithmetic_unary operation, tinytc_value_t a0,
                                   location const &lc)
    : standard_inst{IK::arith_unary}, operation_(operation) {
    op(op_a, a0);
    loc(lc);

    tinytc_data_type_t to_ty = nullptr;

    if (isa<coopmatrix_data_type>(*a().ty())) {
        if (operation_ != arithmetic_unary::neg) {
            throw compilation_error(loc(), status::ir_coopmatrix_unsupported);
        }
        to_ty = a().ty();
    } else {
        auto a_ty = get_scalar_type(loc(), a());
        to_ty = a_ty;

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
            throw compilation_error(loc(), status::ir_int_unsupported);
        }
        if (!inst_supports_fp && is_floating_type(a_ty->ty())) {
            throw compilation_error(loc(), status::ir_fp_unsupported);
        }
        if (!inst_supports_complex && is_complex_type(a_ty->ty())) {
            throw compilation_error(loc(), status::ir_complex_unsupported);
        }
        switch (operation_) {
        case arithmetic_unary::abs:
        case arithmetic_unary::im:
        case arithmetic_unary::re:
            to_ty = scalar_data_type::get(a_ty->context(), element_type(a_ty->ty()));
            break;
        default:
            break;
        }
    }

    result(0) = value_node{to_ty, this, lc};
}

cast_inst::cast_inst(tinytc_value_t a0, tinytc_data_type_t to_ty, location const &lc)
    : standard_inst{IK::cast} {
    op(op_a, a0);
    loc(lc);

    auto const check_scalar_casting_rules = [](scalar_type a_ty, scalar_type r_ty,
                                               location const &lc) {
        if (is_complex_type(a_ty) && !is_complex_type(r_ty)) {
            throw compilation_error(lc, status::ir_forbidden_cast);
        }
    };

    if (auto ct = dyn_cast<coopmatrix_data_type>(a().ty()); ct) {
        auto rt = dyn_cast<coopmatrix_data_type>(to_ty);
        if (!rt) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix);
        }
        if (ct->rows() != rt->rows() || ct->cols() != rt->cols() || ct->use() != rt->use()) {
            throw compilation_error(lc, status::ir_forbidden_cast);
        }
        check_scalar_casting_rules(ct->component_ty(), rt->component_ty(), loc());
    } else {
        auto rt = dyn_cast<scalar_data_type>(to_ty);
        if (rt == nullptr) {
            throw compilation_error(lc, status::ir_expected_scalar);
        }

        auto at = get_scalar_type(loc(), a());
        check_scalar_casting_rules(at->ty(), rt->ty(), loc());
    }

    result(0) = value_node{to_ty, this, loc()};
}

compare_inst::compare_inst(cmp_condition cond, tinytc_value_t a0, tinytc_value_t b0,
                           location const &lc)
    : standard_inst{IK::compare}, cond_(cond) {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    auto at = get_scalar_type(loc(), a());
    auto bt = get_scalar_type(loc(), b());

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
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
        throw compilation_error(loc(), status::ir_complex_unsupported);
    }

    auto result_ty = scalar_data_type::get(at->context(), scalar_type::i1);
    result(0) = value_node{result_ty, this, lc};
}

constant_inst::constant_inst(value_type const &value, tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::constant}, value_(value) {
    loc(lc);

    const auto type_ok = [](value_type const &val, scalar_type ty) {
        return (is_integer_type(ty) && std::holds_alternative<std::int64_t>(val)) ||
               (is_floating_type(ty) && std::holds_alternative<double>(val)) ||
               (is_complex_type(ty) && std::holds_alternative<std::complex<double>>(val));
    };

    if (auto st = dyn_cast<scalar_data_type>(ty); st) {
        if (!type_ok(value_, st->ty())) {
            throw compilation_error(loc(), status::ir_scalar_mismatch);
        }
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(ty); ct) {
        if (!type_ok(value_, ct->component_ty())) {
            throw compilation_error(loc(), status::ir_scalar_mismatch);
        }
    } else {
        throw compilation_error(loc(), status::ir_expected_coopmatrix_or_scalar);
    }

    result(0) = value_node{ty, this, lc};
}

auto constant_inst::is_zero() const -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){0}; }, value_);
}
auto constant_inst::is_identity() const -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){1}; }, value_);
}

cooperative_matrix_load_inst::cooperative_matrix_load_inst(transpose t, bool checked,
                                                           tinytc_value_t op0, tinytc_value_t p0,
                                                           tinytc_value_t p1,
                                                           tinytc_data_type_t to_ty,
                                                           location const &lc)
    : standard_inst{IK::cooperative_matrix_load}, t_(t), checked_(checked) {
    op(op_operand, op0);
    op(op_pos0, p0);
    op(op_pos1, p1);
    loc(lc);

    auto ot = dyn_cast<memref_data_type>(operand().ty());
    if (!ot) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    auto rt = dyn_cast<coopmatrix_data_type>(to_ty);
    if (!rt) {
        throw compilation_error(loc(), status::ir_expected_coopmatrix);
    }
    if (ot->element_ty() != rt->component_ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), status::ir_expected_matrix);
    }

    check_index_ty(lc, pos0().ty());
    check_index_ty(lc, pos1().ty());

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

    auto at = dyn_cast<coopmatrix_data_type>(a().ty());
    auto bt = dyn_cast<coopmatrix_data_type>(b().ty());
    auto ct = dyn_cast<coopmatrix_data_type>(c().ty());
    auto rt = dyn_cast<coopmatrix_data_type>(to_ty);
    if (!at || !bt || !ct || !rt) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }

    auto M = rt->rows();
    auto N = rt->cols();
    auto K = at->cols();
    if (ct->rows() != M || ct->cols() != N || at->rows() != M || bt->rows() != K ||
        bt->cols() != N) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "A=" << at->rows() << "x" << at->cols() << ", ";
        oss << "B=" << bt->rows() << "x" << bt->cols() << ", ";
        oss << "C=" << ct->rows() << "x" << ct->cols() << ", ";
        oss << "result=" << rt->rows() << "x" << rt->cols();
        throw compilation_error(loc(), status::ir_incompatible_shapes, oss.str());
    }
    if (at->use() != matrix_use::a && bt->use() != matrix_use::b && ct->use() != matrix_use::acc &&
        rt->use() != matrix_use::acc) {
        throw compilation_error(loc(), status::ir_invalid_matrix_use);
    }

    result(0) = value_node{to_ty, this, lc};
}

cooperative_matrix_scale_inst::cooperative_matrix_scale_inst(tinytc_value_t a0, tinytc_value_t b0,
                                                             location const &lc)
    : standard_inst{IK::cooperative_matrix_scale} {
    op(op_a, a0);
    op(op_b, b0);
    loc(lc);

    auto at = dyn_cast<scalar_data_type>(a().ty());
    if (!at) {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }
    auto bt = dyn_cast<coopmatrix_data_type>(b().ty());
    if (!bt) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }

    if (at->ty() != bt->component_ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }

    result(0) = value_node{b().ty(), this, lc};
}

cooperative_matrix_store_inst::cooperative_matrix_store_inst(bool checked, store_flag flag,
                                                             tinytc_value_t val0,
                                                             tinytc_value_t op0, tinytc_value_t p0,
                                                             tinytc_value_t p1, location const &lc)
    : standard_inst{IK::cooperative_matrix_store}, checked_(checked), flag_(flag) {
    op(op_val, val0);
    op(op_operand, op0);
    op(op_pos0, p0);
    op(op_pos1, p1);
    loc(lc);

    auto vt = dyn_cast<coopmatrix_data_type>(val().ty());
    if (!vt) {
        throw compilation_error(loc(), status::ir_expected_coopmatrix);
    }
    auto ot = dyn_cast<memref_data_type>(operand().ty());
    if (!ot) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    if (vt->component_ty() != ot->element_ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), status::ir_expected_matrix);
    }

    check_index_ty(lc, pos0().ty());
    check_index_ty(lc, pos1().ty());
}

expand_inst::expand_inst(tinytc_value_t op0, std::int64_t expanded_mode,
                         array_view<std::int64_t> static_expand_shape0,
                         array_view<tinytc_value_t> expand_shape0, location const &lc)
    : standard_inst{IK::expand, static_cast<std::int64_t>(1 + expand_shape0.size())},
      expanded_mode_(expanded_mode), static_expand_shape_(std::move(static_expand_shape0)) {
    op(0, op0);
    for (std::size_t i = 0; i < expand_shape0.size(); ++i) {
        op(1 + i, expand_shape0[i]);
    }
    loc(lc);

    auto m = get_memref_type(loc(), operand());
    bool const range_ok = 0 <= expanded_mode_ && expanded_mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    if (static_expand_shape_.size() < 2) {
        throw compilation_error(loc(), status::ir_expand_shape_order_too_small);
    }
    if (std::count(static_expand_shape_.begin(), static_expand_shape_.end(), dynamic) !=
        num_operands() - 1) {
        throw compilation_error(loc(), status::ir_expand_shape_mismatch);
    }

    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim() + static_expand_shape_.size() - 1);
    stride.reserve(m->dim() + static_expand_shape_.size() - 1);
    for (std::int64_t i = 0; i < expanded_mode_; ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }

    stride.push_back(m->stride(expanded_mode_));
    shape.push_back(static_expand_shape_[0]);
    for (std::size_t j = 1; j < static_expand_shape_.size(); ++j) {
        stride.push_back(is_dynamic_value(stride.back()) || is_dynamic_value(shape.back())
                             ? dynamic
                             : stride.back() * shape.back());
        shape.push_back(static_expand_shape_[j]);
    }
    for (std::int64_t i = expanded_mode_ + 1; i < m->dim(); ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }

    auto result_ty = memref_data_type::get(m->element_data_ty(), shape, stride, m->addrspace());
    result(0) = value_node{result_ty, this, lc};
}

fuse_inst::fuse_inst(tinytc_value_t op0, std::int64_t from, std::int64_t to, location const &lc)
    : standard_inst{IK::fuse}, from_(from), to_(to) {
    op(0, op0);
    loc(lc);
    auto m = get_memref_type(loc(), operand());
    bool const range_ok = 0 <= from_ && from_ < to_ && to_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }
    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim());
    stride.reserve(m->dim());
    std::int64_t i = 0;
    for (; i < from_; ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }
    std::int64_t prod = 1;
    for (; i <= to_; ++i) {
        if (is_dynamic_value(m->shape(i))) {
            prod = dynamic;
            break;
        }
        prod *= m->shape(i);
    }
    shape.push_back(prod);
    stride.push_back(m->stride(from_));
    for (i = to_ + 1; i < m->dim(); ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }
    auto result_ty = memref_data_type::get(m->element_data_ty(), shape, stride, m->addrspace());
    result(0) = value_node{result_ty, this, lc};
}

load_inst::load_inst(tinytc_value_t op0, array_view<tinytc_value_t> index_list0, location const &lc)
    : standard_inst{IK::load, static_cast<std::int64_t>(1 + index_list0.size())} {
    op(0, op0);
    for (std::size_t i = 0; i < index_list0.size(); ++i) {
        check_index_ty(lc, index_list0[i]->ty());
        op(1 + i, index_list0[i]);
    }
    loc(lc);

    visit(overloaded{
              [&](group_data_type &g) {
                  if (static_cast<std::int64_t>(index_list().size()) != 1) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result(0) = value_node{g.ty(), this, lc};
              },
              [&](memref_data_type &m) {
                  if (m.dim() != static_cast<std::int64_t>(index_list().size())) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  auto result_ty = scalar_data_type::get(m.context(), m.element_ty());
                  result(0) = value_node{result_ty, this, lc};
              },
              [&](auto &) { throw compilation_error(loc(), status::ir_expected_memref_or_group); }},
          *operand().ty());
}

gemm_inst::gemm_inst(transpose tA, transpose tB, tinytc_value_t alpha0, tinytc_value_t A0,
                     tinytc_value_t B0, tinytc_value_t beta0, tinytc_value_t C0, bool atomic,
                     location const &lc)
    : blas_a3_inst(IK::gemm_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic),
      tA_(tA), tB_(tB) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 2 || b->dim() != 2 || c->dim() != 2) {
        throw compilation_error(loc(), status::ir_expected_matrix,
                                "gemm only supported for memref of order 2 (matrices)");
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
        throw compilation_error(loc(), status::ir_incompatible_shapes, oss.str());
    }
}

gemv_inst::gemv_inst(transpose tA, tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                     tinytc_value_t beta0, tinytc_value_t C0, bool atomic, location const &lc)
    : blas_a3_inst(IK::gemv_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic),
      tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 2 || b->dim() != 1 || c->dim() != 1) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix,
                                "gemv only supports matrix-vector products");
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
        throw compilation_error(loc(), status::ir_incompatible_shapes, oss.str());
    }
}

ger_inst::ger_inst(tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                   tinytc_value_t beta0, tinytc_value_t C0, bool atomic, location const &lc)
    : blas_a3_inst(IK::ger_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 1 || b->dim() != 1 || c->dim() != 2) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix,
                                "ger requires two vectors as input and one matrix as output");
    }

    auto M = c->shape(0);
    auto N = c->shape(1);
    if (a->shape(0) != M || b->shape(0) != N) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "a=" << a->shape(0) << ", ";
        oss << "b=" << b->shape(0) << ", ";
        oss << "C=" << c->shape(0) << "x" << c->shape(1);
        throw compilation_error(loc(), status::ir_incompatible_shapes, oss.str());
    }
}

foreach_inst::foreach_inst(tinytc_value_t from, tinytc_value_t to, tinytc_data_type_t loop_var_type,
                           location const &loc)
    : loop_inst{
          IK::foreach_loop, std::move(from), std::move(to), nullptr, {}, {}, loop_var_type, loc} {
    child_region(0).kind(region_kind::spmd);
}

hadamard_inst::hadamard_inst(tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t B0,
                             tinytc_value_t beta0, tinytc_value_t C0, bool atomic,
                             location const &lc)
    : blas_a3_inst(IK::hadamard_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 1 || b->dim() != 1 || c->dim() != 1) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix,
                                "hadamard requires two vectors as input and one vector as output");
    }

    auto M = c->shape(0);
    if (a->shape(0) != M || b->shape(0) != M) {
        std::ostringstream oss;
        oss << "Got ";
        oss << "a=" << a->shape(0) << ", ";
        oss << "b=" << b->shape(0) << ", ";
        oss << "c=" << c->shape(0);
        throw compilation_error(loc(), status::ir_incompatible_shapes, oss.str());
    }
}

if_inst::if_inst(tinytc_value_t condition, array_view<tinytc_data_type_t> return_types,
                 location const &lc)
    : standard_inst{IK::if_, 1, static_cast<int64_t>(return_types.size())} {
    op(0, condition);
    loc(lc);
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        if (!isa<scalar_data_type>(*return_types[i]) &&
            !isa<coopmatrix_data_type>(*return_types[i])) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_or_scalar);
        }
        result(i) = value_node{return_types[i], this, lc};
    }
}

parallel_inst::parallel_inst(location const &lc) : standard_inst{IK::parallel} {
    loc(lc);

    child_region(0).kind(region_kind::spmd);
}

size_inst::size_inst(tinytc_value_t op0, std::int64_t mode, location const &lc)
    : standard_inst{IK::size}, mode_(mode) {
    op(0, op0);
    loc(lc);
    auto m = get_memref_type(loc(), operand());
    bool const range_ok = 0 <= mode_ && mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    auto result_ty = scalar_data_type::get(op(0).context(), scalar_type::index);
    result(0) = value_node{result_ty, this, lc};
}

subview_inst::subview_inst(tinytc_value_t op0, array_view<std::int64_t> static_offsets0,
                           array_view<std::int64_t> static_sizes0,
                           array_view<tinytc_value_t> offsets0, array_view<tinytc_value_t> sizes0,
                           location const &lc)
    : standard_inst{IK::subview, static_cast<std::int64_t>(1 + offsets0.size() + sizes0.size())},
      static_offsets_(std::move(static_offsets0)), static_sizes_(std::move(static_sizes0)) {
    op(0, op0);
    {
        std::size_t i = 1;
        for (auto const &val : offsets0) {
            op(i++, val);
        }
        num_dyn_offsets_ = i - 1;
        for (auto const &val : sizes0) {
            op(i++, val);
        }
    }
    loc(lc);

    auto m = get_memref_type(loc(), operand());
    if (m->dim() != static_cast<std::int64_t>(static_offsets_.size()) ||
        m->dim() != static_cast<std::int64_t>(static_sizes_.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }
    if (std::count(static_offsets_.begin(), static_offsets_.end(), dynamic) != num_dyn_offsets_ ||
        std::count(static_sizes_.begin(), static_sizes_.end(), dynamic) !=
            num_operands() - num_dyn_offsets_ - 1) {
        throw compilation_error(loc(), status::ir_subview_mismatch);
    }

    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim());
    stride.reserve(m->dim());
    for (std::int64_t i = 0; i < m->dim(); ++i) {
        auto offset = static_offsets_[i];
        auto size = static_sizes_[i];
        if ((offset < 0 && !is_dynamic_value(offset)) || (size < 0 && !is_dynamic_value(size))) {
            throw compilation_error(loc(), status::ir_invalid_slice);
        }
        if (size > 0 || is_dynamic_value(size)) {
            shape.push_back(size);
            stride.push_back(m->stride(i));
        }
    }

    auto result_ty = memref_data_type::get(m->element_data_ty(), shape, stride, m->addrspace());
    result(0) = value_node{result_ty, this, lc};
}

store_inst::store_inst(store_flag flag, tinytc_value_t val0, tinytc_value_t op0,
                       array_view<tinytc_value_t> index_list0, location const &lc)
    : standard_inst{IK::store, static_cast<std::int64_t>(2 + index_list0.size())}, flag_{flag} {
    op(op_val, val0);
    op(op_operand, op0);
    {
        std::size_t i = op_operand;
        for (auto const &val : index_list0) {
            check_index_ty(lc, val->ty());
            op(++i, val);
        }
    }
    loc(lc);

    auto v = get_scalar_type(loc(), val());
    auto o = get_memref_type(loc(), operand());

    if (v->ty() != o->element_ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }

    if (o->dim() != static_cast<std::int64_t>(index_list0.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }
}

sum_inst::sum_inst(transpose tA, tinytc_value_t alpha0, tinytc_value_t A0, tinytc_value_t beta0,
                   tinytc_value_t B0, bool atomic, location const &lc)
    : blas_a2_inst(IK::sum_blas_a2, std::move(alpha0), std::move(A0), std::move(beta0),
                   std::move(B0), atomic),
      tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    bool const size_ok = (a->dim() == 2 && b->dim() == 1) || (a->dim() == 1 && b->dim() == 0);
    if (!size_ok) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix);
    }

    if (a->dim() == 2) {
        if (a->shape(tA_ == transpose::T ? 1 : 0) != b->shape(0)) {
            throw compilation_error(loc(), status::ir_incompatible_shapes);
        }
    }
}

yield_inst::yield_inst(array_view<tinytc_value_t> vals, location const &lc)
    : standard_inst{IK::yield, static_cast<std::int64_t>(vals.size())} {
    loc(lc);
    for (std::size_t i = 0; i < vals.size(); ++i) {
        op(i, vals[i]);
    }
}

} // namespace tinytc
