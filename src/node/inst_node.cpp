// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst_node.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/types.hpp"
#include "util.hpp"

#include <clir/builtin_type.hpp>
#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>

namespace tinytc {

scalar_data_type *get_scalar_type(location const &loc, value &v) {
    auto m = dynamic_cast<scalar_data_type *>(v->ty().get());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    return m;
}

memref_data_type *get_memref_type(location const &loc, value &v) {
    auto m = dynamic_cast<memref_data_type *>(v->ty().get());
    if (m == nullptr) {
        throw compilation_error(loc, status::ir_expected_memref);
    }
    return m;
}

blas_a2_inst::blas_a2_inst(value alpha, value A, value beta, value B, bool atomic)
    : alpha_(std::move(alpha)), A_(std::move(A)), beta_(std::move(beta)), B_(std::move(B)),
      atomic_(atomic) {}

blas_a3_inst::blas_a3_inst(value alpha, value A, value B, value beta, value C, bool atomic)
    : alpha_(std::move(alpha)), A_(std::move(A)), B_(std::move(B)), beta_(std::move(beta)),
      C_(std::move(C)), atomic_(atomic) {}

loop_inst::loop_inst(value loop_var, value from, value to, region body, location const &lc)
    : loop_inst(std::move(loop_var), std::move(from), std::move(to), nullptr, std::move(body), lc) {
}

loop_inst::loop_inst(value loop_var, value from, value to, value step, region body,
                     location const &lc)
    : loop_var_(std::move(loop_var)), from_(from), to_(to), step_(std::move(step)),
      body_(std::move(body)) {
    loc(lc);
    auto lvt = get_scalar_type(loc(), loop_var_);
    auto fromt = get_scalar_type(loc(), from_);
    auto tot = get_scalar_type(loc(), to_);
    bool step_ok = true;
    if (step_) {
        auto stept = get_scalar_type(loc(), step_);
        step_ok = lvt->ty() == stept->ty();
    }

    if (lvt->ty() != fromt->ty() || lvt->ty() != tot->ty() || !step_ok) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
}

alloca_inst::alloca_inst(data_type ty, location const &lc)
    : result_(std::move(ty)), stack_ptr_(-1) {
    loc(lc);
    auto memref = dynamic_cast<memref_data_type *>(result_->ty().get());
    if (memref == nullptr) {
        throw compilation_error(loc(), status::ir_expected_memref);
    }
    memref->addrspace(clir::address_space::local_t);
}

axpby_inst::axpby_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic,
                       location const &lc)
    : super(std::move(alpha), std::move(A), std::move(beta), std::move(B), atomic), tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);

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

binary_op_inst::binary_op_inst(binary_op op, value a, value b, location const &lc)
    : op_(op), a_(std::move(a)), b_(std::move(b)) {
    loc(lc);

    auto at = get_scalar_type(loc(), a_);
    auto bt = get_scalar_type(loc(), b_);

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
    result_ = value(at->ty());
}

cast_inst::cast_inst(value a, scalar_type to_ty, location const &lc) : a_(std::move(a)) {
    loc(lc);
    result_ = value(to_ty);
}

compare_inst::compare_inst(cmp_condition cond, value a, value b, location const &lc)
    : cond_(cond), a_(std::move(a)), b_(std::move(b)) {
    loc(lc);

    auto at = get_scalar_type(loc(), a_);
    auto bt = get_scalar_type(loc(), b_);

    if (at->ty() != bt->ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }
    result_ = value(scalar_type::bool_);
}

gemm_inst::gemm_inst(transpose tA, transpose tB, value alpha, value A, value B, value beta, value C,
                     bool atomic, location const &lc)
    : super(std::move(alpha), std::move(A), std::move(B), std::move(beta), std::move(C), atomic),
      tA_(tA), tB_(tB) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);
    auto c = get_memref_type(loc(), C_);

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

gemv_inst::gemv_inst(transpose tA, value alpha, value A, value B, value beta, value C, bool atomic,
                     location const &lc)
    : super(std::move(alpha), std::move(A), std::move(B), std::move(beta), std::move(C), atomic),
      tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);
    auto c = get_memref_type(loc(), C_);

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

ger_inst::ger_inst(value alpha, value A, value B, value beta, value C, bool atomic,
                   location const &lc)
    : super(std::move(alpha), std::move(A), std::move(B), std::move(beta), std::move(C), atomic) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);
    auto c = get_memref_type(loc(), C_);

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

hadamard_inst::hadamard_inst(value alpha, value A, value B, value beta, value C, bool atomic,
                             location const &lc)
    : super(std::move(alpha), std::move(A), std::move(B), std::move(beta), std::move(C), atomic) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);
    auto c = get_memref_type(loc(), C_);

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

