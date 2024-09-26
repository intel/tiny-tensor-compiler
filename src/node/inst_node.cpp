// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst_node.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <clir/builtin_type.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>

namespace tinytc {

scalar_data_type *get_scalar_type(location const &loc, value const &v) {
    auto m = dyn_cast<scalar_data_type>(v->ty());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    return m;
}

memref_data_type *get_memref_type(location const &loc, value const &v) {
    auto m = dyn_cast<memref_data_type>(v->ty());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_memref);
    }
    return m;
}

blas_a2_inst::blas_a2_inst(IK tid, value alpha, value A, value beta, value B, bool atomic)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha) = std::move(alpha);
    op(op_A) = std::move(A);
    op(op_beta) = std::move(beta);
    op(op_B) = std::move(B);
}

blas_a3_inst::blas_a3_inst(IK tid, value alpha, value A, value B, value beta, value C, bool atomic)
    : standard_inst{tid}, atomic_(atomic) {
    op(op_alpha) = std::move(alpha);
    op(op_A) = std::move(A);
    op(op_B) = std::move(B);
    op(op_beta) = std::move(beta);
    op(op_C) = std::move(C);
}

loop_inst::loop_inst(IK tid, value loop_var0, value from0, value to0, value step0, region body0,
                     location const &lc)
    : standard_inst{tid, step0 ? 4 : 3} {
    op(op_loop_var) = std::move(loop_var0);
    op(op_from) = std::move(from0);
    op(op_to) = std::move(to0);
    op(op_step) = std::move(step0);
    child_region(0) = std::move(body0);

    loc(lc);
    auto lvt = get_scalar_type(loc(), loop_var());
    auto fromt = get_scalar_type(loc(), from());
    auto tot = get_scalar_type(loc(), to());
    bool step_ok = true;
    if (step()) {
        auto stept = get_scalar_type(loc(), step());
        step_ok = lvt->ty() == stept->ty();
    }

    if (lvt->ty() != fromt->ty() || lvt->ty() != tot->ty() || !step_ok) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
}

alloca_inst::alloca_inst(tinytc_data_type_t ty, location const &lc)
    : standard_inst{IK::alloca}, stack_ptr_{-1} {
    loc(lc);

    result(0) = make_value(std::move(ty));
    auto memref = dyn_cast<memref_data_type>(result(0)->ty());
    if (memref == nullptr) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    if (memref->addrspace() != address_space::local) {
        throw compilation_error(loc(), status::ir_expected_local_address_space);
    }
}

axpby_inst::axpby_inst(transpose tA, value alpha0, value A0, value beta0, value B0, bool atomic,
                       location const &lc)
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

arith_inst::arith_inst(arithmetic operation, value a0, value b0, location const &lc)
    : standard_inst{IK::arith}, operation_(operation) {
    op(op_a) = std::move(a0);
    op(op_b) = std::move(b0);
    loc(lc);

    auto at = get_scalar_type(loc(), a());
    auto bt = get_scalar_type(loc(), b());

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
    bool inst_supports_fp = false;
    switch (operation) {
    case arithmetic::add:
    case arithmetic::sub:
    case arithmetic::mul:
    case arithmetic::div:
    case arithmetic::rem:
        inst_supports_fp = true;
        break;
    case arithmetic::shl:
    case arithmetic::shr:
    case arithmetic::and_:
    case arithmetic::or_:
    case arithmetic::xor_:
        inst_supports_fp = false;
        break;
    }
    if (!inst_supports_fp && is_floating_type(at->ty())) {
        throw compilation_error(loc(), status::ir_fp_unsupported);
    }
    result(0) = make_value(at);
}

