// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst_node.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <clir/builtin_type.hpp>

#include <cstddef>
#include <memory>
#include <sstream>

namespace tinytc {

scalar_data_type *get_scalar_type(location const &loc, value const &v) {
    auto m = dyn_cast<scalar_data_type>(v->ty().get());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    return m;
}

memref_data_type *get_memref_type(location const &loc, value const &v) {
    auto m = dyn_cast<memref_data_type>(v->ty().get());
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

    region_node &body = *child_region(0);
    if (body.empty() || !isa<yield_inst>(**(body.end() - 1))) {
        body.insert(body.end(), make_yield({}, lc));
    }
}

alloca_inst::alloca_inst(data_type ty, location const &lc)
    : standard_inst{IK::alloca}, stack_ptr_{-1} {
    loc(lc);

    result(0) = make_value(std::move(ty));
    auto memref = dyn_cast<memref_data_type>(result(0)->ty().get());
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
    result(0) = make_value(at->ty());
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
    result(0) = make_value(at->ty());
}

cast_inst::cast_inst(value a, scalar_type to_ty, location const &lc) : standard_inst{IK::cast} {
    op(op_a) = std::move(a);
    loc(lc);

    result(0) = make_value(std::move(to_ty));
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

    result(0) = make_value(scalar_type::i1);
}

expand_inst::expand_inst(value op0, std::int64_t mode, std::vector<value> const &expand_shape0,
                         location const &lc)
    : standard_inst{IK::expand, static_cast<std::int64_t>(1 + expand_shape0.size())}, mode_(mode) {
    op(0) = std::move(op0);
    for (std::size_t i = 0; i < expand_shape0.size(); ++i) {
        op(1 + i) = expand_shape0[i];
    }
    loc(lc);

    auto m = get_memref_type(loc(), operand());
    bool const range_ok = 0 <= mode_ && mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    if (expand_shape().size() < 2) {
        throw compilation_error(loc(), status::ir_expand_shape_order_too_small);
    }

    auto known_expand_shape = std::vector<std::int64_t>();
    known_expand_shape.reserve(expand_shape().size());
    std::size_t dyn_count = 0, non_imm_count = 0;
    for (auto &s : expand_shape()) {
        visit(overloaded{[&](int_imm &i) {
                             if (is_dynamic_value(i.value())) {
                                 known_expand_shape.push_back(dynamic);
                                 ++dyn_count;
                                 return;
                             }
                             if (i.value() < 0) {
                                 throw compilation_error(loc(), status::ir_invalid_shape);
                             }
                             known_expand_shape.push_back(i.value());
                         },
                         [&](auto &) {
                             known_expand_shape.push_back(dynamic);
                             ++non_imm_count;
                         }},
              *s);
    }

    if (dyn_count > 1) {
        throw compilation_error(loc(), status::ir_multiple_dynamic_modes);
    }

    auto size = m->shape(mode_);
    if (!is_dynamic_value(size) && non_imm_count == 0) {
        std::int64_t prod = 1;
        std::int64_t dyn_mode = -1;
        for (std::size_t i = 0; i < known_expand_shape.size(); ++i) {
            auto const s = known_expand_shape[i];
            if (is_dynamic_value(s)) {
                dyn_mode = i;
            } else {
                prod *= s;
            }
        }
        if (dyn_mode >= 0) {
            std::int64_t const s = size / prod;
            known_expand_shape[dyn_mode] = s;
            expand_shape()[dyn_mode] = make_imm(s);
            prod *= s;
        }
        if (prod != size) {
            throw compilation_error(loc(), status::ir_expand_shape_mismatch);
        }
    }

    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim() + known_expand_shape.size() - 1);
    stride.reserve(m->dim() + known_expand_shape.size() - 1);
    for (std::int64_t i = 0; i < mode_; ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }

