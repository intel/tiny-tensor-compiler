// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/convert_to_opencl.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "gemm_generator.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

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
#include <complex>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace tinytc {

std::string var_name(std::string name) {
    if (name.empty() || !isalpha(name[0])) {
        // we use clir unique names to clean up possible duplicates
        name = "x" + name;
    }
    return name;
}

dope_vector dope_vector::from_value(value_node const &v, decl_fun_t declare) {
    memref_data_type const *m = nullptr;
    auto dt = clir::data_type{};
    visit(overloaded{[&](memref_data_type const &mr) {
                         m = &mr;
                         dt = to_clir_ty(scalar_type::index);
                     },
                     [&](group_data_type const &g) {
                         m = dyn_cast<memref_data_type>(g.ty());
                         dt = clir::pointer_to(
                             to_clir_ty(scalar_type::index, clir::address_space::global_t));
                     },
                     [](auto const &) {}},
          *v.ty());
    if (m == nullptr) {
        throw compilation_error(
            v.loc(), status::internal_compiler_error,
            "dope_vector::from_value must only be called for memref or group type");
    }
    auto dv = dope_vector::from_memref_type(std::string(v.name()), *m, std::move(dt), declare);
    visit(overloaded{[&](memref_data_type const &) {},
                     [&](group_data_type const &g) {
                         if (is_dynamic_value(g.offset())) {
                             auto var = clir::var(
                                 (std::ostringstream{} << var_name(v.name()) << "_offset").str());
                             declare(to_clir_ty(scalar_type::index), var, type::offset, 0);
                             dv.offset(std::move(var));
                         } else {
                             dv.offset(g.offset());
                         }
                     },
                     [](auto const &) {}},
          *v.ty());
    return dv;
}

dope_vector dope_vector::from_memref_type(std::string const &prefix, memref_data_type const &m,
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

convert_to_opencl_pass::convert_to_opencl_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
    declared_vars_.push_back({});
}

auto convert_to_opencl_pass::get_dope_vector(value_node const &v) -> dope_vector & {
    auto dv = dope_vector_.find(std::bit_cast<std::uintptr_t>(&v));
    if (dv == dope_vector_.end()) {
        throw compilation_error(v.loc(), status::internal_compiler_error,
                                "Dope vector for value is missing");
    }
    return dv->second;
}

void convert_to_opencl_pass::set_dope_vector(value_node const &v, dope_vector dv) {
    uintptr_t u = std::bit_cast<uintptr_t>(&v);
    dope_vector_[u] = std::move(dv);
}

clir::var convert_to_opencl_pass::declare(value_node const &v) {
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

auto convert_to_opencl_pass::get_coopmatrix_type(value_node const &v) const
    -> const coopmatrix_data_type * {
    auto t = dyn_cast<coopmatrix_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_coopmatrix);
    }
    return t;
}

auto convert_to_opencl_pass::get_memref_type(value_node const &v) const
    -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

auto convert_to_opencl_pass::get_scalar_type(value_node const &v) -> scalar_type {
    auto st = dyn_cast<scalar_data_type>(v.ty());
    if (!st) {
        throw compilation_error(v.loc(), status::ir_expected_scalar);
    }
    return st->ty();
};

/* Data type nodes */
clir::data_type convert_to_opencl_pass::operator()(void_data_type const &) {
    return clir::builtin_type::void_t;
}
clir::data_type convert_to_opencl_pass::operator()(coopmatrix_data_type const &ct) {
    return array_of(to_clir_ty(ct.component_ty()), ct.length(core_cfg_.subgroup_size));
}
clir::data_type convert_to_opencl_pass::operator()(group_data_type const &g) {
    auto ptr_ty = visit(*this, *g.ty());
    ptr_ty = clir::visit(overloaded{[](clir::internal::pointer &t) {
                                        return clir::pointer_to(clir::pointer_to(
                                            t.ty(), clir::address_space::global_t));
                                    },
                                    [](auto &) { return clir::data_type{}; }},
                         *ptr_ty);
    if (!ptr_ty) {
        throw compilation_error(location{}, status::internal_compiler_error,
                                "Could not determine OpenCL type of group type");
    }
    return ptr_ty;
}
clir::data_type convert_to_opencl_pass::operator()(memref_data_type const &d) {
    return clir::pointer_to(to_clir_ty(d.element_ty(), to_clir_address_space(d.addrspace())));
}
clir::data_type convert_to_opencl_pass::operator()(scalar_data_type const &s) {
    return to_clir_ty(s.ty());
}