expand_inst::expand_inst(value op, std::int64_t mode, std::vector<value> expand_shape,
                         location const &lc)
    : op_(std::move(op)), mode_(mode), expand_shape_(std::move(expand_shape)) {
    loc(lc);

    auto m = get_memref_type(loc(), op_);
    bool const range_ok = 0 <= mode_ && mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    if (expand_shape_.size() < 2) {
        throw compilation_error(loc(), status::ir_expand_shape_order_too_small);
    }

    auto known_expand_shape = std::vector<std::int64_t>();
    known_expand_shape.reserve(expand_shape_.size());
    std::size_t dyn_count = 0, non_imm_count = 0;
    for (auto &s : expand_shape_) {
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
            expand_shape_[dyn_mode] = value(s);
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
    result_ = value(data_type(r.release()));
}

fuse_inst::fuse_inst(value op, std::int64_t from, std::int64_t to, location const &lc)
    : op_(std::move(op)), from_(from), to_(to) {
    loc(lc);
    auto m = get_memref_type(loc(), op_);
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
    result_ = value(data_type(r.release()));
}

if_inst::if_inst(value condition, region then, region otherwise,
                 std::vector<scalar_type> const &return_types, location const &lc)
    : condition_(std::move(condition)), then_(std::move(then)), otherwise_(std::move(otherwise)) {
    loc(lc);
    for (auto &ty : return_types) {
        results_.push_back(value(ty));
    }
}

load_inst::load_inst(value op, std::vector<value> index_list, location const &lc)
    : op_(std::move(op)), index_list_(std::move(index_list)) {
    loc(lc);
    visit(overloaded{
              [&](group_data_type &g) {
                  if (static_cast<std::int64_t>(index_list_.size()) != 1) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result_ = value(g.ty());
              },
              [&](memref_data_type &m) {
                  if (m.dim() != static_cast<std::int64_t>(index_list_.size())) {
                      throw compilation_error(loc(), status::ir_invalid_number_of_indices);
                  }
                  result_ = value(m.element_ty());
              },
              [&](auto &) { throw compilation_error(loc(), status::ir_expected_memref_or_group); }},
          *op_->ty());
}

neg_inst::neg_inst(value a, location const &lc) : a_(std::move(a)) {
    loc(lc);

    auto at = get_scalar_type(loc(), a_);
    result_ = value(at->ty());
}

size_inst::size_inst(value op, std::int64_t mode, location const &lc)
    : op_(std::move(op)), mode_(mode) {
    loc(lc);
    auto m = get_memref_type(loc(), op_);
    bool const range_ok = 0 <= mode_ && mode_ < m->dim();
    if (!range_ok) {
        throw compilation_error(loc(), status::ir_out_of_bounds);
    }

    result_ = value(scalar_type::index);
}

subview_inst::subview_inst(value op, std::vector<slice> slices, location const &lc)
    : op_(std::move(op)), slices_(std::move(slices)) {
    loc(lc);
    auto m = get_memref_type(loc(), op_);
    if (m->dim() != static_cast<std::int64_t>(slices_.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }

    auto shape = std::vector<std::int64_t>{};
    auto stride = std::vector<std::int64_t>{};
    shape.reserve(m->dim());
    stride.reserve(m->dim());
    for (std::int64_t i = 0; i < m->dim(); ++i) {
        auto &slice = slices_[i];
        visit(overloaded{[&](int_imm &i) {
                             if (i.value() < 0) {
                                 throw compilation_error(loc(), status::ir_invalid_slice);
                             }
                         },
                         [](auto &) {}},
              *slice.first);
        if (slice.second) { // if size is given
            visit(overloaded{[&](int_imm &i) {
                                 if (i.value() < 1 && !is_dynamic_value(i.value())) {
                                     throw compilation_error(loc(), status::ir_invalid_slice);
                                 }
                             },
                             [](auto &) {}},
                  *slice.second);
            auto size = visit(overloaded{[&](int_imm &offset, int_imm &size) -> std::int64_t {
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
                              *slice.first, *slice.second);
            shape.push_back(size);
            stride.push_back(m->stride(i));
        }
    }
    auto r = std::make_unique<memref_data_type>(m->element_ty(), shape, stride);

    r->addrspace(m->addrspace());
    result_ = value(data_type(r.release()));
}

store_inst::store_inst(value val, value op, std::vector<value> index_list, location const &lc)
    : val_(std::move(val)), op_(std::move(op)), index_list_(std::move(index_list)) {
    loc(lc);
    auto v = get_scalar_type(loc(), val_);
    auto o = get_memref_type(loc(), op_);

    if (v->ty() != o->element_ty()) {
        throw compilation_error(loc(), status::ir_scalar_mismatch);
    }

    if (o->dim() != static_cast<std::int64_t>(index_list_.size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }
}

sum_inst::sum_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic,
                   location const &lc)
    : super(std::move(alpha), std::move(A), std::move(beta), std::move(B), atomic), tA_(tA) {
    loc(lc);
    auto a = get_memref_type(loc(), A_);
    auto b = get_memref_type(loc(), B_);

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