arith_unary_inst::arith_unary_inst(arithmetic_unary operation, value a0, location const &lc)
    : standard_inst{IK::arith_unary}, operation_(operation) {
    op(op_a) = std::move(a0);
    loc(lc);

    auto at = get_scalar_type(loc(), a());
    bool inst_supports_fp = false;
    switch (operation) {
    case arithmetic_unary::neg:
        inst_supports_fp = true;
        break;
    case arithmetic_unary::not_:
        inst_supports_fp = false;
        break;
    }
    if (!inst_supports_fp && is_floating_type(at->ty())) {
        throw compilation_error(loc(), status::ir_fp_unsupported);
    }
    result(0) = make_value(at);
}

cast_inst::cast_inst(value a, scalar_type to_ty, location const &lc) : standard_inst{IK::cast} {
    op(op_a) = std::move(a);
    loc(lc);

    result(0) = make_value(scalar_data_type::get(op(op_a)->context(), std::move(to_ty)));
}

compare_inst::compare_inst(cmp_condition cond, value a0, value b0, location const &lc)
    : standard_inst{IK::compare}, cond_(cond) {
    op(op_a) = std::move(a0);
    op(op_b) = std::move(b0);
    loc(lc);

    auto at = get_scalar_type(loc(), a());
    auto bt = get_scalar_type(loc(), b());

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }

    result(0) = make_value(scalar_data_type::get(at->context(), scalar_type::i1));
}

constant_inst::constant_inst(std::variant<std::int64_t, double> const &value, tinytc_data_type_t ty,
                             location const &lc)
    : standard_inst{IK::constant}, value_(value) {
    loc(lc);

    if (auto st = dyn_cast<scalar_data_type>(ty); st) {
        if ((is_floating_type(st->ty()) && std::holds_alternative<std::int64_t>(value_)) ||
            (!is_floating_type(st->ty()) && std::holds_alternative<double>(value_))) {
            throw compilation_error(loc(), status::ir_scalar_mismatch);
        }
    } else {
        throw compilation_error(loc(), status::ir_expected_scalar);
    }

    result(0) = make_value(ty);
}