/* Value nodes */
auto convert_to_opencl_pass::val(value_node const &v) -> clir::expr {
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
std::vector<clir::stmt> convert_to_opencl_pass::operator()(alloca_inst const &a) {
    if (a.stack_ptr() < 0) {
        throw compilation_error(a.loc(), status::internal_compiler_error,
                                "Invalid stack_ptr in alloca. Did you run set_stack_ptrs?");
    }
    auto result_var = declare(*a.result());
    auto t = dyn_cast<memref_data_type>(a.result()->ty());
    if (t == nullptr) {
        throw compilation_error(a.loc(), status::ir_expected_memref);
    }
    auto ptr_ty = operator()(*t);
    auto result = declaration_assignment(ptr_ty, std::move(result_var),
                                         clir::cast(ptr_ty, stack_ + a.stack_ptr()));
    stack_high_water_mark_ = std::max(stack_high_water_mark_,
                                      static_cast<std::size_t>(a.stack_ptr()) + t->size_in_bytes());

    // no declarations are necessary as alloca only accepts fixed-size memrefs
    set_dope_vector(a.result(0),
                    dope_vector::from_value(*a.result(), [](clir::data_type, clir::var,
                                                            dope_vector::type, std::int64_t) {}));
    return {std::move(result)};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(axpby_inst const &inst) {
    auto at = get_memref_type(inst.A());
    auto bt = get_memref_type(inst.B());
    auto alpha_ty = get_scalar_type(inst.alpha());
    auto beta_ty = get_scalar_type(inst.beta());
    auto &adv = get_dope_vector(inst.A());
    auto &bdv = get_dope_vector(inst.B());

    auto pA = inst.tA() == transpose::T && at->dim() == 2 ? 1 : 0;

    auto alpha = val(inst.alpha());
    auto beta = val(inst.beta());
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
                    const auto a_scaled = multiply(alpha_ty, at->element_ty(), alpha, std::move(a));
                    store_helper(bb, inst.atomic(), b, bt->element_ty(),
                                 to_clir_address_space(bt->addrspace()), std::move(a_scaled),
                                 beta_ty, beta);
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

    auto A = val(inst.A());
    auto B = val(inst.B());
    if (bt->dim() == 0) {
        auto bb = clir::block_builder{};
        const auto a_scaled = multiply(alpha_ty, at->element_ty(), alpha, A[0]);
        store_helper(bb, inst.atomic(), B, bt->element_ty(), to_clir_address_space(bt->addrspace()),
                     std::move(a_scaled), beta_ty, std::move(beta));
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

std::vector<clir::stmt> convert_to_opencl_pass::operator()(barrier_inst const &b) {
    clir::expr fence = 0;
    if (b.has_fence(address_space::global)) {
        fence = fence | clir::cl_mem_fence_flags::CLK_GLOBAL_MEM_FENCE;
    }
    if (b.has_fence(address_space::local)) {
        fence = fence | clir::cl_mem_fence_flags::CLK_LOCAL_MEM_FENCE;
    }
    return {clir::expression_statement(
        clir::call_builtin(clir::builtin_function::barrier, {std::move(fence)}))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(arith_inst const &a) {
    auto const make = [](arithmetic op, clir::expr a, clir::expr b, scalar_type sty) -> clir::expr {
        switch (op) {
        case arithmetic::add:
            return std::move(a) + std::move(b);
        case arithmetic::sub:
            return std::move(a) - std::move(b);
        case arithmetic::mul:
            return multiply(sty, sty, std::move(a), std::move(b));
        case arithmetic::div:
            return divide(sty, sty, std::move(a), std::move(b));
        case arithmetic::rem:
            if (is_floating_type(sty)) {
                return clir::fmod(std::move(a), std::move(b));
            }
            return std::move(a) % std::move(b);
        case arithmetic::shl:
            return std::move(a) << std::move(b);
        case arithmetic::shr:
            return std::move(a) >> std::move(b);
        case arithmetic::and_:
            if (sty == scalar_type::i1) {
                return std::move(a) && std::move(b);
            }
            return std::move(a) & std::move(b);
        case arithmetic::or_:
            if (sty == scalar_type::i1) {
                return std::move(a) || std::move(b);
            }
            return std::move(a) | std::move(b);
        case arithmetic::xor_:
            return std::move(a) ^ std::move(b);
        }
        return {};
    };

    auto lhs = declare(a.result(0));
    auto lhs_ty = visit(*this, *a.result()->ty());
    auto av = val(a.a());
    auto bv = val(a.b());
    if (auto st = dyn_cast<scalar_data_type>(a.result(0).ty()); st) {
        auto op = make(a.operation(), av, bv, st->ty());
        return {declaration_assignment(std::move(lhs_ty), std::move(lhs), std::move(op))};
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(a.result(0).ty()); ct) {
        auto clinst = std::vector<clir::stmt>{};
        auto const len = ct->length(core_cfg_.subgroup_size);
        clinst.reserve(len + 1);
        clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
        const auto sty = ct->component_ty();
        for (std::int64_t i = 0; i < len; ++i) {
            auto op = make(a.operation(), av[i], bv[i], sty);
            clinst.emplace_back(expression_statement(assignment(lhs[i], std::move(op))));
        }
        return clinst;
    }
    throw compilation_error(a.loc(), status::ir_expected_coopmatrix_or_scalar);
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(arith_unary_inst const &a) {
    auto const make = [](arithmetic_unary op, clir::expr a, scalar_type sty) -> clir::expr {
        switch (op) {
        case arithmetic_unary::abs:
            if (is_complex_type(sty)) {
                return clir::call_builtin(clir::builtin_function::sqrt,
                                          {a.s(0) * a.s(0) + a.s(1) * a.s(1)});
            }
            if (is_floating_type(sty)) {
                return clir::call_builtin(clir::builtin_function::fabs, {std::move(a)});
            }
            return clir::call_builtin(clir::builtin_function::abs, {std::move(a)});
        case arithmetic_unary::neg:
            return -std::move(a);
        case arithmetic_unary::not_:
            if (sty == scalar_type::i1) {
                return !std::move(a);
            }
            return ~std::move(a);
        case arithmetic_unary::conj:
            return clir::init_vector(to_clir_ty(sty), {a.s(0), -a.s(1)});
        case arithmetic_unary::im:
            return std::move(a).s(1);
        case arithmetic_unary::re:
            return std::move(a).s(0);
        }
        return {};
    };

    auto lhs = declare(a.result(0));
    auto lhs_ty = visit(*this, *a.result()->ty());
    auto av = val(a.a());
    if (auto st = dyn_cast<scalar_data_type>(a.a().ty()); st) {
        auto op = make(a.operation(), av, st->ty());
        return {declaration_assignment(std::move(lhs_ty), std::move(lhs), std::move(op))};
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(a.a().ty()); ct) {
        auto clinst = std::vector<clir::stmt>{};
        auto const len = ct->length(core_cfg_.subgroup_size);
        clinst.reserve(len + 1);
        clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
        const auto sty = ct->component_ty();
        for (std::int64_t i = 0; i < len; ++i) {
            auto op = make(a.operation(), av[i], sty);
            clinst.emplace_back(expression_statement(assignment(lhs[i], std::move(op))));
        }
        return clinst;
    }
    throw compilation_error(a.loc(), status::ir_expected_coopmatrix_or_scalar);
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(cast_inst const &c) {
    auto const make = [](clir::expr a, scalar_type aty, scalar_type rty) -> clir::expr {
        if (is_complex_type(aty) && is_complex_type(rty)) {
            switch (rty) {
            case scalar_type::c32:
                return clir::call("convert_float2", {std::move(a)});
            case scalar_type::c64:
                return clir::call("convert_double2", {std::move(a)});
            default:
                throw status::internal_compiler_error;
            }
        } else if (is_complex_type(rty)) {
            return clir::init_vector(to_clir_ty(rty), {std::move(a), 0});
        }
        return cast(to_clir_ty(rty), std::move(a));
    };

    auto lhs = declare(c.result(0));
    auto lhs_ty = visit(*this, *c.result(0).ty());
    auto av = val(c.a());

    if (auto rt = dyn_cast<scalar_data_type>(c.result(0).ty()); rt) {
        auto aty = get_scalar_type(c.a());
        auto op = make(av, aty, rt->ty());
        return {declaration_assignment(std::move(lhs_ty), std::move(lhs), std::move(op))};
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(c.result(0).ty()); ct) {
        const auto rty = ct->component_ty();
        auto at = dyn_cast<coopmatrix_data_type>(c.a().ty());
        if (!at) {
            throw compilation_error(c.loc(), status::ir_expected_coopmatrix);
        }
        const auto aty = at->component_ty();

        auto clinst = std::vector<clir::stmt>{};
        auto const len = ct->length(core_cfg_.subgroup_size);
        clinst.reserve(len + 1);
        clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
        for (std::int64_t i = 0; i < len; ++i) {
            auto op = make(av[i], aty, rty);
            clinst.emplace_back(expression_statement(assignment(lhs[i], std::move(op))));
        }
        return clinst;
    }
    throw compilation_error(c.loc(), status::ir_expected_coopmatrix_or_scalar);
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(compare_inst const &c) {
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
                                   make(c.cond(), val(c.a()), val(c.b())))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(constant_inst const &c) {
    auto const get_rhs = [&c](scalar_type ty, short ty_bits) {
        return std::visit(overloaded{
                              [&](std::int64_t i) { return clir::expr(i, ty_bits); },
                              [&](double d) { return clir::expr(d, ty_bits); },
                              [&](std::complex<double> d) {
                                  return init_vector(to_clir_ty(ty),
                                                     {clir::expr(d.real(), ty_bits),
                                                      clir::expr(d.imag(), ty_bits)});
                              },
                          },
                          c.value());
    };
    auto lhs = declare(c.result(0));
    auto lhs_ty = visit(*this, *c.result()->ty());
    if (auto st = dyn_cast<scalar_data_type>(c.result(0).ty()); st) {
        auto ty_bits = static_cast<short>(size(st->ty()) * 8);
        return {
            declaration_assignment(std::move(lhs_ty), std::move(lhs), get_rhs(st->ty(), ty_bits))};
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(c.result(0).ty()); ct) {
        auto ty_bits = static_cast<short>(size(ct->component_ty()) * 8);
        auto rhs = get_rhs(ct->component_ty(), ty_bits);
        auto clinst = std::vector<clir::stmt>{};
        auto const len = ct->length(core_cfg_.subgroup_size);
        clinst.reserve(len + 1);
        clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
        for (std::int64_t i = 0; i < len; ++i) {
            clinst.emplace_back(expression_statement(assignment(lhs[i], rhs)));
        }
        return clinst;
    }
    throw compilation_error(c.loc(), status::ir_expected_coopmatrix_or_scalar);
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(cooperative_matrix_load_inst const &c) {
    auto lhs = declare(c.result(0));
    auto lhs_ty = visit(*this, *c.result(0).ty());
    auto ot = get_memref_type(c.operand());
    auto rt = get_coopmatrix_type(c.result(0));
    auto &odv = get_dope_vector(c.operand());

    const int rmode = rt->distributed_mode();
    const int omode = c.t() == transpose::T ? 1 - rmode : rmode;
    const bool check_m = c.checked() == checked_flag::both ||
                         (rmode == 0 && c.checked() == checked_flag::rows) ||
                         (rmode == 1 && c.checked() == checked_flag::cols);
    const bool check_k = c.checked() == checked_flag::both ||
                         (rmode == 1 && c.checked() == checked_flag::rows) ||
                         (rmode == 0 && c.checked() == checked_flag::cols);
    const bool enable_sub_group_reads =
        core_cfg_.block_read_write_supported && c.t() == transpose::N && ot->stride(omode) == 1;

    auto clinst = std::vector<clir::stmt>{};
    auto const len = rt->length(core_cfg_.subgroup_size);
    clinst.reserve(len + 5);
    clinst.emplace_back(declaration(std::move(lhs_ty), lhs));

    clir::expr pv[] = {val(c.pos0()), val(c.pos1())};
    auto pointer = clir::var{};
    clinst.emplace_back(
        declaration_assignment(visit(*this, *c.operand().ty()), pointer,
                               val(c.operand()) + pv[0] * odv.stride(0) + pv[1] * odv.stride(1)));
    clir::var rem[2] = {};
    if (check_m || check_k) {
        clinst.emplace_back(
            declaration_assignment(to_clir_ty(scalar_type::index), rem[0], odv.shape(0) - pv[0]));
        clinst.emplace_back(
            declaration_assignment(to_clir_ty(scalar_type::index), rem[1], odv.shape(1) - pv[1]));
    }

    const std::int64_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        auto row_in_bounds = clir::var{};
        if (check_m) {
            auto m = clir::get_sub_group_local_id() + block * core_cfg_.subgroup_size;
            clinst.emplace_back(declaration_assignment(to_clir_ty(scalar_type::i1), row_in_bounds,
                                                       m >= -pv[omode] && m < rem[omode]));
        }
        for (std::int64_t k = 0; k < rt->shape(1 - rmode); ++k) {
            auto col_cond = [&] { return k >= -pv[1 - omode] && k < rem[1 - omode]; };

            auto const store = [&](clir::expr rhs) -> clir::stmt {
                return expression_statement(
                    assignment(lhs[k + block * rt->shape(1 - rmode)], std::move(rhs)));
            };
            auto const remainder = rt->shape(rmode) - core_cfg_.subgroup_size * block;
            const bool needs_mask = remainder < core_cfg_.subgroup_size;
            if (enable_sub_group_reads && !needs_mask && !check_m) {
                auto rhs = sub_group_block_read_helper(
                    pointer + block * core_cfg_.subgroup_size + k * odv.stride(1), ot->element_ty(),
                    to_clir_address_space(ot->addrspace()));
                if (check_k) {
                    rhs = ternary_conditional(col_cond(), std::move(rhs), 0);
                }
                clinst.emplace_back(store(std::move(rhs)));
            } else {
                auto rhs = pointer[odv.stride(omode) * (clir::get_sub_group_local_id() +
                                                        block * core_cfg_.subgroup_size) +
                                   k * odv.stride(1 - omode)];
                clir::expr cond = {};
                if (check_m) {
                    cond = row_in_bounds;
                }
                if (check_k) {
                    cond = cond ? cond && col_cond() : col_cond();
                }
                if (needs_mask) {
                    auto mask_cond = clir::get_sub_group_local_id() < remainder;
                    cond = cond ? cond && mask_cond : mask_cond;
                }

                if (cond) {
                    rhs = ternary_conditional(cond, std::move(rhs), 0);
                }
                clinst.emplace_back(store(std::move(rhs)));
            }
        }
    }
    return clinst;
}
std::vector<clir::stmt>
convert_to_opencl_pass::operator()(cooperative_matrix_mul_add_inst const &c) {
    auto lhs = declare(c.result(0));
    auto lhs_ty = visit(*this, *c.result(0).ty());
    auto rt = get_coopmatrix_type(c.result(0));
    auto at = get_coopmatrix_type(c.a());
    auto bt = get_coopmatrix_type(c.b());
    auto ct = get_coopmatrix_type(c.c());
    auto av = val(c.a());
    auto bv = val(c.b());
    auto cv = val(c.c());

    const auto a_ty = at->component_ty();
    const auto b_ty = bt->component_ty();
    const auto c_ty = ct->component_ty();
    const auto r_ty = rt->component_ty();
    const bool use_double_buffering = is_complex_type(a_ty) && is_complex_type(b_ty);

    const std::int64_t M = rt->rows(), N = rt->cols(), K = at->cols();
    auto clinst = std::vector<clir::stmt>{};
    clinst.reserve(M * N + 2);
    clinst.emplace_back(declaration(lhs_ty, lhs));

    auto c_acc_im = clir::var{};
    if (use_double_buffering) {
        clinst.emplace_back(declaration(lhs_ty, c_acc_im));
    }

    const std::int64_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    const std::int64_t nbb = 4;
    for (std::int64_t m_block = 0; m_block < num_blocks; ++m_block) {
        for (std::int64_t nb = 0; nb < N; nb += nbb) {
            for (std::int64_t k = 0; k < K; ++k) {
                for (std::int64_t n = 0; n < nbb; ++n) {
                    if (nb + n < N) {
                        auto const n_block = (nb + n) / core_cfg_.subgroup_size;
                        auto const n_offset = (nb + n) % core_cfg_.subgroup_size;

                        auto a = av[k + m_block * K];
                        auto b = bv[k + n_block * K];
                        auto c_next = lhs[nb + n + m_block * N];
                        auto c = [&] {
                            if (k == 0) {
                                auto c = cv[nb + n + m_block * N];
                                if (c_ty != r_ty) {
                                    if (is_complex_type(r_ty) && !is_complex_type(c_ty)) {
                                        c = clir::init_vector(to_clir_ty(r_ty), {c, 0});
                                    } else if (r_ty != c_ty) {
                                        return clir::cast(to_clir_ty(r_ty), c);
                                    }
                                }
                                return c;
                            }
                            return c_next;
                        }();
                        const auto c_next_im = [&] { return c_acc_im[nb + n + m_block * N]; };
                        const auto c_im = [&] {
                            if (k == 0) {
                                return init_vector(to_clir_ty(r_ty), {0, 0});
                            }
                            return c_next_im();
                        };

                        auto const add = [&](auto a_ty, auto b_ty, auto c_ty, auto a, auto b,
                                             auto c, auto c_next) {
                            if (a_ty == b_ty && b_ty == c_ty) {
                                clinst.emplace_back(expression_statement(assignment(
                                    std::move(c_next),
                                    clir::fma(std::move(a), std::move(b), std::move(c)))));
                            } else {
                                clinst.emplace_back(expression_statement(
                                    assignment(std::move(c_next),
                                               std::move(c) + std::move(a) * std::move(b))));
                            }
                        };

                        if (is_complex_type(a_ty)) {
                            if (is_complex_type(b_ty)) {
                                auto b_bc_re = sub_group_broadcast(b.s(0), n_offset);
                                auto b_bc_im = sub_group_broadcast(b.s(1), n_offset);
                                add(a_ty, element_type(b_ty), r_ty, a, std::move(b_bc_re), c,
                                    c_next);
                                add(a_ty, element_type(b_ty), r_ty, a, std::move(b_bc_im), c_im(),
                                    c_next_im());
                            } else {
                                auto b_bc = sub_group_broadcast(b, n_offset);
                                add(a_ty, b_ty, r_ty, std::move(a), std::move(b_bc), c, c_next);
                            }
                        } else if (is_complex_type(b_ty)) {
                            auto b_bc_re = sub_group_broadcast(b.s(0), n_offset);
                            auto b_bc_im = sub_group_broadcast(b.s(1), n_offset);
                            add(a_ty, element_type(b_ty), r_ty, a, std::move(b_bc_re), c.s(0),
                                c_next.s(0));
                            add(a_ty, element_type(b_ty), r_ty, a, std::move(b_bc_im), c.s(1),
                                c_next.s(1));
                        } else {
                            auto b_bc = sub_group_broadcast(std::move(b), n_offset);
                            add(a_ty, b_ty, r_ty, std::move(a), std::move(b_bc), std::move(c),
                                std::move(c_next));
                        }
                    }
                }
            }
        }
    }
    if (use_double_buffering) {
        for (std::int64_t i = 0; i < rt->length(core_cfg_.subgroup_size); ++i) {
            clinst.emplace_back(expression_statement(
                add_into(lhs[i], clir::init_vector(to_clir_ty(r_ty),
                                                   {-c_acc_im[i].s(1), c_acc_im[i].s(0)}))));
        }
    }
    return clinst;
}
std::vector<clir::stmt> convert_to_opencl_pass::operator()(cooperative_matrix_scale_inst const &c) {
    auto lhs = declare(c.result(0));
    auto lhs_ty = visit(*this, *c.result()->ty());
    auto av = val(c.a());
    auto bv = val(c.b());
    auto at = get_scalar_type(c.a());
    auto bt = get_coopmatrix_type(c.b());

    auto clinst = std::vector<clir::stmt>{};
    auto const len = bt->length(core_cfg_.subgroup_size);
    clinst.reserve(len + 1);
    clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
    for (std::int64_t i = 0; i < len; ++i) {
        auto op = multiply(at, bt->component_ty(), av, bv[i]);
        clinst.emplace_back(expression_statement(assignment(lhs[i], std::move(op))));
    }
    return clinst;
}
std::vector<clir::stmt> convert_to_opencl_pass::operator()(cooperative_matrix_store_inst const &c) {
    auto ot = get_memref_type(c.operand());
    auto vt = get_coopmatrix_type(c.val());
    auto &odv = get_dope_vector(c.operand());
    auto valv = val(c.val());

    const int vmode = vt->distributed_mode();
    const int omode = vmode;
    const bool check_m = c.checked() == checked_flag::both ||
                         (vmode == 0 && c.checked() == checked_flag::rows) ||
                         (vmode == 1 && c.checked() == checked_flag::cols);
    const bool check_k = c.checked() == checked_flag::both ||
                         (vmode == 1 && c.checked() == checked_flag::rows) ||
                         (vmode == 0 && c.checked() == checked_flag::cols);

    auto clinst = std::vector<clir::stmt>{};
    auto const len = vt->length(core_cfg_.subgroup_size);
    clinst.reserve(len + 4);

    clir::expr pv[] = {val(c.pos0()), val(c.pos1())};
    auto base_pointer = clir::var{};
    clinst.emplace_back(
        declaration_assignment(visit(*this, *c.operand().ty()), base_pointer,
                               val(c.operand()) + pv[0] * odv.stride(0) + pv[1] * odv.stride(1)));
    clir::var rem[2] = {};
    if (check_m || check_k) {
        clinst.emplace_back(
            declaration_assignment(to_clir_ty(scalar_type::index), rem[0], odv.shape(0) - pv[0]));
        clinst.emplace_back(
            declaration_assignment(to_clir_ty(scalar_type::index), rem[1], odv.shape(1) - pv[1]));
    }

    const std::int64_t num_blocks = vt->num_blocks(core_cfg_.subgroup_size);
    auto const num_k = vt->shape(1 - vmode);
    auto store_block = std::vector<clir::stmt>{};
    store_block.reserve(num_k);
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        store_block.clear();
        for (std::int64_t k = 0; k < num_k; ++k) {
            auto const remainder = vt->shape(vmode) - core_cfg_.subgroup_size * block;
            const bool needs_mask = remainder < core_cfg_.subgroup_size;

            auto pointer = base_pointer +
                           odv.stride(omode) *
                               (clir::get_sub_group_local_id() + block * core_cfg_.subgroup_size) +
                           k * odv.stride(1 - omode);
            auto rhs = valv[k + block * vt->shape(1 - vmode)];
            clir::expr cond = {};
            if (check_k) {
                cond = k >= -pv[1 - omode] && k < rem[1 - omode];
            }
            if (needs_mask) {
                auto mask_cond = clir::get_sub_group_local_id() < remainder;
                cond = cond ? cond && mask_cond : mask_cond;
            }

            if (cond) {
                store_block.emplace_back(
                    clir::if_selection_builder(cond)
                        .then([&](clir::block_builder &bb) {
                            for (auto &s : atomic_store_helper_new(c.flag(), ot, std::move(pointer),
                                                                   std::move(rhs))) {
                                bb.add(std::move(s));
                            }
                        })
                        .get_product());
            } else {
                for (auto &s :
                     atomic_store_helper_new(c.flag(), ot, std::move(pointer), std::move(rhs))) {
                    store_block.emplace_back(std::move(s));
                }
            }
        }

        if (check_m) {
            auto m = clir::get_sub_group_local_id() + block * core_cfg_.subgroup_size;
            clinst.emplace_back(clir::if_selection_builder(m >= -pv[omode] && m < rem[omode])
                                    .then([&](clir::block_builder &bb) {
                                        for (auto &i : store_block) {
                                            bb.add(i);
                                        }
                                    })
                                    .get_product());
        } else {
            for (auto &i : store_block) {
                clinst.emplace_back(i);
            }
        }
    }

    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(expand_inst const &e) {
    auto result_var = declare(*e.result());
    auto m = get_memref_type(e.operand());
    auto &dv = get_dope_vector(e.operand());
    auto static_shape = e.static_expand_shape();
    auto dyn_shape = e.expand_shape();

    auto rhs = val(e.operand());
    auto clinst = std::vector<clir::stmt>{};
    clinst.emplace_back(
        clir::declaration_assignment(this->operator()(*m), std::move(result_var), std::move(rhs)));

    auto shape = std::vector<clir::expr>{};
    auto stride = std::vector<clir::expr>{};
    shape.reserve(m->dim() + static_shape.size() - 1);
    stride.reserve(m->dim() + static_shape.size() - 1);
    std::int64_t i = 0;
    for (; i < e.expanded_mode(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }

    auto eshape_cl = std::vector<clir::expr>{};
    eshape_cl.reserve(static_shape.size());
    int j = 0;
    for (auto &s : static_shape) {
        if (is_dynamic_value(s)) {
            eshape_cl.emplace_back(val(dyn_shape[j++]));
        } else {
            eshape_cl.emplace_back(clir::expr(s, static_cast<short>(size(scalar_type::index) * 8)));
        }
    }

    stride.push_back(m->stride(e.expanded_mode()));
    shape.push_back(eshape_cl[0]);
    for (std::size_t j = 1; j < eshape_cl.size(); ++j) {
        stride.push_back(stride.back() * shape.back());
        shape.push_back(eshape_cl[j]);
    }
    for (i = e.expanded_mode() + 1; i < m->dim(); ++i) {
        shape.push_back(dv.shape(i));
        stride.push_back(dv.stride(i));
    }

    set_dope_vector(e.result(0),
                    dope_vector::from_value(*e.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride[j] : shape[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}
std::vector<clir::stmt> convert_to_opencl_pass::operator()(fuse_inst const &f) {
    auto result_var = declare(*f.result());
    auto m = get_memref_type(f.operand());
    auto &dv = get_dope_vector(f.operand());

    auto rhs = val(f.operand());
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

    set_dope_vector(*f.result(),
                    dope_vector::from_value(*f.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride[j] : shape[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(load_inst const &e) {
    auto rhs = val(e.operand());

    auto clinst = std::vector<clir::stmt>{};

    visit(overloaded{[&](group_data_type const &) {
                         if (e.index_list().size() != 1) {
                             throw compilation_error(e.loc(), status::ir_invalid_number_of_indices);
                         }

                         auto idx = val(e.index_list().front());
                         rhs = rhs + idx;

                         auto &dv = get_dope_vector(e.operand());
                         rhs = clir::dereference(std::move(rhs)) + dv.offset();

                         set_dope_vector(
                             *e.result(),
                             dope_vector::from_value(
                                 *e.result(), [&](clir::data_type a, clir::var b,
                                                  dope_vector::type t, std::int64_t j) {
                                     auto init = t == dope_vector::type::stride ? dv.stride(j)
                                                                                : dv.shape(j);
                                     clinst.emplace_back(clir::declaration_assignment(
                                         std::move(a), std::move(b), std::move(init)[idx]));
                                 }));
                     },
                     [&](memref_data_type const &m) {
                         if (static_cast<std::int64_t>(e.index_list().size()) != m.dim()) {
                             throw compilation_error(e.loc(), status::ir_invalid_number_of_indices);
                         }
                         auto &dv = get_dope_vector(e.operand());
                         for (std::int64_t i = 0; i < m.dim(); ++i) {
                             rhs = rhs + val(e.index_list()[i]) * dv.stride(i);
                         }
                         rhs = clir::dereference(std::move(rhs));
                     },
                     [&e](auto const &) {
                         throw compilation_error(e.loc(), status::ir_expected_memref_or_group);
                     }},
          *e.operand().ty());

    auto lhs = declare(*e.result());
    auto result_type = e.result()->ty();
    if (result_type == nullptr) {
        throw compilation_error(e.loc(), status::internal_compiler_error, "Expected type");
    }

    clinst.emplace(clinst.begin(), declaration_assignment(visit(*this, *result_type),
                                                          std::move(lhs), std::move(rhs)));

    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(group_id_inst const &g) {
    auto rhs = clir::get_global_id(2);
    auto lhs = declare(*g.result());
    return {
        declaration_assignment(visit(*this, *g.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(group_size_inst const &g) {
    auto rhs = clir::get_global_size(2);
    auto lhs = declare(*g.result());
    return {
        declaration_assignment(visit(*this, *g.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(lifetime_stop_inst const &) {
    return {};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(gemm_inst const &g) {
    auto a = get_memref_type(g.A());
    auto b = get_memref_type(g.B());
    auto c = get_memref_type(g.C());
    auto &adv = get_dope_vector(g.A());
    auto &bdv = get_dope_vector(g.B());
    auto &cdv = get_dope_vector(g.C());

    auto const M = c->shape(0);
    auto const N = c->shape(1);
    auto const ak = g.tA() == transpose::T ? 0 : 1;
    auto const K = a->shape(ak);

    auto gemm_ty = gemm_scalar_type{get_scalar_type(g.alpha()), a->element_ty(), b->element_ty(),
                                    get_scalar_type(g.beta()), c->element_ty()};
    auto cfg = gemm_configuration{std::move(gemm_ty),
                                  g.tA(),
                                  g.tB(),
                                  M,
                                  N,
                                  K,
                                  {a->stride(0), a->stride(1)},
                                  {b->stride(0), b->stride(1)},
                                  {c->stride(0), c->stride(1)},
                                  std::nullopt,
                                  std::nullopt,
                                  g.atomic()};
    auto name = cfg.identifier();
    int name_counter = 0;
    while (reserved_names_.find(name) != reserved_names_.end()) {
        name = cfg.identifier("gemm" + std::to_string(++name_counter));
    }
    if (has_gemm_.find(name) == has_gemm_.end()) {
        auto f = generate_gemm(cfg, tiling_, core_cfg_, name, to_clir_address_space(a->addrspace()),
                               to_clir_address_space(b->addrspace()),
                               to_clir_address_space(c->addrspace()));
        prog_builder_.add(std::move(f));
    }
    has_gemm_.emplace(name);
    return {clir::expression_statement(clir::call(
        std::move(name), {cdv.shape(0), cdv.shape(1), adv.shape(ak), val(g.alpha()), val(g.A()),
                          adv.stride(0), adv.stride(1), val(g.B()), bdv.stride(0), bdv.stride(1),
                          val(g.beta()), val(g.C()), cdv.stride(0), cdv.stride(1)}))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(gemv_inst const &g) {
    auto a = get_memref_type(g.A());
    auto b = get_memref_type(g.B());
    auto c = get_memref_type(g.C());
    auto &adv = get_dope_vector(g.A());
    auto &bdv = get_dope_vector(g.B());
    auto &cdv = get_dope_vector(g.C());

    auto const M = c->shape(0);
    auto const ak = g.tA() == transpose::T ? 0 : 1;
    auto const K = a->shape(ak);
    constexpr auto N = 1;

    auto gemm_ty = gemm_scalar_type{get_scalar_type(g.alpha()), a->element_ty(), b->element_ty(),
                                    get_scalar_type(g.beta()), c->element_ty()};
    auto cfg = gemm_configuration{std::move(gemm_ty),
                                  g.tA(),
                                  transpose::N,
                                  M,
                                  N,
                                  K,
                                  {a->stride(0), a->stride(1)},
                                  {b->stride(0), 0},
                                  {c->stride(0), 0},
                                  std::nullopt,
                                  std::nullopt,
                                  g.atomic()};
    auto name = cfg.identifier("gemv");
    int name_counter = 0;
    while (reserved_names_.find(name) != reserved_names_.end()) {
        name = cfg.identifier("gemv" + std::to_string(++name_counter));
    }
    if (has_gemm_.find(name) == has_gemm_.end()) {
        auto f = generate_gemm(cfg, tiling_, core_cfg_, name, to_clir_address_space(a->addrspace()),
                               to_clir_address_space(b->addrspace()),
                               to_clir_address_space(c->addrspace()));
        prog_builder_.add(std::move(f));
    }
    has_gemm_.emplace(name);
    return {clir::expression_statement(
        clir::call(std::move(name), {cdv.shape(0), 1, adv.shape(ak), val(g.alpha()), val(g.A()),
                                     adv.stride(0), adv.stride(1), val(g.B()), bdv.stride(0), 0,
                                     val(g.beta()), val(g.C()), cdv.stride(0), 0}))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(ger_inst const &g) {
    auto at = get_memref_type(g.A());
    auto bt = get_memref_type(g.B());
    auto ct = get_memref_type(g.C());
    auto &adv = get_dope_vector(g.A());
    auto &bdv = get_dope_vector(g.B());
    auto &cdv = get_dope_vector(g.C());

    auto alpha = val(g.alpha());
    auto beta = val(g.beta());
    auto alpha_ty = get_scalar_type(g.alpha());
    auto beta_ty = get_scalar_type(g.beta());

    auto A = val(g.A());
    auto B = val(g.B());
    auto C = val(g.C());

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
                           auto b = bb.declare_assign(to_clir_ty(bt->element_ty()), "b",
                                                      B[(block + n) * bdv.stride(0)]);
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
                                       auto ab = bb.declare_assign(
                                           to_clir_ty(ct->element_ty()), "ab",
                                           multiply(at->element_ty(), bt->element_ty(),
                                                    std::move(a), b));
                                       const auto ab_scaled = multiply(alpha_ty, ct->element_ty(),
                                                                       alpha, std::move(ab));
                                       store_helper(bb, g.atomic(), c, ct->element_ty(),
                                                    to_clir_address_space(ct->addrspace()),
                                                    std::move(ab_scaled), beta_ty, beta);
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

std::vector<clir::stmt> convert_to_opencl_pass::operator()(for_inst const &in) {
    auto clinst = std::vector<clir::stmt>{};

    yielded_vars_.push_back(std::vector<clir::var>{});
    for (std::int64_t i = 0; i < in.num_results(); ++i) {
        auto lhs_ty = visit(*this, *in.result(i).ty());
        auto lhs = declare(in.result(i));

        // Link the iteration variable to the result variable
        uintptr_t u = std::bit_cast<uintptr_t>(&in.result(i));
        uintptr_t v = std::bit_cast<uintptr_t>(&in.iter_arg(i));
        declared_vars_.back()[v] = declared_vars_.back()[u];

        auto iinit = val(in.iter_init(i));
        if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(i).ty()); ct) {
            clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
            auto const len = ct->length(core_cfg_.subgroup_size);
            for (std::int64_t j = 0; j < len; ++j) {
                clinst.emplace_back(expression_statement(assignment(lhs[j], iinit[j])));
            }
        } else {
            clinst.emplace_back(clir::declaration_assignment(lhs_ty, lhs, iinit));
        }
        yielded_vars_.back().emplace_back(std::move(lhs));
    }

    auto lv = declare(in.loop_var());
    auto lv_ty = visit(*this, *in.loop_var().ty());
    auto start = clir::declaration_assignment(std::move(lv_ty), lv, val(in.from()));
    auto condition = lv < val(in.to());
    auto step = in.has_step() ? clir::add_into(lv, val(in.step())) : ++lv;
    auto body = run_on_region(in.body());
    clinst.emplace_back(clir::stmt(std::make_shared<clir::internal::for_loop>(
        std::move(start), std::move(condition), std::move(step), std::move(body))));

    yielded_vars_.pop_back();
    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(foreach_inst const &p) {
    auto lv = declare(p.loop_var());
    auto lv_ty = visit(*this, *p.loop_var().ty());
    auto from = val(p.from());
    auto to = val(p.to());
    auto bb = clir::block_builder{};
    auto sg = bb.declare_assign(clir::generic_uint(), "sg", clir::get_sub_group_id());
    auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
    auto trip_count = bb.declare_assign(lv_ty, "trip_count", to - from);
    tile_loop_by_sgs(
        bb, trip_count, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(),
        std::move(sg), [&](clir::block_builder &bb, clir::expr block, bool, clir::expr) {
            bb.add(clir::declaration_assignment(lv_ty, lv, std::move(block) + m + from));
            bb.add(run_on_region(p.body()));
        });
    return {bb.get_product()};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(hadamard_inst const &g) {
    auto at = get_memref_type(g.A());
    auto bt = get_memref_type(g.B());
    auto ct = get_memref_type(g.C());
    auto &adv = get_dope_vector(g.A());
    auto &bdv = get_dope_vector(g.B());
    auto &cdv = get_dope_vector(g.C());

    auto alpha = val(g.alpha());
    auto beta = val(g.beta());
    auto alpha_ty = get_scalar_type(g.alpha());
    auto beta_ty = get_scalar_type(g.beta());

    auto A = val(g.A());
    auto B = val(g.B());
    auto C = val(g.C());

    auto bb = clir::block_builder{};
    auto sg = bb.declare_assign(clir::generic_uint(), "sg", clir::get_sub_group_id());
    auto m = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
    tile_loop_by_sgs(
        bb, cdv.shape(0), core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(),
        std::move(sg),
        [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
            clir::expr inner_trip_count) {
            auto const inner_loop = [&](clir::block_builder &bb) {
                auto b = B[(block + m) * bdv.stride(0)];
                auto a = A[(block + m) * adv.stride(0)];

                auto c = bb.declare_assign((*this)(*ct), "c", C + (block + m) * cdv.stride(0));
                auto ab = bb.declare_assign(
                    to_clir_ty(ct->element_ty()), "ab",
                    multiply(at->element_ty(), bt->element_ty(), std::move(a), b));
                const auto ab_scaled = multiply(alpha_ty, ct->element_ty(), alpha, std::move(ab));
                store_helper(bb, g.atomic(), c, ct->element_ty(),
                             to_clir_address_space(ct->addrspace()), std::move(ab_scaled), beta_ty,
                             beta);
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

std::vector<clir::stmt> convert_to_opencl_pass::operator()(if_inst const &in) {
    auto clinst = std::vector<clir::stmt>{};
    yielded_vars_.push_back(std::vector<clir::var>{});
    for (auto const &r : in.results()) {
        auto v = declare(r);
        clinst.emplace_back(clir::declaration(visit(*this, *r.ty()), v));
        yielded_vars_.back().emplace_back(std::move(v));
    }
    auto ib = clir::if_selection_builder(val(in.condition()));
    ib.set_then(run_on_region(in.then()));
    if (!in.is_otherwise_empty()) {
        ib.set_otherwise(run_on_region(in.otherwise()));
    }
    yielded_vars_.pop_back();
    clinst.emplace_back(ib.get_product());
    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(num_subgroups_inst const &sg) {
    auto rhs = clir::get_num_sub_groups();
    auto lhs = declare(*sg.result());
    return {
        declaration_assignment(visit(*this, *sg.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(parallel_inst const &p) {
    return {run_on_region(p.body())};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(size_inst const &s) {
    auto v = declare(*s.result());
    auto &dv = get_dope_vector(s.operand());

    return {clir::declaration_assignment(visit(*this, *s.result()->ty()), std::move(v),
                                         dv.shape(s.mode()))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(subgroup_id_inst const &sg) {
    auto rhs = clir::get_sub_group_id();
    auto lhs = declare(*sg.result());
    return {
        declaration_assignment(visit(*this, *sg.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(subgroup_local_id_inst const &sg) {
    auto rhs = clir::get_sub_group_local_id();
    auto lhs = declare(*sg.result());
    return {
        declaration_assignment(visit(*this, *sg.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(subgroup_size_inst const &sg) {
    auto rhs = clir::get_sub_group_size();
    auto lhs = declare(*sg.result());
    return {
        declaration_assignment(visit(*this, *sg.result()->ty()), std::move(lhs), std::move(rhs))};
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(subview_inst const &s) {
    auto result_var = declare(*s.result());
    auto t = get_memref_type(s.operand());
    auto &dv = get_dope_vector(s.operand());

    auto rhs = val(s.operand());
    int j = 0;
    auto shape_out = std::vector<clir::expr>{};
    auto stride_out = std::vector<clir::expr>{};
    shape_out.reserve(t->dim());
    stride_out.reserve(t->dim());
    auto dyn_offsets = s.offsets();
    auto dyn_sizes = s.sizes();
    for (std::int64_t i = 0, joffset = 0, jsize = 0; i < t->dim(); ++i) {
        auto offset = s.static_offsets()[i];

        auto offset_cl = clir::expr{};
        if (is_dynamic_value(offset)) {
            offset_cl = val(dyn_offsets[joffset++]);
        } else {
            offset_cl =
                clir::expr(offset, static_cast<short>(tinytc::size(scalar_type::index) * 8));
        }
        rhs = rhs + offset_cl * dv.stride(j);

        auto size = s.static_sizes()[i];
        if (size > 0 || is_dynamic_value(size)) {
            auto size_cl = clir::expr{};
            if (is_dynamic_value(size)) {
                size_cl = val(dyn_sizes[jsize++]);
            } else {
                size_cl =
                    clir::expr(size, static_cast<short>(tinytc::size(scalar_type::index) * 8));
            }
            shape_out.emplace_back(size_cl);
            stride_out.emplace_back(dv.stride(j));
        }

        ++j;
    }

    auto clinst = std::vector<clir::stmt>{};
    clinst.emplace_back(
        clir::declaration_assignment(this->operator()(*t), std::move(result_var), std::move(rhs)));

    set_dope_vector(*s.result(),
                    dope_vector::from_value(*s.result(), [&](clir::data_type a, clir::var b,
                                                             dope_vector::type t, std::int64_t j) {
                        auto init = t == dope_vector::type::stride ? stride_out[j] : shape_out[j];
                        clinst.emplace_back(clir::declaration_assignment(std::move(a), std::move(b),
                                                                         std::move(init)));
                    }));
    return clinst;
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(store_inst const &s) {
    auto ot = get_memref_type(s.operand());

    if (static_cast<std::int64_t>(s.index_list().size()) != ot->dim()) {
        throw compilation_error(s.loc(), status::ir_invalid_number_of_indices);
    }

    auto lhs = val(s.operand());
    auto &dv = get_dope_vector(s.operand());
    for (std::int64_t i = 0; i < ot->dim(); ++i) {
        lhs = lhs + val(s.index_list()[i]) * dv.stride(i);
    }

    auto rhs = val(s.val());
    return atomic_store_helper_new(s.flag(), ot, std::move(lhs), std::move(rhs));
}

std::vector<clir::stmt> convert_to_opencl_pass::operator()(sum_inst const &inst) {
    auto at = get_memref_type(inst.A());
    auto bt = get_memref_type(inst.B());
    auto &adv = get_dope_vector(inst.A());
    auto &bdv = get_dope_vector(inst.B());

    auto alpha = val(inst.alpha());
    auto beta = val(inst.beta());
    auto alpha_ty = get_scalar_type(inst.alpha());
    auto beta_ty = get_scalar_type(inst.beta());

    auto zero = clir::expr(0.0, static_cast<short>(size(at->element_ty()) * 8));

    auto A = val(inst.A());
    auto B = val(inst.B());
    auto bb = clir::block_builder{};
    auto acc = bb.declare_assign(to_clir_ty(at->element_ty()), "acc", std::move(zero));
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
        auto sum = bb.declare_assign(to_clir_ty(bt->element_ty()), "sum",
                                     clir::work_group_reduce_add(acc));
        bb.add(clir::if_selection_builder(clir::get_sub_group_id() == 0 &&
                                          clir::get_sub_group_local_id() == 0)
                   .then([&](clir::block_builder &bb) {
                       const auto sum_scaled = multiply(alpha_ty, at->element_ty(), alpha, sum);
                       store_helper(bb, inst.atomic(), B, bt->element_ty(),
                                    to_clir_address_space(bt->addrspace()), std::move(sum_scaled),
                                    beta_ty, beta);
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
                    const auto sum_scaled = multiply(alpha_ty, at->element_ty(), alpha, acc);
                    store_helper(bb, inst.atomic(), b, bt->element_ty(),
                                 to_clir_address_space(bt->addrspace()), std::move(sum_scaled),
                                 beta_ty, beta);
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

std::vector<clir::stmt> convert_to_opencl_pass::operator()(yield_inst const &in) {
    if (yielded_vars_.empty()) {
        throw compilation_error(in.loc(), status::ir_unexpected_yield);
    }
    if (static_cast<std::int64_t>(yielded_vars_.back().size()) != in.num_operands()) {
        throw compilation_error(in.loc(), status::ir_yield_mismatch);
    }
    std::vector<clir::stmt> clinst;
    for (std::int64_t i = 0; i < in.num_operands(); ++i) {
        auto &yielded_var = yielded_vars_.back()[i];
        auto ov = val(in.op(i));
        if (auto ct = dyn_cast<coopmatrix_data_type>(in.op(i).ty()); ct) {
            auto const len = ct->length(core_cfg_.subgroup_size);
            for (std::int64_t j = 0; j < len; ++j) {
                clinst.push_back(expression_statement(assignment(yielded_var[j], ov[j])));
            }
        } else {
            clinst.push_back(expression_statement(assignment(yielded_var, ov)));
        }
    }
    return clinst;
}

/* Region nodes */
clir::stmt convert_to_opencl_pass::run_on_region(region_node const &reg) {
    declared_vars_.push_back({});
    auto bb = clir::block_builder{};
    for (auto &s : reg.insts()) {
        for (auto &cs : visit(*this, s)) {
            bb.add(cs);
        }
    }
    declared_vars_.pop_back();
    return bb.get_product();
}

/* Function nodes */
auto convert_to_opencl_pass::run_on_function(function_node const &fn) -> clir::func {
    stack_high_water_mark_ = 0;
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

    // Create prototype
    auto fb = clir::kernel_builder(std::string(fn.name()));
    for (auto const &v : fn.params()) {
        fb.argument(visit(*this, *v.ty()), declare(v));
        auto dv = visit(
            overloaded{[&fb, &v](memref_data_type const &) -> std::optional<dope_vector> {
                           return std::make_optional(dope_vector::from_value(
                               v, [&](clir::data_type a, clir::var b, dope_vector::type,
                                      std::int64_t) { fb.argument(std::move(a), std::move(b)); }));
                       },
                       [&fb, &v](group_data_type const &) -> std::optional<dope_vector> {
                           return std::make_optional(dope_vector::from_value(
                               v, [&](clir::data_type a, clir::var b, dope_vector::type,
                                      std::int64_t) { fb.argument(std::move(a), std::move(b)); }));
                       },
                       [](auto const &) { return std::nullopt; }},
            *v.ty());
        if (dv) {
            set_dope_vector(v, std::move(*dv));
        }
    }

    fb.attribute(clir::reqd_work_group_size(work_group_size[0], work_group_size[1], 1));
    fb.attribute(clir::intel_reqd_sub_group_size(subgroup_size));

    auto body = run_on_region(fn.body());

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
    return clir::function(fb.get_product(), std::move(body));
}

/* Program nodes */
auto convert_to_opencl_pass::run_on_program(program_node const &p) -> clir::prog {
    reserved_names_.clear();
    for (auto const &fn : p) {
        reserved_names_.insert(std::string(fn.name()));
    }

    prog_builder_ = clir::program_builder{};
    for (auto const &fn : p) {
        prog_builder_.add(run_on_function(fn));
    }
    return prog_builder_.get_product();
}

} // namespace tinytc
