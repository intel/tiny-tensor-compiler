// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/inst_view.hpp"
#include "node/region.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "scalar_type.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/iterator.hpp"
#include "util/overloaded.hpp"

#include <algorithm>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc {

coopmatrix_type *get_coopmatrix_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<coopmatrix_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_coopmatrix);
    }
    return m;
}

number_type *get_scalar_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<number_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_number);
    }
    return m;
}

memref_type *get_memref_type(location const &loc, tinytc_value const &v) {
    auto m = dyn_cast<memref_type>(v.ty());
    if (m == nullptr) {
        throw compilation_error(loc, {&v}, status::ir_expected_memref);
    }
    return m;
}

void check_index_ty(location const &loc, tinytc_value const &v) {
    if (!isa<index_type>(*v.ty())) {
        throw compilation_error(loc, {&v}, status::ir_expected_index);
    }
}

void check_memref_shape(memref_type *rt, std::int64_t ri, memref_type *ot, std::int64_t oi,
                        location const &loc) {
    if (rt->shape(ri) != ot->shape(oi)) {
        auto extra_info = std::ostringstream{} << "Size of mode " << ri
                                               << " does not match operand mode " << oi << " ["
                                               << rt->shape(ri) << "!=" << ot->shape(oi) << "]";
        throw compilation_error(loc, status::ir_invalid_shape, std::move(extra_info).str());
    }
}
void check_memref_stride(memref_type *rt, std::int64_t ri, memref_type *ot, std::int64_t oi,
                         location const &loc) {
    if (!is_dynamic_value(rt->stride(ri)) && rt->stride(ri) != ot->stride(oi)) {
        auto extra_info = std::ostringstream{} << "Stride of mode " << ri
                                               << " does not match operand stride " << oi << " ["
                                               << rt->stride(ri) << "!=" << ot->stride(oi) << "]";
        throw compilation_error(loc, status::ir_invalid_stride, std::move(extra_info).str());
    }
}

void check_memref_mode(memref_type *rt, std::int64_t ri, memref_type *ot, std::int64_t oi,
                       location const &loc) {
    check_memref_shape(rt, ri, ot, oi, loc);
    check_memref_stride(rt, ri, ot, oi, loc);
}

auto get_and_check_memref_type_addrspace(tinytc_value const &operand, tinytc_type_t ty,
                                         location const &loc)
    -> std::pair<memref_type *, memref_type *> {
    auto rt = dyn_cast<memref_type>(ty);
    if (!rt) {
        throw compilation_error(loc, status::ir_expected_memref);
    }
    auto ot = get_memref_type(loc, operand);
    if (rt->element_ty() != ot->element_ty()) {
        throw compilation_error(loc, {&operand}, status::ir_number_mismatch);
    }
    if (rt->addrspace() != ot->addrspace()) {
        throw compilation_error(loc, {&operand}, status::ir_address_space_mismatch);
    }
    return {ot, rt};
}

void alloca_inst::setup_and_check() {
    auto memref = dyn_cast<memref_type>(result().ty());
    if (memref == nullptr) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    if (memref->addrspace() != address_space::local) {
        throw compilation_error(loc(), status::ir_expected_local_address_space);
    }

    stack_ptr(-1);
}

void barrier_inst::setup_and_check() {}

auto barrier_inst::has_fence(address_space as) -> bool {
    return (fence_flags() & static_cast<tinytc_address_spaces_t>(as)) > 0;
}

