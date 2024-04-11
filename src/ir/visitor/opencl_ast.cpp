// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/opencl_ast.hpp"
#include "error.hpp"
#include "ir/codegen_tools.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/gemm_generator.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util.hpp"

#include <clir/attr.hpp>
#include <clir/attr_defs.hpp>
#include <clir/builtin_function.hpp>
#include <clir/builtin_type.hpp>
#include <clir/data_type.hpp>
#include <clir/expr.hpp>
#include <clir/handle.hpp>
#include <clir/internal/data_type_node.hpp>
#include <clir/visit.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>

using clir::visit;

namespace tinytc {

std::string var_name(std::string name) {
    if (name.empty() || !isalpha(name[0])) {
        // we use clir unique names to clean up possible duplicates
        name = "x" + name;
    }
    return name;
}

dope_vector dope_vector::from_value(value_node &v, decl_fun_t declare) {
    memref_data_type *m = nullptr;
    auto dt = clir::data_type{};
    visit(overloaded{[&](memref_data_type &mr) {
                         m = &mr;
                         dt = internal::to_clir_ty(scalar_type::index);
                     },
                     [&](group_data_type &g) {
                         m = dynamic_cast<memref_data_type *>(g.ty().get());
                         dt = clir::pointer_to(internal::to_clir_ty(scalar_type::index,
                                                                    clir::address_space::global_t));
                     },
                     [](auto &) {}},
          *v.ty());
    if (m == nullptr) {
        throw compilation_error(
            v.loc(), status::internal_compiler_error,
            "dope_vector::from_value must only be called for memref or group type");
    }
    return dope_vector::from_memref_type(std::string(v.name()), *m, std::move(dt),
                                         std::move(declare));
}

dope_vector dope_vector::from_memref_type(std::string const &prefix, memref_data_type &m,
                                          clir::data_type dt, decl_fun_t declare) {
    auto shape = std::vector<clir::expr>{};
    auto stride = std::vector<clir::expr>{};
    shape.resize(m.dim());
    stride.resize(m.dim());
    for (std::int64_t j = 0; j < m.dim(); ++j) {
        if (is_dynamic_value(m.shape(j))) {
            auto oss = std::ostringstream{};
            oss << var_name(prefix) << "_shape" << j;
            auto var = clir::var(oss.str());
            declare(dt, var, type::shape, j);
            shape[j] = std::move(var);
        } else {
            shape[j] = m.shape(j);
        }
        if (is_dynamic_value(m.stride(j))) {
            auto oss = std::ostringstream{};
            oss << var_name(prefix) << "_stride" << j;
            auto var = clir::var(oss.str());
            declare(dt, var, type::stride, j);
            stride[j] = std::move(var);
        } else {
            stride[j] = m.stride(j);
        }
    }
    return dope_vector(std::move(shape), std::move(stride));
}

opencl_ast::opencl_ast(std::shared_ptr<core_info> info) : info_(std::move(info)) {
    declared_vars_.push_back({});
}

auto opencl_ast::get_dope_vector(value_node *v) -> dope_vector & {
    auto dv = dope_vector_.find(std::bit_cast<std::uintptr_t>(v));
    if (dv == dope_vector_.end()) {
        throw compilation_error(v->loc(), status::internal_compiler_error,
                                "Dope vector for value is missing");
    }
    return dv->second;
}

void opencl_ast::set_dope_vector(value_node *v, dope_vector dv) {
    uintptr_t u = std::bit_cast<uintptr_t>(v);
    dope_vector_[u] = std::move(dv);
}

clir::var opencl_ast::declare(value_node &v) {
    uintptr_t u = std::bit_cast<uintptr_t>(&v);
    for (auto it = declared_vars_.rbegin(); it != declared_vars_.rend(); ++it) {
        if (it->find(u) != it->end()) {
            throw compilation_error(v.loc(), status::internal_compiler_error,
                                    "Variable already declared");
        }
    }

    auto name = var_name(std::string(v.name()));
    declared_vars_.back()[u] = clir::var(std::move(name));
    return declared_vars_.back()[u];
}

memref_data_type *opencl_ast::get_memref_type(value_node &v) {
    auto t = dynamic_cast<memref_data_type *>(v.ty().get());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

scalar_type opencl_ast::get_scalar_type(data_type_node &ty) {
    return visit(overloaded{[](scalar_data_type &i) -> scalar_type { return i.ty(); },
                            [](memref_data_type &i) -> scalar_type { return i.element_ty(); },
                            [&](auto &i) -> scalar_type {
                                throw compilation_error(i.loc(),
                                                        status::ir_expected_memref_or_scalar);
                                return scalar_type{};
                            }},
                 ty);
};

/* Data type nodes */
clir::data_type opencl_ast::operator()(void_data_type &) { return clir::builtin_type::void_t; }
clir::data_type opencl_ast::operator()(group_data_type &g) {
    auto ptr_ty = visit(*this, *g.ty());
    ptr_ty = visit(overloaded{[](clir::internal::pointer &t) {
                                  return clir::pointer_to(
                                      clir::pointer_to(t.ty(), clir::address_space::global_t));
                              },
                              [](auto &) { return clir::data_type{}; }},
                   *ptr_ty);
    if (!ptr_ty) {
        throw compilation_error(g.loc(), status::internal_compiler_error,
                                "Could not determine OpenCL type of group type");
    }
    return ptr_ty;
}
clir::data_type opencl_ast::operator()(memref_data_type &d) {
    return clir::pointer_to(d.clir_element_ty());
}
clir::data_type opencl_ast::operator()(scalar_data_type &s) { return s.clir_ty(); }

/* Value nodes */
clir::expr opencl_ast::operator()(float_imm &v) {
    auto ty = get_scalar_type(*v.ty());
    return clir::expr(v.value(), static_cast<short>(size(ty) * 8));
}
clir::expr opencl_ast::operator()(int_imm &v) {
    auto ty = get_scalar_type(*v.ty());
    return clir::expr(v.value(), static_cast<short>(size(ty) * 8));
}
clir::expr opencl_ast::operator()(val &v) {
    uintptr_t u = std::bit_cast<uintptr_t>(&v);
    for (auto it = declared_vars_.rbegin(); it != declared_vars_.rend(); ++it) {
        if (auto j = it->find(u); j != it->end()) {
            return j->second;
        }
    }

    throw compilation_error(v.loc(), status::internal_compiler_error,
                            "Undeclared variable: " + std::string(v.name()));
}

/* Stmt nodes */
std::vector<clir::stmt> opencl_ast::operator()(alloca_inst &a) {
    if (a.stack_ptr() < 0) {
        throw compilation_error(a.loc(), status::internal_compiler_error,
                                "Invalid stack_ptr in alloca. Did you run set_stack_ptrs?");
    }
    auto result_var = declare(*a.result());
    auto t = dynamic_cast<memref_data_type *>(a.result()->ty().get());
    if (t == nullptr) {
        throw compilation_error(a.loc(), status::ir_expected_memref);
    }
    auto ptr_ty = clir::pointer_to(t->clir_element_ty());
    auto result = declaration_assignment(ptr_ty, std::move(result_var),
                                         clir::cast(ptr_ty, stack_ + a.stack_ptr()));
    stack_high_water_mark_ = std::max(stack_high_water_mark_,
                                      static_cast<std::size_t>(a.stack_ptr()) + t->size_in_bytes());

    // no declarations are neceesary as alloca only accepts fixed-size memrefs
    set_dope_vector(a.result().get(),
                    dope_vector::from_value(*a.result(), [](clir::data_type, clir::var,
                                                            dope_vector::type, std::int64_t) {}));
    return {std::move(result)};
}

std::vector<clir::stmt> opencl_ast::operator()(axpby_inst &inst) {
    auto at = get_memref_type(*inst.A());
    auto bt = get_memref_type(*inst.B());
    auto &adv = get_dope_vector(inst.A().get());
    auto &bdv = get_dope_vector(inst.B().get());

    auto pA = inst.tA() == transpose::T && at->dim() == 2 ? 1 : 0;

    auto alpha = visit(*this, *inst.alpha());
    auto beta = visit(*this, *inst.beta());
    auto const inner_loop = [&](clir::block_builder &bb, clir::expr Ab, clir::expr Bb,
                                clir::expr trip_count, std::size_t num_tiles, clir::var sg_id) {
        auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
        tile_loop_by_sgs(
            bb, std::move(trip_count), core_cfg_.subgroup_size, num_tiles, std::move(sg_id),
            [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                clir::expr inner_trip_count) {
                auto const inner_loop = [&](clir::block_builder &bb) {
                    auto a = Ab[(block + m) * adv.stride(pA)];
                    auto b = bb.declare_assign((*this)(*bt), "b", Bb + (block + m) * bdv.stride(0));
                    store_helper(bb, inst.atomic(), b, bt->element_ty(), bt->addrspace(),
                                 alpha * std::move(a), beta);
                };
                if (is_remainder) {
                    bb.add(clir::if_selection_builder(m < std::move(inner_trip_count))
                               .then(inner_loop)
                               .get_product());
                } else {
                    inner_loop(bb);
                }
            });
    };

    auto A = visit(*this, *inst.A());
    auto B = visit(*this, *inst.B());
    if (bt->dim() == 0) {
        auto bb = clir::block_builder{};
        store_helper(bb, inst.atomic(), B, bt->element_ty(), bt->addrspace(),
                     std::move(alpha) * A[0], std::move(beta));
        return {bb.get_product()};
    }

    if (bt->dim() == 1) {
        auto bb = clir::block_builder{};
        auto sg_m = bb.declare_assign(clir::generic_uint(), "sg_m", clir::get_sub_group_id());
        inner_loop(bb, std::move(A), std::move(B), bdv.shape(0),
                   tiling_.m_tiles() * tiling_.n_tiles(), std::move(sg_m));
        return {bb.get_product()};
    } else if (bt->dim() == 2) {
        auto bb = clir::block_builder{};
        auto sg_n = bb.declare_assign(clir::generic_uint(), "sg_n",
                                      clir::get_sub_group_id() / tiling_.m_tiles());
        auto sg_m = bb.declare_assign(clir::generic_uint(), "sg_m",
                                      clir::get_sub_group_id() % tiling_.m_tiles());
        tile_loop_uniformly(
            bb, bdv.shape(1), core_cfg_.subgroup_size, tiling_.n_tiles(), std::move(sg_n),
            [&](clir::block_builder &bb, clir::expr block, clir::expr trip_count) {
                auto n = clir::var("n");
                bb.add(
                    clir::for_loop_builder(clir::declaration_assignment(clir::generic_int(), n, 0),
                                           n < std::move(trip_count), ++n)
                        .body([&](clir::block_builder &bb) {
                            auto Ab = bb.declare_assign(this->operator()(*at), "Ab",
                                                        A + (block + n) * adv.stride(1 - pA));
                            auto Bb = bb.declare_assign(this->operator()(*bt), "Bb",
                                                        B + (block + n) * bdv.stride(1));
                            inner_loop(bb, Ab, Bb, bdv.shape(0), tiling_.m_tiles(), sg_m);
                        })
                        .get_product());
            });
        return {bb.get_product()};
    }
    throw compilation_error(inst.loc(), status::ir_expected_vector_or_matrix);
}

std::vector<clir::stmt> opencl_ast::operator()(barrier_inst &) {
    return {clir::expression_statement(clir::call_builtin(
        clir::builtin_function::barrier, {clir::cl_mem_fence_flags::CLK_LOCAL_MEM_FENCE}))};
}

std::vector<clir::stmt> opencl_ast::operator()(binary_op_inst &b) {
    auto const make = [](binary_op op, clir::expr a, clir::expr b, bool floating) -> clir::expr {
        switch (op) {
        case binary_op::add:
            return std::move(a) + std::move(b);
        case binary_op::sub:
            return std::move(a) - std::move(b);
        case binary_op::mul:
            return std::move(a) * std::move(b);
        case binary_op::div:
            return std::move(a) / std::move(b);
        case binary_op::rem:
            if (floating) {
                return clir::fmod(std::move(a), std::move(b));
            }
            return std::move(a) % std::move(b);
        }
        return {};
    };
    auto sty = get_scalar_type(*b.a()->ty());
    auto v = declare(*b.result());
    return {declaration_assignment(
        visit(*this, *b.result()->ty()), std::move(v),
        make(b.op(), visit(*this, *b.a()), visit(*this, *b.b()), is_floating_type(sty)))};
}

std::vector<clir::stmt> opencl_ast::operator()(cast_inst &c) {
    auto v = declare(*c.result());
    auto result_ty = visit(*this, *c.result()->ty());
    auto cst = cast(result_ty, visit(*this, *c.a()));
    return {declaration_assignment(std::move(result_ty), std::move(v), std::move(cst))};
}

std::vector<clir::stmt> opencl_ast::operator()(compare_inst &c) {
    auto const make = [](cmp_condition cond, clir::expr a, clir::expr b) -> clir::expr {
        switch (cond) {
        case cmp_condition::eq:
            return std::move(a) == std::move(b);
        case cmp_condition::ne:
            return std::move(a) != std::move(b);
        case cmp_condition::gt:
            return std::move(a) > std::move(b);
        case cmp_condition::ge:
            return std::move(a) >= std::move(b);
        case cmp_condition::lt:
            return std::move(a) < std::move(b);
        case cmp_condition::le:
            return std::move(a) <= std::move(b);
        }
        return {};
    };
    auto v = declare(*c.result());
    return {declaration_assignment(visit(*this, *c.result()->ty()), std::move(v),
                                   make(c.cond(), visit(*this, *c.a()), visit(*this, *c.b())))};
}

std::vector<clir::stmt> opencl_ast::operator()(expand_inst &e) {
    auto result_var = declare(*e.result());
    auto m = get_memref_type(*e.operand());
    auto &dv = get_dope_vector(e.operand().get());
    auto &eshape = e.expand_shape();

    auto rhs = visit(*this, *e.operand());
    auto clinst = std::vector<clir::stmt>{};
    clinst.emplace_back(
        clir::declaration_assignment(this->operator()(*m), std::move(result_var), std::move(rhs)));

    auto shape = std::vector<clir::expr>{};
    auto stride = std::vector<clir::expr>{};
    shape.reserve(m->dim() + eshape.size() - 1);
    stride.reserve(m->dim() + eshape.size() - 1);
    std::int64_t i = 0;
    for (; i < e.mode(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }

    auto eshape_cl = std::vector<clir::expr>{};
    eshape_cl.reserve(eshape.size());
    for (auto &s : eshape) {
        eshape_cl.push_back(visit(*this, *s));
    }

    auto const get_shape = [&](std::size_t j) -> clir::expr {
        auto is_dynamic = visit(overloaded{[&](int_imm &i) { return is_dynamic_value(i.value()); },
                                           [](auto &) { return false; }},
                                *eshape[j]);
        if (is_dynamic) {
            clir::expr prod = 1;
            for (std::size_t k = 0; k < eshape_cl.size(); ++k) {
                if (j != k) {
                    prod = prod * eshape_cl[k];
                }
            }
            auto inferred_size = clir::var("inferred_size");
            clinst.emplace_back(
                clir::declaration_assignment(internal::to_clir_ty(scalar_type::index),
                                             inferred_size, std::move(prod) / dv.shape(e.mode())));
            return std::move(inferred_size);
        }
        return eshape_cl[j];
    };

    stride.push_back(m->stride(e.mode()));
    shape.push_back(get_shape(0));
    for (std::size_t j = 1; j < eshape.size(); ++j) {
        stride.push_back(stride.back() * shape.back());
        shape.push_back(get_shape(j));
    }
    for (i = e.mode() + 1; i < m->dim(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }

    set_dope_vector(e.result().get(),
                    dope_vector::from_value(*e.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride[j] : shape[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}
std::vector<clir::stmt> opencl_ast::operator()(fuse_inst &f) {
    auto result_var = declare(*f.result());
    auto m = get_memref_type(*f.operand());
    auto &dv = get_dope_vector(f.operand().get());

    auto rhs = visit(*this, *f.operand());
    auto shape = std::vector<clir::expr>{};
    auto stride = std::vector<clir::expr>{};
    shape.reserve(m->dim());
    stride.reserve(m->dim());
    std::int64_t i = 0;
    for (; i < f.from(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }
    clir::expr prod = dv.shape(i++);
    for (; i <= f.to(); ++i) {
        prod = prod * dv.shape(i);
    }
    shape.push_back(prod);
    stride.push_back(dv.stride(f.from()));
    for (i = f.to() + 1; i < m->dim(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }

    auto clinst = std::vector<clir::stmt>{};
    clinst.emplace_back(
        clir::declaration_assignment(this->operator()(*m), std::move(result_var), std::move(rhs)));

    set_dope_vector(f.result().get(),
                    dope_vector::from_value(*f.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride[j] : shape[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}

std::vector<clir::stmt> opencl_ast::operator()(load_inst &e) {
    auto op_val = e.operand();
    auto rhs = visit(*this, *op_val);

    auto clinst = std::vector<clir::stmt>{};

    visit(overloaded{[&](group_data_type &) {
                         if (e.index_list().size() != 1) {
                             throw compilation_error(e.loc(), status::ir_invalid_number_of_indices);
                         }
                         auto idx = visit(*this, *e.index_list().front());
                         rhs = rhs + idx;

                         auto &dv = get_dope_vector(e.operand().get());
                         set_dope_vector(
                             e.result().get(),
                             dope_vector::from_value(
                                 *e.result(), [&](clir::data_type a, clir::var b,
                                                  dope_vector::type t, std::int64_t j) {
                                     auto init = t == dope_vector::type::stride ? dv.stride(j)
                                                                                : dv.shape(j);
                                     clinst.emplace_back(clir::declaration_assignment(
                                         std::move(a), std::move(b), std::move(init)[idx]));
                                 }));
                     },
                     [&](memref_data_type &m) {
                         if (static_cast<std::int64_t>(e.index_list().size()) != m.dim()) {
                             throw compilation_error(e.loc(), status::ir_invalid_number_of_indices);
                         }
                         auto &dv = get_dope_vector(e.operand().get());
                         for (std::int64_t i = 0; i < m.dim(); ++i) {
                             rhs = rhs + visit(*this, *e.index_list()[i]) * dv.stride(i);
                         }
                     },
                     [&e](auto &) {
                         throw compilation_error(e.loc(), status::ir_expected_memref_or_group);
                     }},
          *e.operand()->ty());

    auto lhs = declare(*e.result());
    auto result_type = e.result()->ty().get();
    if (result_type == nullptr) {
        throw compilation_error(e.loc(), status::internal_compiler_error, "Expected type");
    }
    clinst.emplace(clinst.begin(),
                   declaration_assignment(visit(*this, *result_type), std::move(lhs),
                                          clir::dereference(std::move(rhs))));

    return clinst;
}

std::vector<clir::stmt> opencl_ast::operator()(group_id_inst &g) {
    auto rhs = clir::get_global_id(2);
    auto lhs = declare(*g.result());
    return {
        declaration_assignment(visit(*this, *g.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> opencl_ast::operator()(group_size_inst &g) {
    auto rhs = clir::get_global_size(2);
    auto lhs = declare(*g.result());
    return {
        declaration_assignment(visit(*this, *g.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> opencl_ast::operator()(lifetime_stop_inst &) { return {}; }

std::vector<clir::stmt> opencl_ast::operator()(gemm_inst &g) {
    auto a = get_memref_type(*g.A());
    auto b = get_memref_type(*g.B());
    auto c = get_memref_type(*g.C());
    auto &adv = get_dope_vector(g.A().get());
    auto &bdv = get_dope_vector(g.B().get());
    auto &cdv = get_dope_vector(g.C().get());

    auto const M = c->shape(0);
    auto const N = c->shape(1);
    auto const ak = g.tA() == transpose::T ? 0 : 1;
    auto const K = a->shape(ak);

    auto const get_fixed = [](value &v) {
        return visit(overloaded{[&](int_imm &i) -> std::optional<double> { return i.value(); },
                                [&](float_imm &i) -> std::optional<double> { return i.value(); },
                                [](auto &) -> std::optional<double> { return std::nullopt; }},
                     *v);
    };

    auto gemm_ty =
        gemm_scalar_type{get_scalar_type(*g.alpha()->ty()), a->element_ty(), b->element_ty(),
                         get_scalar_type(*g.beta()->ty()), c->element_ty()};
    auto cfg = gemm_configuration{std::move(gemm_ty),
                                  g.tA(),
                                  g.tB(),
                                  M,
                                  N,
                                  K,
                                  {a->stride(0), a->stride(1)},
                                  {b->stride(0), b->stride(1)},
                                  {c->stride(0), c->stride(1)},
                                  get_fixed(g.alpha()),
                                  get_fixed(g.beta()),
                                  g.atomic()};
    auto name = cfg.identifier();
    int name_counter = 0;
    while (reserved_names_.find(name) != reserved_names_.end()) {
        name = cfg.identifier("gemm" + std::to_string(++name_counter));
    }
    if (has_gemm_.find(name) == has_gemm_.end()) {
        auto f = generate_gemm(cfg, tiling_, core_cfg_, name, a->addrspace(), b->addrspace(),
                               c->addrspace());
        prog_builder_.add(std::move(f));
    }
    has_gemm_.emplace(name);
    return {clir::expression_statement(clir::call(
        std::move(name),
        {cdv.shape(0), cdv.shape(1), adv.shape(ak), visit(*this, *g.alpha()), visit(*this, *g.A()),
         adv.stride(0), adv.stride(1), visit(*this, *g.B()), bdv.stride(0), bdv.stride(1),
         visit(*this, *g.beta()), visit(*this, *g.C()), cdv.stride(0), cdv.stride(1)}))};
}

std::vector<clir::stmt> opencl_ast::operator()(gemv_inst &g) {
    auto a = get_memref_type(*g.A());
    auto b = get_memref_type(*g.B());
    auto c = get_memref_type(*g.C());
    auto &adv = get_dope_vector(g.A().get());
    auto &bdv = get_dope_vector(g.B().get());
    auto &cdv = get_dope_vector(g.C().get());

    auto const M = c->shape(0);
    auto const ak = g.tA() == transpose::T ? 0 : 1;
    auto const K = a->shape(ak);
    auto const N = 1;
    auto const get_fixed = [](value &v) {
        return visit(overloaded{[&](int_imm &i) -> std::optional<double> { return i.value(); },
                                [&](float_imm &i) -> std::optional<double> { return i.value(); },
                                [](auto &) -> std::optional<double> { return std::nullopt; }},
                     *v);
    };

    auto gemm_ty =
        gemm_scalar_type{get_scalar_type(*g.alpha()->ty()), a->element_ty(), b->element_ty(),
                         get_scalar_type(*g.beta()->ty()), c->element_ty()};
    auto cfg = gemm_configuration{std::move(gemm_ty),
                                  g.tA(),
                                  transpose::N,
                                  M,
                                  N,
                                  K,
                                  {a->stride(0), a->stride(1)},
                                  {b->stride(0), 0},
                                  {c->stride(0), 0},
                                  get_fixed(g.alpha()),
                                  get_fixed(g.beta()),
                                  g.atomic()};
    auto name = cfg.identifier("gemv");
    int name_counter = 0;
    while (reserved_names_.find(name) != reserved_names_.end()) {
        name = cfg.identifier("gemv" + std::to_string(++name_counter));
    }
    if (has_gemm_.find(name) == has_gemm_.end()) {
        auto f = generate_gemm(cfg, tiling_, core_cfg_, name, a->addrspace(), b->addrspace(),
                               c->addrspace());
        prog_builder_.add(std::move(f));
    }
    has_gemm_.emplace(name);
    return {clir::expression_statement(
        clir::call(std::move(name),
                   {cdv.shape(0), 1, adv.shape(ak), visit(*this, *g.alpha()), visit(*this, *g.A()),
                    adv.stride(0), adv.stride(1), visit(*this, *g.B()), bdv.stride(0), 0,
                    visit(*this, *g.beta()), visit(*this, *g.C()), cdv.stride(0), 0}))};
}

std::vector<clir::stmt> opencl_ast::operator()(ger_inst &g) {
    auto bt = get_memref_type(*g.B());
    auto ct = get_memref_type(*g.C());
    auto &adv = get_dope_vector(g.A().get());
    auto &bdv = get_dope_vector(g.B().get());
    auto &cdv = get_dope_vector(g.C().get());

    auto alpha = visit(*this, *g.alpha());
    auto beta = visit(*this, *g.beta());

    auto A = visit(*this, *g.A());
    auto B = visit(*this, *g.B());
    auto C = visit(*this, *g.C());

    auto bb = clir::block_builder{};
    auto sg_n = bb.declare_assign(clir::generic_uint(), "sg_n",
                                  clir::get_sub_group_id() / tiling_.m_tiles());
    auto sg_m = bb.declare_assign(clir::generic_uint(), "sg_m",
                                  clir::get_sub_group_id() % tiling_.m_tiles());
    tile_loop_uniformly(
        bb, cdv.shape(1), core_cfg_.subgroup_size, tiling_.n_tiles(), std::move(sg_n),
        [&](clir::block_builder &bb, clir::expr block, clir::expr trip_count) {
            auto n = clir::var("n");
            bb.add(clir::for_loop_builder(clir::declaration_assignment(clir::generic_int(), n, 0),
                                          n < std::move(trip_count), ++n)
                       .body([&](clir::block_builder &bb) {
                           auto b = bb.declare_assign(internal::to_clir_ty(bt->element_ty()), "b",
                                                      B + (block + n) * bdv.stride(0));
                           auto Cb = bb.declare_assign(this->operator()(*ct), "Cb",
                                                       C + (block + n) * cdv.stride(1));
                           auto m = bb.declare_assign(clir::generic_uint(), "m",
                                                      clir::get_sub_group_local_id());
                           tile_loop_by_sgs(
                               bb, cdv.shape(0), core_cfg_.subgroup_size, tiling_.m_tiles(), sg_m,
                               [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                                   clir::expr inner_trip_count) {
                                   auto const inner_loop = [&](clir::block_builder &bb) {
                                       auto a = A[(block + m) * adv.stride(0)];
                                       auto c = bb.declare_assign((*this)(*ct), "c",
                                                                  Cb + (block + m) * cdv.stride(0));
                                       store_helper(bb, g.atomic(), c, ct->element_ty(),
                                                    ct->addrspace(), alpha * std::move(a) * b,
                                                    beta);
                                   };
                                   if (is_remainder) {
                                       bb.add(clir::if_selection_builder(
                                                  m < std::move(inner_trip_count))
                                                  .then(inner_loop)
                                                  .get_product());
                                   } else {
                                       inner_loop(bb);
                                   }
                               });
                       })
                       .get_product());
        });
    return {bb.get_product()};
}

std::vector<clir::stmt> opencl_ast::operator()(for_inst &p) {
    auto clinst = std::vector<clir::stmt>{};

    auto lv = declare(*p.loop_var());
    auto lv_ty = visit(*this, *p.loop_var()->ty());
    auto start = clir::declaration_assignment(std::move(lv_ty), lv, visit(*this, *p.from()));
    auto condition = lv < visit(*this, *p.to());
    auto step = p.step() ? clir::add_into(lv, visit(*this, *p.step())) : ++lv;
    auto body = visit(*this, *p.body());
    clinst.emplace_back(clir::stmt(std::make_shared<clir::internal::for_loop>(
        std::move(start), std::move(condition), std::move(step), std::move(body))));

    return clinst;
}

std::vector<clir::stmt> opencl_ast::operator()(foreach_inst &p) {
    auto lv = declare(*p.loop_var());
    auto lv_ty = visit(*this, *p.loop_var()->ty());
    auto from = visit(*this, *p.from());
    auto to = visit(*this, *p.to());
    auto bb = clir::block_builder{};
    auto sg = bb.declare_assign(clir::generic_uint(), "sg", clir::get_sub_group_id());
    auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
    auto trip_count = bb.declare_assign(lv_ty, "trip_count", to - from);
    tile_loop_by_sgs(
        bb, trip_count, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(),
        std::move(sg), [&](clir::block_builder &bb, clir::expr block, bool, clir::expr) {
            bb.add(clir::declaration_assignment(lv_ty, lv, std::move(block) + m + from));
            bb.add(visit(*this, *p.body()));
        });
    return {bb.get_product()};
}

std::vector<clir::stmt> opencl_ast::operator()(hadamard_inst &g) {
    auto ct = get_memref_type(*g.C());
    auto &adv = get_dope_vector(g.A().get());
    auto &bdv = get_dope_vector(g.B().get());
    auto &cdv = get_dope_vector(g.C().get());

    auto alpha = visit(*this, *g.alpha());
    auto beta = visit(*this, *g.beta());

    auto A = visit(*this, *g.A());
    auto B = visit(*this, *g.B());
    auto C = visit(*this, *g.C());

    auto bb = clir::block_builder{};
    auto sg = bb.declare_assign(clir::generic_uint(), "sg", clir::get_sub_group_id());
    auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
    tile_loop_by_sgs(bb, cdv.shape(0), core_cfg_.subgroup_size,
                     tiling_.m_tiles() * tiling_.n_tiles(), std::move(sg),
                     [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                         clir::expr inner_trip_count) {
                         auto const inner_loop = [&](clir::block_builder &bb) {
                             auto b = B[(block + m) * bdv.stride(0)];
                             auto a = A[(block + m) * adv.stride(0)];
                             auto c = bb.declare_assign((*this)(*ct), "c",
                                                        C + (block + m) * cdv.stride(0));
                             store_helper(bb, g.atomic(), c, ct->element_ty(), ct->addrspace(),
                                          alpha * std::move(a) * std::move(b), beta);
                         };
                         if (is_remainder) {
                             bb.add(clir::if_selection_builder(m < std::move(inner_trip_count))
                                        .then(inner_loop)
                                        .get_product());
                         } else {
                             inner_loop(bb);
                         }
                     });
    return {bb.get_product()};
}

std::vector<clir::stmt> opencl_ast::operator()(if_inst &in) {
    auto clinst = std::vector<clir::stmt>{};
    yielded_vars_.push_back(std::vector<clir::var>{});
    for (auto &r : in.results_ref()) {
        auto v = declare(*r);
        clinst.emplace_back(clir::declaration(visit(*this, *r->ty()), v));
        yielded_vars_.back().emplace_back(std::move(v));
    }
    auto ib = clir::if_selection_builder(visit(*this, *in.condition()));
    ib.set_then(visit(*this, *in.then()));
    if (in.otherwise()) {
        ib.set_otherwise(visit(*this, *in.otherwise()));
    }
    yielded_vars_.pop_back();
    clinst.emplace_back(ib.get_product());
    return clinst;
}

std::vector<clir::stmt> opencl_ast::operator()(neg_inst &n) {
    auto v = declare(*n.result());
    return {declaration_assignment(visit(*this, *n.result()->ty()), std::move(v),
                                   -visit(*this, *n.a()))};
}

std::vector<clir::stmt> opencl_ast::operator()(size_inst &s) {
    auto v = declare(*s.result());
    auto &dv = get_dope_vector(s.operand().get());

    return {clir::declaration_assignment(visit(*this, *s.result()->ty()), std::move(v),
                                         dv.shape(s.mode()))};
}

std::vector<clir::stmt> opencl_ast::operator()(subview_inst &s) {
    auto result_var = declare(*s.result());
    auto t = get_memref_type(*s.operand());
    if (t->dim() != static_cast<std::int64_t>(s.slices().size())) {
        throw compilation_error(s.loc(), status::ir_invalid_number_of_indices);
    }

    auto &dv = get_dope_vector(s.operand().get());

    auto rhs = visit(*this, *s.operand());
    int j = 0;
    auto shape_out = std::vector<clir::expr>{};
    auto stride_out = std::vector<clir::expr>{};
    shape_out.reserve(t->dim());
    stride_out.reserve(t->dim());
    auto &slices = s.slices();
    for (auto &slice : slices) {
        auto offset = visit(*this, *slice.first);
        rhs = rhs + std::move(offset) * dv.stride(j);
        if (slice.second) {
            bool is_size_unknown = visit(
                overloaded{[&](int_imm &size) -> bool { return is_dynamic_value(size.value()); },
                           [](auto &) -> bool { return false; }},
                *slice.second);
            auto size = clir::expr{};
            if (is_size_unknown) {
                size = dv.shape(j) - visit(*this, *slice.first);
            } else {
                size = visit(*this, *slice.second);
            }
            shape_out.emplace_back(size);
            stride_out.emplace_back(dv.stride(j));
        }
        ++j;
    }

    auto clinst = std::vector<clir::stmt>{};
    clinst.emplace_back(
        clir::declaration_assignment(this->operator()(*t), std::move(result_var), std::move(rhs)));

    set_dope_vector(s.result().get(),
                    dope_vector::from_value(*s.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride_out[j] : shape_out[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}

std::vector<clir::stmt> opencl_ast::operator()(store_inst &s) {
    auto ot = get_memref_type(*s.operand());

    if (static_cast<std::int64_t>(s.index_list().size()) != ot->dim()) {
        throw compilation_error(s.loc(), status::ir_invalid_number_of_indices);
    }

    auto lhs = visit(*this, *s.operand());
    auto &dv = get_dope_vector(s.operand().get());
    for (std::int64_t i = 0; i < ot->dim(); ++i) {
        lhs = lhs + visit(*this, *s.index_list()[i]) * dv.stride(i);
    }

    auto rhs = visit(*this, *s.val());
    auto st = assignment(dereference(std::move(lhs)), std::move(rhs));
    return {expression_statement(std::move(st))};
}

std::vector<clir::stmt> opencl_ast::operator()(sum_inst &inst) {
    auto at = get_memref_type(*inst.A());
    auto bt = get_memref_type(*inst.B());
    auto &adv = get_dope_vector(inst.A().get());
    auto &bdv = get_dope_vector(inst.B().get());

    auto alpha = visit(*this, *inst.alpha());
    auto beta = visit(*this, *inst.beta());

    auto zero = clir::expr(0.0, static_cast<short>(size(at->element_ty()) * 8));

    auto A = visit(*this, *inst.A());
    auto B = visit(*this, *inst.B());
    auto bb = clir::block_builder{};
    auto acc = bb.declare_assign(internal::to_clir_ty(at->element_ty()), "acc", std::move(zero));
    auto sg = bb.declare_assign(clir::generic_uint(), "sg", clir::get_sub_group_id());
    auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
    if (bt->dim() == 0) {
        tile_loop_by_sgs(bb, adv.shape(0), core_cfg_.subgroup_size,
                         tiling_.n_tiles() * tiling_.m_tiles(), std::move(sg),
                         [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                             clir::expr inner_trip_count) {
                             auto const inner_loop = [&](clir::block_builder &bb) {
                                 auto a = A[(block + m) * adv.stride(0)];
                                 bb.add(add_into(acc, std::move(a)));
                             };
                             if (is_remainder) {
                                 bb.add(clir::if_selection_builder(m < std::move(inner_trip_count))
                                            .then(inner_loop)
                                            .get_product());
                             } else {
                                 inner_loop(bb);
                             }
                         });
        auto sum = bb.declare_assign(internal::to_clir_ty(bt->element_ty()), "sum",
                                     clir::work_group_reduce_add(acc));
        bb.add(clir::if_selection_builder(clir::get_sub_group_id() == 0 &&
                                          clir::get_sub_group_local_id() == 0)
                   .then([&](clir::block_builder &bb) {
                       store_helper(bb, inst.atomic(), B, bt->element_ty(), bt->addrspace(),
                                    alpha * sum, beta);
                   })
                   .get_product());
    } else if (bt->dim() == 1) {
        auto ak = inst.tA() == transpose::T ? 0 : 1;
        tile_loop_by_sgs(
            bb, adv.shape(0), core_cfg_.subgroup_size, tiling_.n_tiles() * tiling_.m_tiles(),
            std::move(sg),
            [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                clir::expr inner_trip_count) {
                auto n = clir::var("n");
                auto const inner_loop = [&](clir::block_builder &bb) {
                    bb.add(clir::for_loop_builder(
                               clir::declaration_assignment(clir::generic_int(), n, 0),
                               n < adv.shape(ak), ++n)
                               .body([&](clir::block_builder &bb) {
                                   auto a =
                                       A[(block + m) * adv.stride(1 - ak) + n * adv.stride(ak)];
                                   bb.add(add_into(acc, std::move(a)));
                               })
                               .get_product());
                    auto b = bb.declare_assign((*this)(*bt), "b", B + (block + m) * bdv.stride(0));
                    store_helper(bb, inst.atomic(), b, bt->element_ty(), bt->addrspace(),
                                 alpha * acc, beta);
                };
                if (is_remainder) {
                    bb.add(clir::if_selection_builder(m < std::move(inner_trip_count))
                               .then(inner_loop)
                               .get_product());
                } else {
                    inner_loop(bb);
                }
            });
    } else {
        throw compilation_error(inst.loc(), status::ir_expected_vector_or_matrix);
    }
    return {bb.get_product()};
}

std::vector<clir::stmt> opencl_ast::operator()(yield_inst &in) {
    if (yielded_vars_.empty()) {
        throw compilation_error(in.loc(), status::ir_unexpected_yield);
    }
    if (yielded_vars_.back().size() != in.vals().size()) {
        throw compilation_error(in.loc(), status::ir_yield_mismatch);
    }
    std::vector<clir::stmt> clinst;
    for (std::size_t i = 0; i < in.vals().size(); ++i) {
        clinst.push_back(clir::expression_statement(
            clir::assignment(yielded_vars_.back()[i], visit(*this, *in.vals()[i]))));
    }
    return clinst;
}

/* Region nodes */
clir::stmt opencl_ast::operator()(rgn &b) {
    declared_vars_.push_back({});
    auto bb = clir::block_builder{};
    for (auto &s : b.insts()) {
        for (auto &cs : visit(*this, *s)) {
            bb.add(cs);
        }
    }
    declared_vars_.pop_back();
    return bb.get_product();
}

/* Function nodes */
clir::func opencl_ast::operator()(prototype &p) {
    auto fb = clir::kernel_builder(std::string(p.name()));
    for (auto &v : p.args()) {
        fb.argument(visit(*this, *v->ty()), declare(*v));
        auto dv = visit(
            overloaded{[&fb, &v](memref_data_type &) -> std::optional<dope_vector> {
                           return std::make_optional(dope_vector::from_value(
                               *v, [&](clir::data_type a, clir::var b, dope_vector::type,
                                       std::int64_t) { fb.argument(std::move(a), std::move(b)); }));
                       },
                       [&fb, &v](group_data_type &) -> std::optional<dope_vector> {
                           return std::make_optional(dope_vector::from_value(
                               *v, [&](clir::data_type a, clir::var b, dope_vector::type,
                                       std::int64_t) { fb.argument(std::move(a), std::move(b)); }));
                       },
                       [](auto &) { return std::nullopt; }},
            *v->ty());
        if (dv) {
            set_dope_vector(v.get(), std::move(*dv));
        }
    }

    auto const wgs = tiling_.work_group_size(core_cfg_.subgroup_size);
    fb.attribute(clir::reqd_work_group_size(wgs[0], wgs[1], 1));
    fb.attribute(clir::intel_reqd_sub_group_size(core_cfg_.subgroup_size));
    return fb.get_product();
}

clir::func opencl_ast::operator()(function &fn) {
    auto const subgroup_size = fn.subgroup_size();
    try {
        core_cfg_ = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    auto const work_group_size = fn.work_group_size();
    tiling_[0] = work_group_size[0] / subgroup_size;
    tiling_[1] = work_group_size[1];

    stack_ = clir::var("stack");
    auto proto = visit(*this, *fn.prototype());
    auto body = visit(*this, *fn.body());
    if (stack_high_water_mark_ > 0) {
        auto bb = dynamic_cast<clir::internal::block *>(body.get());
        if (bb == nullptr) {
            throw compilation_error(fn.loc(), status::internal_compiler_error,
                                    "Expected clir basic block");
        }
        bb->stmts().insert(bb->stmts().begin(),
                           declaration(clir::array_of(clir::data_type(clir::builtin_type::uchar_t,
                                                                      clir::address_space::local_t),
                                                      stack_high_water_mark_),
                                       stack_, {clir::aligned(size(scalar_type::f64) * 8)}));
    }
    return clir::function(std::move(proto), std::move(body));
}

/* Program nodes */
clir::prog opencl_ast::operator()(program &p) {
    struct name_visitor {
        auto operator()(function &f) -> std::string_view { return visit(*this, *f.prototype()); }
        auto operator()(prototype &p) -> std::string_view { return p.name(); }
    };
    reserved_names_.clear();
    for (auto &decl : p.declarations()) {
        reserved_names_.insert(std::string(visit(name_visitor{}, *decl)));
    }

    prog_builder_ = clir::program_builder{};
    for (auto &decl : p.declarations()) {
        stack_high_water_mark_ = 0;
        prog_builder_.add(visit(*this, *decl));
    }
    return prog_builder_.get_product();
}

} // namespace tinytc