    stride.push_back(m->stride(mode_));
    shape.push_back(known_expand_shape[0]);
    for (std::size_t j = 1; j < known_expand_shape.size(); ++j) {
        stride.push_back(is_dynamic_value(stride.back()) || is_dynamic_value(shape.back())
                             ? dynamic
                             : stride.back() * shape.back());
        shape.push_back(known_expand_shape[j]);
    }
    for (std::int64_t i = mode_ + 1; i < m->dim(); ++i) {
        shape.push_back(m->shape(i));
        stride.push_back(m->stride(i));
    }
    auto r = std::make_unique<memref_data_type>(m->element_ty(), shape, stride);

    r->addrspace(m->addrspace());
    result(0) = make_value(data_type(r.release()));
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
    auto r = std::make_unique<memref_data_type>(m->element_ty(), shape, stride);

    r->addrspace(m->addrspace());
    result(0) = make_value(data_type(r.release()));
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
                  result(0) = make_value(m.element_ty());
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
                 std::vector<scalar_type> const &return_types, location const &lc)
    : standard_inst{IK::if_, 1, static_cast<int64_t>(return_types.size()), otherwise0 ? 2 : 1} {
    op(0) = std::move(condition);
    child_region(child_region_then) = std::move(then0);
    child_region(child_region_otherwise) = std::move(otherwise0);
    loc(lc);
    for (std::size_t i = 0; i < return_types.size(); ++i) {
        result(i) = make_value(return_types[i]);
    }

    for (std::int64_t i = 0; i < num_child_regions(); ++i) {
        region_node &body = *child_region(i);
        if (body.empty() || !isa<yield_inst>(**(body.end() - 1))) {
            body.insert(body.end(), make_yield({}, lc));
        }
    }
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

    result(0) = make_value(scalar_type::index);
}

subview_inst::subview_inst(value op0, std::vector<value> const &offset_list0,
                           std::vector<value> const &size_list0, location const &lc)
    : standard_inst{IK::subview,
                    static_cast<std::int64_t>(1 + offset_list0.size() + size_list0.size())} {
    op(0) = std::move(op0);
    {
        std::size_t i = 1;
        for (auto const &val : offset_list0) {
            op(i++) = val;
        }
        for (auto const &val : size_list0) {
            op(i++) = val ? val : make_index(0);
        }
    }
    loc(lc);

    auto m = get_memref_type(loc(), operand());
    if (m->dim() != static_cast<std::int64_t>(offset_list0.size()) ||
        m->dim() != static_cast<std::int64_t>(size_list0.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }

    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim());
    stride.reserve(m->dim());
    for (std::int64_t i = 0; i < m->dim(); ++i) {
        auto &offset = offset_list()[i];
        auto &size = size_list()[i];
        visit(overloaded{[&](int_imm &i) {
                             if (i.value() < 0) {
                                 throw compilation_error(loc(), status::ir_invalid_slice);
                             }
                         },
                         [](auto &) {}},
              *offset);
        visit(overloaded{[&](int_imm &i) {
                             if (i.value() < 0 && !is_dynamic_value(i.value())) {
                                 throw compilation_error(loc(), status::ir_invalid_slice);
                             }
                         },
                         [](auto &) {}},
              *size);
        auto size_value = visit(overloaded{[&](int_imm &offset, int_imm &size) -> std::int64_t {
                                               if (is_dynamic_value(size.value())) {
                                                   return is_dynamic_value(m->shape(i))
                                                              ? dynamic
                                                              : m->shape(i) - offset.value();
                                               }
                                               return size.value();
                                           },
                                           [&](val &, int_imm &size) -> std::int64_t {
                                               if (is_dynamic_value(size.value())) {
                                                   return dynamic;
                                               }
                                               return size.value();
                                           },
                                           [](auto &, auto &) -> std::int64_t { return dynamic; }},
                                *offset, *size);
        if (size_value > 0 || is_dynamic_value(size_value)) {
            shape.push_back(size_value);
            stride.push_back(m->stride(i));
        }
    }
    auto r = std::make_unique<memref_data_type>(m->element_ty(), shape, stride);

    r->addrspace(m->addrspace());
    result(0) = make_value(data_type(r.release()));
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