void cast_inst::setup_and_check() {
    auto to_ty = result().ty();

    if (auto rt = dyn_cast<coopmatrix_type>(to_ty); rt) {
        auto ct = dyn_cast<coopmatrix_type>(a().ty());
        if (!ct) {
            throw compilation_error(loc(), {&a()}, status::ir_expected_coopmatrix);
        }
        if (ct->rows() != rt->rows() || ct->cols() != rt->cols()) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
        const bool use_matches = ct->use() == rt->use();
        const bool use_conversion_allowed =
            ct->use() == matrix_use::acc &&
            (rt->use() == matrix_use::a || rt->use() == matrix_use::b);
        if (!use_matches && !use_conversion_allowed) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
        if (!is_cast_allowed(ct->component_ty(), rt->component_ty())) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
    } else {
        if (!isa<number_type>(*to_ty)) {
            throw compilation_error(loc(), status::ir_expected_number);
        }

        if (!is_cast_allowed(a().ty(), to_ty)) {
            throw compilation_error(loc(), {&a()}, status::ir_forbidden_cast);
        }
    }
}

void constant_inst::setup_and_check() {
    auto ty = result().ty();

    const auto type_ok = [](constant_value_type const &val, tinytc_type_t ty) {
        return (isa<integer_type>(*ty) && std::holds_alternative<std::int64_t>(val)) ||
               (isa<float_type>(*ty) && std::holds_alternative<double>(val)) ||
               (isa<complex_type>(*ty) && std::holds_alternative<std::complex<double>>(val));
    };

    if (auto bt = dyn_cast<boolean_type>(ty); bt) {
        if (!std::holds_alternative<bool>(value())) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else if (auto st = dyn_cast<number_type>(ty); st) {
        if (!type_ok(value(), st)) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else if (auto ct = dyn_cast<coopmatrix_type>(ty); ct) {
        if (!type_ok(value(), ct->component_ty())) {
            throw compilation_error(loc(), status::ir_constant_mismatch);
        }
    } else {
        throw compilation_error(loc(), status::ir_expected_coopmatrix_number_or_boolean);
    }
}

auto constant_inst::is_zero() -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){0}; }, value());
}
auto constant_inst::is_identity() -> bool {
    return std::visit([](auto const &v) { return v == decltype(v){1}; }, value());
}

void cooperative_matrix_apply_inst::setup_and_check() {
    auto ty = result().ty();

    if (a().ty() != ty) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }

    auto at = get_coopmatrix_type(loc(), a());

    auto i32_ty = i32_type::get(at->context());
    body().loc(loc());
    body().kind(region_kind::spmd);
    body().set_num_params(3);
    body().set_param(0, i32_ty);
    body().set_param(1, i32_ty);
    body().set_param(2, at->component_ty());
}

void cooperative_matrix_extract_inst::setup_and_check() {
    auto ty = result().ty();

    auto matt = get_coopmatrix_type(loc(), mat());
    if (matt->component_ty() != ty) {
        throw compilation_error(loc(), {&mat()}, status::ir_number_mismatch);
    }
}

void cooperative_matrix_insert_inst::setup_and_check() {
    auto ty = result().ty();

    if (mat().ty() != ty) {
        throw compilation_error(loc(), {&mat()}, status::ir_operand_type_must_match_return_type);
    }

    auto valt = get_scalar_type(loc(), val());
    auto matt = get_coopmatrix_type(loc(), mat());
    if (matt->component_ty() != valt) {
        throw compilation_error(loc(), {&val(), &mat()}, status::ir_number_mismatch);
    }
}

void cooperative_matrix_load_inst::setup_and_check() {
    auto rt = dyn_cast<coopmatrix_type>(result().ty());
    if (!rt) {
        throw compilation_error(loc(), status::ir_expected_coopmatrix);
    }

    auto ot = get_memref_type(loc(), operand());
    if (ot->element_ty() != rt->component_ty()) {
        throw compilation_error(loc(), {&operand()}, status::ir_number_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), {&operand()}, status::ir_expected_memref_order_2);
    }

    check_index_ty(loc(), pos0());
    check_index_ty(loc(), pos1());
}

void cooperative_matrix_mul_add_inst::setup_and_check() {
    auto rt = dyn_cast<coopmatrix_type>(result().ty());
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
    if (!promotable(AB_ty, ct->component_ty())) {
        throw compilation_error(loc(), {&a(), &b(), &c()}, status::ir_forbidden_promotion);
    }
    if (!is_cast_allowed(ct->component_ty(), rt->component_ty())) {
        throw compilation_error(loc(), {&c()}, status::ir_forbidden_cast);
    }
}