expand_inst::expand_inst(value op0, std::int64_t expanded_mode,
                         std::vector<std::int64_t> static_expand_shape0,
                         std::vector<value> const &expand_shape0, location const &lc)
    : standard_inst{IK::expand, static_cast<std::int64_t>(1 + expand_shape0.size())},
      expanded_mode_(expanded_mode), static_expand_shape_(std::move(static_expand_shape0)) {
    op(0) = std::move(op0);
    for (std::size_t i = 0; i < expand_shape0.size(); ++i) {
        op(1 + i) = expand_shape0[i];
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

    result(0) = make_value(
        memref_data_type::get(m->context(), m->element_ty(), shape, stride, m->addrspace()));
}

fuse_inst::fuse_inst(value op0, std::int64_t from, std::int64_t to, location const &lc)
    : standard_inst{IK::fuse}, from_(from), to_(to) {
    op(0) = std::move(op0);
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

    result(0) = make_value(
        memref_data_type::get(m->context(), m->element_ty(), shape, stride, m->addrspace()));
}

load_inst::load_inst(value op0, std::vector<value> const &index_list0, location const &lc)
    : standard_inst{IK::load, static_cast<std::int64_t>(1 + index_list0.size())} {
    op(0) = std::move(op0);
    for (std::size_t i = 0; i < index_list0.size(); ++i) {
        op(1 + i) = index_list0[i];
    }
    loc(lc);

    visit(overloaded{
              [&](group_data_type &g) {
                  if (static_cast<std::int64_t>(index_list().size()) != 1) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result(0) = make_value(g.ty());
              },
              [&](memref_data_type &m) {
                  if (m.dim() != static_cast<std::int64_t>(index_list().size())) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result(0) = make_value(scalar_data_type::get(m.context(), m.element_ty()));
              },
              [&](auto &) { throw compilation_error(loc(), status::ir_expected_memref_or_group); }},
          *operand()->ty());
}

gemm_inst::gemm_inst(transpose tA, transpose tB, value alpha0, value A0, value B0, value beta0,
                     value C0, bool atomic, location const &lc)
    : blas_a3_inst(IK::gemm_blas_a3, std::move(alpha0), std::move(A0), std::move(B0),
                   std::move(beta0), std::move(C0), atomic),
      tA_(tA), tB_(tB) {
    loc(lc);
    auto a = get_memref_type(loc(), A());
    auto b = get_memref_type(loc(), B());
    auto c = get_memref_type(loc(), C());

    if (a->dim() != 2 || b->dim() != 2 || c->dim() != 2) {
        throw compilation_error(loc(), status::ir_expected_vector_or_matrix,
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

gemv_inst::gemv_inst(transpose tA, value alpha0, value A0, value B0, value beta0, value C0,
                     bool atomic, location const &lc)
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

ger_inst::ger_inst(value alpha0, value A0, value B0, value beta0, value C0, bool atomic,
                   location const &lc)
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

foreach_inst::foreach_inst(value loop_var, value from, value to, region body, location const &loc)
    : loop_inst{IK::foreach_loop,
                std::move(loop_var),
                std::move(from),
                std::move(to),
                {},
                std::move(body),
                loc} {
    child_region(0)->kind(region_kind::spmd);
}

hadamard_inst::hadamard_inst(value alpha0, value A0, value B0, value beta0, value C0, bool atomic,
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

if_inst::if_inst(value condition, region then0, region otherwise0,
                 std::vector<tinytc_data_type_t> const &return_types, location const &lc)
    : standard_inst{IK::if_, 1, static_cast<int64_t>(return_types.size()), otherwise0 ? 2 : 1} {
    op(0) = std::move(condition);
    child_region(child_region_then) = std::move(then0);
    child_region(child_region_otherwise) = std::move(otherwise0);
    loc(lc);
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        result(i) = make_value(return_types[i]);
    }
}

parallel_inst::parallel_inst(region body, location const &lc) : standard_inst{IK::parallel} {
    child_region(0) = std::move(body);
    loc(lc);

    child_region(0)->kind(region_kind::spmd);
}

size_inst::size_inst(value op0, std::int64_t mode, location const &lc)
    : standard_inst{IK::size}, mode_(mode) {
    op(0) = std::move(op0);
    loc(lc);
    auto m = get_memref_type(loc(), operand());
    bool const range_ok = 0 <= mode_ && mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    result(0) = make_value(scalar_data_type::get(op(0)->context(), scalar_type::index));
}

subview_inst::subview_inst(value op0, std::vector<std::int64_t> static_offsets0,
                           std::vector<std::int64_t> static_sizes0,
                           std::vector<value> const &offsets0, std::vector<value> const &sizes0,
                           location const &lc)
    : standard_inst{IK::subview, static_cast<std::int64_t>(1 + offsets0.size() + sizes0.size())},
      static_offsets_(std::move(static_offsets0)), static_sizes_(std::move(static_sizes0)) {
    op(0) = std::move(op0);
    {
        std::size_t i = 1;
        for (auto const &val : offsets0) {
            op(i++) = val;
        }
        num_dyn_offsets_ = i - 1;
        for (auto const &val : sizes0) {
            op(i++) = val;
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

    result(0) = make_value(
        memref_data_type::get(m->context(), m->element_ty(), shape, stride, m->addrspace()));
}

store_inst::store_inst(value val0, value op0, std::vector<value> const &index_list0,
                       location const &lc)
    : standard_inst{IK::store, static_cast<std::int64_t>(2 + index_list0.size())} {
    op(op_val) = std::move(val0);
    op(op_operand) = std::move(op0);
    {
        std::size_t i = op_operand;
        for (auto const &val : index_list0) {
            op(++i) = val;
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

sum_inst::sum_inst(transpose tA, value alpha0, value A0, value beta0, value B0, bool atomic,
                   location const &lc)
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

} // namespace tinytc