auto cooperative_matrix_mul_add_inst::is_c_zero() -> bool {
    if (auto c_def = c().defining_inst(); c_def) {
        if (auto c_def_const = dyn_cast<const constant_inst>(c_def); c_def_const) {
            return std::visit(
                overloaded{[](bool) { return false; }, [](auto v) { return v == decltype(v){0}; }},
                c_def_const.value());
        }
    }
    return false;
}

void cooperative_matrix_prefetch_inst::setup_and_check() {
    auto ot = get_memref_type(loc(), operand());
    if (ot->dim() != 2) {
        throw compilation_error(loc(), {&operand()}, status::ir_expected_memref_order_2);
    }
    if (rows() <= 0 || cols() <= 0) {
        throw compilation_error(loc(), {}, status::ir_invalid_shape);
    }

    check_index_ty(loc(), pos0());
    check_index_ty(loc(), pos1());
}

void cooperative_matrix_reduce_inst::setup_and_check() {
    auto at = get_coopmatrix_type(loc(), a());
    auto rt = get_coopmatrix_type(loc(), result().ty());
    if (at->component_ty() != rt->component_ty()) {
        throw compilation_error(loc(), {&a()}, status::ir_number_mismatch);
    }
    if (at->use() != rt->use()) {
        throw compilation_error(loc(), {&a()}, status::ir_invalid_matrix_use);
    }
    const int m = mode() == reduce_mode::column ? 0 : 1;
    if (rt->shape(1 - m) != at->shape(1 - m) || rt->shape(m) != 1) {
        throw compilation_error(loc(), {&a()}, status::ir_invalid_shape);
    }
}
void cooperative_matrix_reduce_add_inst::setup_and_check() {
    cooperative_matrix_reduce_inst::setup_and_check();
}
void cooperative_matrix_reduce_max_inst::setup_and_check() {
    cooperative_matrix_reduce_inst::setup_and_check();
}
void cooperative_matrix_reduce_min_inst::setup_and_check() {
    cooperative_matrix_reduce_inst::setup_and_check();
}

void cooperative_matrix_scale_inst::setup_and_check() {
    auto ty = result().ty();

    if (b().ty() != ty) {
        throw compilation_error(loc(), {&b()}, status::ir_operand_type_must_match_return_type);
    }

    auto bt = get_coopmatrix_type(loc(), b());

    if (a().ty() != bt->component_ty()) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_number_mismatch);
    }
}

void cooperative_matrix_store_inst::setup_and_check() {
    auto vt = get_coopmatrix_type(loc(), val());
    auto ot = get_memref_type(loc(), operand());
    if (vt->component_ty() != ot->element_ty()) {
        throw compilation_error(loc(), {&val(), &operand()}, status::ir_number_mismatch);
    }
    if (ot->dim() != 2) {
        throw compilation_error(loc(), {&operand()}, status::ir_expected_memref_order_2);
    }

    check_index_ty(loc(), pos0());
    check_index_ty(loc(), pos1());
}

void expand_inst::setup_and_check() {
    for (auto &es : expand_shape()) {
        check_index_ty(loc(), es);
    }

    auto ty = result().ty();

    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    bool const range_ok = 0 <= expanded_mode() && expanded_mode() < ot->dim();
    if (!range_ok) {
        throw compilation_error(loc(), {&operand()}, status::ir_out_of_bounds);
    }

    if (static_expand_shape().size() < 2) {
        throw compilation_error(loc(), status::ir_expand_shape_order_too_small);
    }
    if (std::count(static_expand_shape().begin(), static_expand_shape().end(), dynamic) !=
        expand_shape().size()) {
        throw compilation_error(loc(), status::ir_expand_shape_mismatch);
    }

    for (std::int64_t i = 0; i < expanded_mode(); ++i) {
        check_memref_mode(rt, i, ot, i, loc());
    }
    auto stride = ot->stride(expanded_mode());
    for (std::size_t i = 0; i < static_expand_shape().size(); ++i) {
        const auto mode = expanded_mode() + i;
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
    for (std::int64_t i = expanded_mode() + 1; i < ot->dim(); ++i) {
        check_memref_mode(rt, i + static_expand_shape().size() - 1, ot, i, loc());
    }
}

void fuse_inst::setup_and_check() {
    auto ty = result().ty();
    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    bool const range_ok = 0 <= from() && from() < to() && to() < ot->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    for (std::int64_t i = 0; i < from(); ++i) {
        check_memref_mode(rt, i, ot, i, loc());
    }

    std::int64_t prod = 1;
    for (std::int64_t i = from(); i <= to(); ++i) {
        if (is_dynamic_value(ot->shape(i))) {
            prod = dynamic;
            break;
        }
        prod *= ot->shape(i);
    }
    if (rt->shape(from()) != prod) {
        auto extra_info = std::ostringstream{} << "Size of mode " << from()
                                               << " does not match shape product ("
                                               << rt->shape(from()) << "!=" << prod << ")";
        throw compilation_error(loc(), status::ir_invalid_shape, std::move(extra_info).str());
    }
    check_memref_stride(rt, from(), ot, from(), loc());

    for (std::int64_t i = to() + 1; i < ot->dim(); ++i) {
        check_memref_mode(rt, i - to() + from(), ot, i, loc());
    }
}

void if_inst::setup_and_check() {
    then().loc(loc());
    otherwise().loc(loc());
    if (!isa<boolean_type>(*condition().ty())) {
        throw compilation_error(loc(), {&condition()}, status::ir_expected_boolean);
    }

    for (auto &r : results()) {
        auto &ty = *r.ty();
        if (!isa<boolean_type>(ty) && !isa<number_type>(ty) && !isa<coopmatrix_type>(ty)) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_number_or_boolean);
        }
    }
}

auto if_inst::is_otherwise_empty() -> bool { return otherwise().insts().empty(); }

void lifetime_stop_inst::setup_and_check() {}

void load_inst::setup_and_check() {
    auto ty = result().ty();

    visit(overloaded{
              [&](group_type &g) {
                  if (g.ty() != ty) {
                      throw compilation_error(loc(), {&operand()},
                                              status::ir_operand_type_must_match_return_type);
                  }
                  if (static_cast<std::int64_t>(index_list().size()) != 1) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
              },
              [&](memref_type &m) {
                  if (m.element_ty() != ty) {
                      throw compilation_error(loc(), {&operand()},
                                              status::ir_operand_type_must_match_return_type);
                  }
                  if (m.dim() != static_cast<std::int64_t>(index_list().size())) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
              },
              [&](auto &) { throw compilation_error(loc(), status::ir_expected_memref_or_group); }},
          *operand().ty());
}

void parallel_inst::setup_and_check() {
    body().kind(region_kind::spmd);
    body().loc(loc());
}

void size_inst::setup_and_check() {
    if (!isa<index_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_index);
    }

    const bool range_ok =
        visit(overloaded{[&](group_type &) -> bool { return 0 <= mode() && mode() < 1; },
                         [&](memref_type &m) -> bool { return 0 <= mode() && mode() < m.dim(); },
                         [&](auto &) -> bool {
                             throw compilation_error(loc(), status::ir_expected_memref_or_group);
                         }},
              *operand().ty());
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }
}

void subgroup_broadcast_inst::setup_and_check() {
    auto ty = result().ty();
    if (!isa<number_type>(*ty)) {
        throw compilation_error(loc(), status::ir_expected_number);
    }

    if (a().ty() != ty) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }

    if (!isa<i32_type>(*idx().ty())) {
        throw compilation_error(loc(), {&idx()}, status::ir_expected_i32);
    }
}

void subview_inst::setup_and_check() {
    for (auto &val : offsets()) {
        check_index_ty(loc(), val);
    }
    for (auto &val : sizes()) {
        check_index_ty(loc(), val);
    }

    auto ty = result().ty();
    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    if (ot->dim() != static_cast<std::int64_t>(static_offsets().size()) ||
        ot->dim() != static_cast<std::int64_t>(static_sizes().size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }
    if (std::count(static_offsets().begin(), static_offsets().end(), dynamic) != offsets().size() ||
        std::count(static_sizes().begin(), static_sizes().end(), dynamic) != sizes().size()) {
        throw compilation_error(loc(), status::ir_subview_mismatch);
    }

    std::int64_t ri = 0;
    for (std::int64_t i = 0; i < ot->dim(); ++i) {
        auto offset = static_offsets()[i];
        auto size = static_sizes()[i];
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
}

void store_inst::setup_and_check() {
    for (auto &val : index_list()) {
        check_index_ty(loc(), val);
    }

    auto o = get_memref_type(loc(), operand());

    if (val().ty() != o->element_ty()) {
        throw compilation_error(loc(), {&val(), &operand()}, status::ir_number_mismatch);
    }

    if (o->dim() != static_cast<std::int64_t>(index_list().size())) {
        throw compilation_error(loc(), {&operand()}, status::ir_invalid_number_of_indices);
    }
}

void yield_inst::setup_and_check() {}

void arith_inst::setup_and_check() {}
void arith_inst::setup_and_check(support_flags support) {
    auto ty = result().ty();

    if (a().ty() != ty) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }
    if (b().ty() != ty) {
        throw compilation_error(loc(), {&b()}, status::ir_operand_type_must_match_return_type);
    }

    if (isa<boolean_type>(*ty)) {
        if (!(support & supports_bool)) {
            throw compilation_error(loc(), status::ir_boolean_unsupported);
        }
    } else {
        auto const check_scalar_ty = [&](tinytc_type_t ty) {
            if (!(support & supports_float) && isa<float_type>(*ty)) {
                throw compilation_error(loc(), status::ir_fp_unsupported);
            }
            if (!(support & supports_complex) && isa<complex_type>(*ty)) {
                throw compilation_error(loc(), status::ir_complex_unsupported);
            }
        };

        if (auto ct = dyn_cast<coopmatrix_type>(ty); ct) {
            check_scalar_ty(ct->component_ty());
        } else if (isa<number_type>(*ty)) {
            check_scalar_ty(ty);
        } else {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_or_number);
        }
    }
}
void add_inst::setup_and_check() {
    arith_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void sub_inst::setup_and_check() {
    arith_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void mul_inst::setup_and_check() {
    arith_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void div_inst::setup_and_check() {
    arith_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void rem_inst::setup_and_check() { arith_inst::setup_and_check(supports_int | supports_float); }
void max_inst::setup_and_check() { arith_inst::setup_and_check(supports_int | supports_float); }
void min_inst::setup_and_check() { arith_inst::setup_and_check(supports_int | supports_float); }
void shl_inst::setup_and_check() { arith_inst::setup_and_check(supports_int); }
void shr_inst::setup_and_check() { arith_inst::setup_and_check(supports_int); }
void and_inst::setup_and_check() { arith_inst::setup_and_check(supports_bool | supports_int); }
void or_inst::setup_and_check() { arith_inst::setup_and_check(supports_bool | supports_int); }
void xor_inst::setup_and_check() { arith_inst::setup_and_check(supports_bool | supports_int); }

void arith_unary_inst::setup_and_check() {}
void arith_unary_inst::setup_and_check(support_flags support, bool component_type_match) {
    auto ty = result().ty();

    if (isa<boolean_type>(*ty)) {
        if (!(support & supports_bool)) {
            throw compilation_error(loc(), status::ir_boolean_unsupported);
        }
    } else {
        auto const check_scalar_ty = [&](tinytc_type_t a_ty, tinytc_type_t r_ty) {
            if (component_type_match) {
                if (r_ty != component_type(a_ty)) {
                    throw compilation_error(loc(), {&a()},
                                            status::ir_operand_type_must_match_return_type);
                }
            } else {
                if (a_ty != r_ty) {
                    throw compilation_error(loc(), {&a()},
                                            status::ir_operand_type_must_match_return_type);
                }
            }
            if (!(support & supports_int) && isa<integer_type>(*a_ty)) {
                throw compilation_error(loc(), {&a()}, status::ir_int_unsupported);
            }
            if (!(support & supports_float) && isa<float_type>(*a_ty)) {
                throw compilation_error(loc(), {&a()}, status::ir_fp_unsupported);
            }
            if (!(support & supports_complex) && isa<complex_type>(*a_ty)) {
                throw compilation_error(loc(), {&a()}, status::ir_complex_unsupported);
            }
        };

        auto ct = dyn_cast<coopmatrix_type>(a().ty());
        auto rt = dyn_cast<coopmatrix_type>(ty);
        if (ct && rt) {
            check_scalar_ty(ct->component_ty(), rt->component_ty());
        } else if (isa<number_type>(*a().ty()) && isa<number_type>(*ty)) {
            check_scalar_ty(a().ty(), ty);
        } else {
            throw compilation_error(loc(), {&a()}, status::ir_expected_coopmatrix_or_number);
        }
    }
}
void abs_inst::setup_and_check() {
    arith_unary_inst::setup_and_check(supports_int | supports_float | supports_complex, true);
}
void neg_inst::setup_and_check() {
    arith_unary_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void not_inst::setup_and_check() {
    arith_unary_inst::setup_and_check(supports_bool | supports_int);
}
void conj_inst::setup_and_check() { arith_unary_inst::setup_and_check(supports_complex); }
void im_inst::setup_and_check() { arith_unary_inst::setup_and_check(supports_complex, true); }
void re_inst::setup_and_check() { arith_unary_inst::setup_and_check(supports_complex, true); }

void blas_a2_inst::setup_and_check() {
    auto At = get_memref_type(loc(), A());
    auto Bt = get_memref_type(loc(), B());

    if (!promotable(alpha().ty(), At->element_ty())) {
        throw compilation_error(loc(), {&alpha(), &A()}, status::ir_forbidden_promotion);
    }
    if (!promotable(At->element_ty(), Bt->element_ty())) {
        throw compilation_error(loc(), {&A(), &B()}, status::ir_forbidden_promotion);
    }
    if (!promotable(beta().ty(), Bt->element_ty())) {
        throw compilation_error(loc(), {&beta(), &B()}, status::ir_forbidden_promotion);
    }
}

void axpby_inst::setup_and_check() {
    blas_a2_inst::setup_and_check();

    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    if (b->dim() < 0 || b->dim() > 2) {
        throw compilation_error(loc(), {&B()}, status::ir_expected_memref_order_0_1_or_2);
    }

    bool shape_equal = false;
    if (tA() == transpose::T && a->dim() == 2 && b->dim() == 2) {
        shape_equal = a->shape()[1] == b->shape()[0] && a->shape()[0] == b->shape()[1];
    } else {
        shape_equal = a->shape() == b->shape();
    }

    if (!shape_equal) {
        throw compilation_error(loc(), {&A(), &B()}, status::ir_incompatible_shapes);
    }
}

void cumsum_inst::setup_and_check() {
    blas_a2_inst::setup_and_check();

    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());

    if (a->dim() < 1) {
        throw compilation_error(loc(), {&A()}, status::ir_expected_non_scalar_memref);
    }
    if (mode() >= a->dim()) {
        throw compilation_error(loc(), {&A()}, status::ir_out_of_bounds);
    }

    bool shape_equal = a->dim() == b->dim();
    if (shape_equal) {
        for (std::int64_t i = 0; i < a->dim(); ++i) {
            shape_equal = shape_equal && a->shape(i) == b->shape(i);
        }
    }

    if (!shape_equal) {
        throw compilation_error(loc(), {&A(), &B()}, status::ir_incompatible_shapes);
    }
}

void sum_inst::setup_and_check() {
    blas_a2_inst::setup_and_check();

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
        if (a->shape(tA() == transpose::T ? 1 : 0) != b->shape(0)) {
            throw compilation_error(loc(), {&A(), &B()}, status::ir_incompatible_shapes);
        }
    }
}

void blas_a3_inst::setup_and_check() {
    auto At = get_memref_type(loc(), A());
    auto Bt = get_memref_type(loc(), B());
    auto Ct = get_memref_type(loc(), C());

    const auto AB_ty = promote(At->element_ty(), Bt->element_ty());
    if (!AB_ty) {
        throw compilation_error(loc(), {&A(), &B()}, status::ir_forbidden_promotion);
    }
    if (!promotable(alpha().ty(), AB_ty)) {
        throw compilation_error(loc(), {&alpha(), &A(), &B()}, status::ir_forbidden_promotion);
    }
    if (!promotable(AB_ty, Ct->element_ty())) {
        throw compilation_error(loc(), {&A(), &B(), &C()}, status::ir_forbidden_promotion);
    }
    if (!promotable(beta().ty(), Ct->element_ty())) {
        throw compilation_error(loc(), {&beta(), &C()}, status::ir_forbidden_promotion);
    }
}

void gemm_inst::setup_and_check() {
    blas_a3_inst::setup_and_check();

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

    auto ak = tA() == transpose::T ? 0 : 1;
    auto bk = tB() == transpose::T ? 1 : 0;
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

void gemv_inst::setup_and_check() {
    blas_a3_inst::setup_and_check();

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

    auto ak = tA() == transpose::T ? 0 : 1;
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

void ger_inst::setup_and_check() {
    blas_a3_inst::setup_and_check();

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

void hadamard_inst::setup_and_check() {
    blas_a3_inst::setup_and_check();

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

void builtin_inst::setup_and_check() {}

void group_id_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<index_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_index);
    }
}
void num_groups_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<index_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_index);
    }
}
void num_subgroups_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<i32_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_i32);
    }
}
void subgroup_size_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<i32_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_i32);
    }
}
void subgroup_id_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<i32_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_i32);
    }
}
void subgroup_linear_id_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<i32_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_i32);
    }
}
void subgroup_local_id_inst::setup_and_check() {
    builtin_inst::setup_and_check();
    if (!isa<i32_type>(*result().ty())) {
        throw compilation_error(loc(), status::ir_expected_i32);
    }
}

void compare_inst::setup_and_check() {}
void compare_inst::setup_and_check(support_flags support) {
    auto ty = result().ty();

    if (!isa<boolean_type>(*ty)) {
        throw compilation_error(loc(), status::ir_expected_boolean);
    }

    if (!isa<number_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_expected_number);
    }
    if (!(support & supports_complex) && isa<complex_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_complex_unsupported);
    }

    if (a().ty() != b().ty()) {
        throw compilation_error(loc(), {&a(), &b()}, status::ir_number_mismatch);
    }
}
void equal_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void not_equal_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void greater_than_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float);
}
void greater_than_equal_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float);
}
void less_than_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float);
}
void less_than_equal_inst::setup_and_check() {
    compare_inst::setup_and_check(supports_int | supports_float);
}

void loop_inst::setup_and_check() {}

void for_inst::setup_and_check() {
    loop_inst::setup_and_check();

    if (!isa<integer_type>(*from().ty())) {
        throw compilation_error(loc(), {&from()}, status::ir_expected_int);
    }
    if (from().ty() != to().ty()) {
        throw compilation_error(loc(), {&from(), &to()}, status::ir_number_mismatch);
    }
    if (has_step()) {
        if (from().ty() != step().ty()) {
            throw compilation_error(loc(), {&from(), &step()}, status::ir_number_mismatch);
        }
    }

    auto res = results();
    body().set_num_params(1 + res.size());
    body().set_param(0, from().ty());

    auto init = iter_init();
    if (init.size() != res.size()) {
        throw compilation_error(loc(), status::ir_init_return_type_mismatch);
    }
    for (std::int64_t i = 0; i < res.size(); ++i) {
        auto ty = res[i].ty();
        if (init[i].ty() != ty) {
            throw compilation_error(loc(), {&init[i]}, status::ir_init_return_type_mismatch);
        }
        if (!isa<boolean_type>(*ty) && !isa<number_type>(*ty) && !isa<coopmatrix_type>(*ty)) {
            throw compilation_error(loc(), status::ir_expected_coopmatrix_number_or_boolean);
        }
        body().set_param(1 + i, ty);
    }
}

void foreach_inst::setup_and_check() {
    loop_inst::setup_and_check();

    auto from_ = from();
    auto to_ = to();
    if (from_.size() == 0 || from_.size() != to_.size()) {
        throw compilation_error(loc(), status::ir_from_to_mismatch);
    }

    auto num_lv = from_.size();
    body().kind(region_kind::spmd);
    body().set_num_params(num_lv);
    for (std::int64_t i = 0; i < num_lv; ++i) {
        if (!isa<integer_type>(*from_[i].ty())) {
            throw compilation_error(loc(), {&from_[i]}, status::ir_expected_int);
        }
        if (from_[i].ty() != to_[i].ty()) {
            throw compilation_error(loc(), {&from_[i], &to_[i]}, status::ir_number_mismatch);
        }
        body().set_param(i, from_[i].ty());
    }
}

void math_unary_inst::setup_and_check() {}
void math_unary_inst::setup_and_check(support_flags support) {
    if (!isa<number_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_expected_number);
    }

    if (!(support & supports_int) && isa<integer_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_int_unsupported);
    } else if (!(support & supports_float) && isa<float_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_fp_unsupported);
    } else if (!(support & supports_complex) && isa<complex_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_complex_unsupported);
    }

    if (a().ty() != result().ty()) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }
}
void cos_inst::setup_and_check() { math_unary_inst::setup_and_check(supports_float); }
void sin_inst::setup_and_check() { math_unary_inst::setup_and_check(supports_float); }
void exp_inst::setup_and_check() {
    math_unary_inst::setup_and_check(supports_float | supports_complex);
}
void exp2_inst::setup_and_check() {
    math_unary_inst::setup_and_check(supports_float | supports_complex);
}
void native_cos_inst::setup_and_check() { math_unary_inst::setup_and_check(supports_float); }
void native_sin_inst::setup_and_check() { math_unary_inst::setup_and_check(supports_float); }
void native_exp_inst::setup_and_check() {
    math_unary_inst::setup_and_check(supports_float | supports_complex);
}
void native_exp2_inst::setup_and_check() {
    math_unary_inst::setup_and_check(supports_float | supports_complex);
}

void subgroup_operation_inst::setup_and_check() {}
void subgroup_operation_inst::setup_and_check(support_flags support) {
    if (isa<number_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_expected_number);
    }
    if (!(support & supports_complex) && isa<complex_type>(*a().ty())) {
        throw compilation_error(loc(), {&a()}, status::ir_complex_unsupported);
    }

    if (a().ty() != result().ty()) {
        throw compilation_error(loc(), {&a()}, status::ir_operand_type_must_match_return_type);
    }
}
void subgroup_exclusive_scan_add_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void subgroup_exclusive_scan_max_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}
void subgroup_exclusive_scan_min_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}
void subgroup_inclusive_scan_add_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void subgroup_inclusive_scan_max_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}
void subgroup_inclusive_scan_min_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}
void subgroup_reduce_add_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float | supports_complex);
}
void subgroup_reduce_max_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}
void subgroup_reduce_min_inst::setup_and_check() {
    subgroup_operation_inst::setup_and_check(supports_int | supports_float);
}

} // namespace tinytc
