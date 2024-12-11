// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/lower_coopmatrix_aux.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class coopmatrix_code_generator {
  public:
    coopmatrix_code_generator(core_config core_cfg, region_node &reg)
        : core_cfg_{std::move(core_cfg)}, bb_{&reg} {}
    // Returns true if instruction was replaced
    bool operator()(inst_node &in);
    bool operator()(arith_inst &in);
    bool operator()(arith_unary_inst &in);
    bool operator()(cast_inst &in);
    bool operator()(constant_inst &in);
    bool operator()(cooperative_matrix_load_inst &in);
    bool operator()(cooperative_matrix_mul_add_inst &in);
    bool operator()(cooperative_matrix_scale_inst &in);
    bool operator()(cooperative_matrix_store_inst &in);
    bool operator()(for_inst &in);
    bool operator()(if_inst &in);
    bool operator()(yield_inst &in);

    void run_on_region(region_node &reg);

  private:
    void declare(tinytc_value const &v, std::vector<value> vals);
    template <typename OldRange, typename NewRange>
    void declare_copy_expand(OldRange old_range, NewRange new_range) {
        auto pos = new_range.begin();
        for (auto &res : old_range) {
            if (auto ct = dyn_cast<coopmatrix_data_type>(res.ty()); ct) {
                auto decl = std::vector<value>(ct->length(core_cfg_.subgroup_size));
                for (auto &d : decl) {
                    d = &*pos++;
                }
                declare(res, std::move(decl));
            } else {
                ++pos;
            }
        }
    }
    auto vals(location const &loc, tinytc_value const &v,
              std::int64_t expected_size) -> std::vector<value> &;
    template <typename Range>
    auto vals_copy_expand(location const &loc, Range range) -> std::vector<value> {
        auto expanded_vals = std::vector<value>{};
        for (auto &v : range) {
            if (auto ct = dyn_cast<coopmatrix_data_type>(v.ty()); ct) {
                for (auto &vv : vals(loc, v, ct->length(core_cfg_.subgroup_size))) {
                    expanded_vals.emplace_back(vv);
                }
            } else {
                expanded_vals.emplace_back(&v);
            }
        }
        return expanded_vals;
    }

    core_config core_cfg_;
    region_builder bb_;
    std::unordered_map<const_tinytc_value_t, std::vector<value>> vals_;
};

void coopmatrix_code_generator::declare(value_node const &v, std::vector<value> vals) {
    vals_[&v] = std::move(vals);
}

auto coopmatrix_code_generator::vals(location const &loc, value_node const &v,
                                     std::int64_t expected_size) -> std::vector<value> & {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        if (static_cast<std::int64_t>(it->second.size()) == expected_size) {
            return it->second;
        }
        throw compilation_error(loc, {&v}, status::internal_compiler_error,
                                (std::ostringstream{} << "Lower coopmatrix: got "
                                                      << it->second.size()
                                                      << " values but expected " << expected_size)
                                    .str());
    }
    throw compilation_error(loc, {&v}, status::internal_compiler_error,
                            "Lower coopmatrix: undefined value");
}

bool coopmatrix_code_generator::operator()(inst_node &) { return false; }

bool coopmatrix_code_generator::operator()(arith_inst &in) {
    if (auto rt = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); rt) {
        auto &av = vals(in.loc(), in.a(), rt->length(core_cfg_.subgroup_size));
        auto &bv = vals(in.loc(), in.b(), rt->length(core_cfg_.subgroup_size));
        auto results = std::vector<value>{};
        results.reserve(av.size());
        for (auto a = av.begin(), b = bv.begin(); a != av.end() && b != bv.end(); ++a, ++b) {
            results.emplace_back(bb_.add(make_arith(in.operation(), *a, *b, rt->ty(), in.loc())));
        }
        declare(in.result(0), std::move(results));
        return true;
    }
    return false;
}
bool coopmatrix_code_generator::operator()(arith_unary_inst &in) {
    if (auto rt = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); rt) {
        auto &av = vals(in.loc(), in.a(), rt->length(core_cfg_.subgroup_size));
        auto results = std::vector<value>{};
        results.reserve(av.size());
        for (auto &a : av) {
            results.emplace_back(bb_.add(make_arith(in.operation(), a, rt->ty(), in.loc())));
        }
        declare(in.result(0), std::move(results));
        return true;
    }
    return false;
}
bool coopmatrix_code_generator::operator()(cast_inst &in) {
    if (auto rt = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); rt) {
        auto &av = vals(in.loc(), in.a(), rt->length(core_cfg_.subgroup_size));
        auto results = std::vector<value>{};
        results.reserve(av.size());
        for (auto &a : av) {
            results.emplace_back(bb_.add(make_cast(a, rt->ty(), in.loc())));
        }
        declare(in.result(0), std::move(results));
        return true;
    }
    return false;
}
bool coopmatrix_code_generator::operator()(constant_inst &in) {
    if (auto rt = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); rt) {
        auto size = rt->length(core_cfg_.subgroup_size);
        auto cst_in = std::make_unique<constant_inst>(in.value(), rt->ty(), in.loc());
        auto cst_val = bb_.add(inst{cst_in.release()});
        declare(in.result(0), std::vector<value>(size, cst_val));
        return true;
    }
    return false;
}

/**
 * For coopmatrix emulation, we have
 *
 * cooperative_matrix_load.trans_b.checked_b %M[%p0,%p1] : coopmatrix<SxXxY,matrix_b> ==
 * cooperative_matrix_load.trans_a.checked_a %M[%p0,%p1] : coopmatrix<SxYxX,matrix_a>
 *
 * where
 *
 * trans_b = { transpose::T if trans_a == transpose::N
 *           { transpose::N else
 * checked_b = { checked_flag::cols if checked_a == checked_flag::rows
 *             { checked_flag::rows if checked_a == checked_flag::cols
 *             { checked            else
 *
 */
bool coopmatrix_code_generator::operator()(cooperative_matrix_load_inst &in) {
    auto ctx = in.operand().context();
    auto bool_ty = boolean_data_type::get(ctx);
    auto index_ty = scalar_data_type::get(ctx, scalar_type::index);
    auto i32_ty = scalar_data_type::get(ctx, scalar_type::i32);
    // auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));

    const auto checked = normalize_checked_flag(in.checked(), rt->use());
    const auto shape = normalize_shape(rt->shape(), rt->use());
    const auto trans = normalize_transpose(in.t(), rt->use());

    const int omode = trans == transpose::T ? 1 : 0;
    const bool check_m = checked == checked_flag::both || checked == checked_flag::rows;
    const bool check_k = checked == checked_flag::both || checked == checked_flag::cols;
    // const std::int32_t alignment = std::max(in.align(), ot->alignment());

    auto tmp = bb_.add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
    auto subgroup_local_id = bb_.add(make_cast(tmp, index_ty, in.loc()));

    auto loaded_vals = std::vector<value>{};
    loaded_vals.reserve(rt->length(core_cfg_.subgroup_size));
    auto pos = std::array<value, 2u>{&in.pos0(), &in.pos1()};
    const std::int64_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        auto check_gen = check_condition_generator(&in.operand(), pos);
        auto fibre =
            get_matrix_fibre(bb_, &in.operand(), pos, omode, shape, subgroup_local_id, in.loc());
        auto const load_block = [&](auto &bb) -> std::vector<value> {
            auto loaded_vals = std::vector<value>{};
            loaded_vals.reserve(shape[1]);
            for (std::int64_t k = 0; k < shape[1]; ++k) {
                auto cst_k = bb_.add(make_constant(k, index_ty, in.loc()));
                auto const load_val = [&](auto &bb) {
                    return bb.add(make_load(fibre, {cst_k}, in.align(), rt->ty(), in.loc()));
                };
                if (check_k) {
                    auto cond = check_gen(bb, cst_k, 1 - omode, in.loc());
                    loaded_vals.emplace_back(
                        make_conditional_execution(bb, cond, load_val, rt->ty(), in.loc()));
                } else {
                    loaded_vals.emplace_back(load_val(bb));
                }
            }
            return loaded_vals;
        };
        if (check_m) {
            auto cond = check_gen(bb_, subgroup_local_id, omode, in.loc());
            if (shape[0] < core_cfg_.subgroup_size) {
                auto csgs = bb_.add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
                auto check3 = bb_.add(
                    make_cmp(cmp_condition::lt, subgroup_local_id, csgs, bool_ty, in.loc()));
                cond = bb_.add(make_arith(arithmetic::and_, cond, check3, bool_ty, in.loc()));
            }
            auto loaded_block =
                make_conditional_execution(bb_, cond, load_block, shape[1], rt->ty(), in.loc());
            loaded_vals.insert(loaded_vals.end(), loaded_block.begin(), loaded_block.end());
        } else {
            auto loaded_block = load_block(bb_);
            loaded_vals.insert(loaded_vals.end(), loaded_block.begin(), loaded_block.end());
        }
        if (block + 1 < num_blocks) {
            auto csgs = bb_.add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
            pos[omode] = bb_.add(make_arith(arithmetic::add, pos[omode], csgs, index_ty, in.loc()));
        }
    }
    declare(in.result(0), std::move(loaded_vals));
    return true;
}

bool coopmatrix_code_generator::operator()(cooperative_matrix_mul_add_inst &in) {
    auto ctx = in.a().context();
    auto i32_ty = scalar_data_type::get(ctx, scalar_type::i32);
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result(0));

    auto &av = vals(in.loc(), in.a(), at->length(core_cfg_.subgroup_size));
    auto &bv = vals(in.loc(), in.b(), bt->length(core_cfg_.subgroup_size));
    auto &cv = vals(in.loc(), in.c(), ct->length(core_cfg_.subgroup_size));

    const auto a_ty = at->component_ty();
    const auto b_ty = bt->component_ty();
    const auto scalar_b_element_ty = scalar_data_type::get(ctx, component_type(b_ty));
    const auto c_ty = ct->component_ty();
    const auto r_ty = rt->component_ty();
    const bool a_and_b_complex = is_complex_type(a_ty) && is_complex_type(b_ty);

    const std::int32_t N = rt->cols(), K = at->cols();

    const std::int32_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    constexpr std::int32_t nbb = 4;

    auto result = std::vector<value>(cv.begin(), cv.end());
    auto result_im =
        a_and_b_complex
            ? std::vector<value>(cv.size(), bb_.add(make_constant_zero(ct->ty(), in.loc())))
            : std::vector<value>{};
    for (std::int32_t m_block = 0; m_block < num_blocks; ++m_block) {
        for (std::int32_t nb = 0; nb < N; nb += nbb) {
            for (std::int32_t k = 0; k < K; ++k) {
                for (std::int32_t n = 0; n < nbb; ++n) {
                    if (nb + n < N) {
                        auto const n_block = (nb + n) / core_cfg_.subgroup_size;
                        auto n_offset = bb_.add(
                            make_constant((nb + n) % core_cfg_.subgroup_size, i32_ty, in.loc()));

                        auto a = av[k + m_block * K];
                        auto b = bv[k + n_block * K];
                        auto b_bc =
                            bb_.add(make_subgroup_broadcast(b, n_offset, bt->ty(), in.loc()));
                        auto &c = result[nb + n + m_block * N];

                        if (a_and_b_complex) {
                            auto &c_im = result_im[nb + n + m_block * N];
                            auto b_bc_re = bb_.add(make_arith(arithmetic_unary::re, b_bc,
                                                              scalar_b_element_ty, in.loc()));
                            auto b_bc_im = bb_.add(make_arith(arithmetic_unary::im, b_bc,
                                                              scalar_b_element_ty, in.loc()));

                            auto ab = mixed_precision_arithmetic(bb_, arithmetic::mul, a, b_bc_re,
                                                                 in.loc());
                            c = mixed_precision_arithmetic(bb_, arithmetic::add, ab, c, in.loc());

                            auto ab_im = mixed_precision_arithmetic(bb_, arithmetic::mul, a,
                                                                    b_bc_im, in.loc());
                            c_im = mixed_precision_arithmetic(bb_, arithmetic::add, ab_im, c_im,
                                                              in.loc());
                        } else {
                            auto ab =
                                mixed_precision_arithmetic(bb_, arithmetic::mul, a, b_bc, in.loc());
                            c = mixed_precision_arithmetic(bb_, arithmetic::add, ab, c, in.loc());
                        }
                    }
                }
            }
        }
    }
    if (a_and_b_complex) {
        auto imaginary_unit =
            bb_.add(make_constant(std::complex<double>{0.0, 1.0}, ct->ty(), in.loc()));
        for (std::size_t i = 0; i < result.size(); ++i) {
            auto &c = result[i];
            auto c_im = result_im[i];
            auto c_im_times_i =
                bb_.add(make_arith(arithmetic::mul, c_im, imaginary_unit, ct->ty(), in.loc()));
            c = bb_.add(make_arith(arithmetic::add, c, c_im_times_i, ct->ty(), in.loc()));
        }
    }
    for (auto &r : result) {
        if (c_ty != r_ty) {
            r = bb_.add(make_cast(r, rt->ty(), in.loc()));
        }
    }
    declare(in.result(0), std::move(result));
    return true;
}

bool coopmatrix_code_generator::operator()(cooperative_matrix_scale_inst &in) {
    auto rt = get_coopmatrix_type(in.result(0));
    auto &bv = vals(in.loc(), in.b(), rt->length(core_cfg_.subgroup_size));
    auto scaled_vals = std::vector<value>{};
    scaled_vals.reserve(bv.size());
    for (auto &b : bv) {
        scaled_vals.emplace_back(
            bb_.add(make_arith(arithmetic::mul, &in.a(), b, rt->ty(), in.loc())));
    }
    declare(in.result(0), std::move(scaled_vals));
    return true;
}

bool coopmatrix_code_generator::operator()(cooperative_matrix_store_inst &in) {
    auto ctx = in.operand().context();
    auto bool_ty = boolean_data_type::get(ctx);
    auto index_ty = scalar_data_type::get(ctx, scalar_type::index);
    auto i32_ty = scalar_data_type::get(ctx, scalar_type::i32);
    // auto ot = get_memref_type(in.operand());
    auto vt = get_coopmatrix_type(in.val());

    int omode = vt->distributed_mode();

    const std::int64_t num_blocks = vt->num_blocks(core_cfg_.subgroup_size);
    const auto checked = normalize_checked_flag(in.checked(), vt->use());
    const auto shape = normalize_shape(vt->shape(), vt->use());
    auto &lowered_vals = vals(in.loc(), in.val(), num_blocks * shape[1]);

    const bool check_m = checked == checked_flag::both || checked == checked_flag::rows;
    const bool check_k = checked == checked_flag::both || checked == checked_flag::cols;
    // const std::int32_t alignment = std::max(in.align(), ot->alignment());

    auto tmp = bb_.add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
    auto subgroup_local_id = bb_.add(make_cast(tmp, index_ty, in.loc()));

    auto pos = std::array<value, 2u>{&in.pos0(), &in.pos1()};
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        auto check_gen = check_condition_generator(&in.operand(), pos);
        auto fibre =
            get_matrix_fibre(bb_, &in.operand(), pos, omode, shape, subgroup_local_id, in.loc());
        auto const store_block = [&](auto &bb) {
            for (std::int64_t k = 0; k < shape[1]; ++k) {
                auto cst_k = bb_.add(make_constant(k, index_ty, in.loc()));
                auto const store_val = [&](auto &bb) {
                    return bb.add(make_store(in.flag(), lowered_vals[k + block * shape[1]], fibre,
                                             {cst_k}, in.align(), in.loc()));
                };
                if (check_k) {
                    auto cond = check_gen(bb, cst_k, 1, in.loc());
                    make_conditional_execution(bb, cond, store_val, in.loc());
                } else {
                    store_val(bb);
                }
            }
        };
        if (check_m) {
            auto cond = check_gen(bb_, subgroup_local_id, 0, in.loc());
            if (shape[0] < core_cfg_.subgroup_size) {
                auto csgs = bb_.add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
                auto check3 = bb_.add(
                    make_cmp(cmp_condition::lt, subgroup_local_id, csgs, bool_ty, in.loc()));
                cond = bb_.add(make_arith(arithmetic::and_, cond, check3, bool_ty, in.loc()));
            }
            make_conditional_execution(bb_, cond, store_block, in.loc());
        } else {
            store_block(bb_);
        }
        if (block + 1 < num_blocks) {
            auto csgs = bb_.add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
            pos[omode] = bb_.add(make_arith(arithmetic::add, pos[omode], csgs, index_ty, in.loc()));
        }
    }
    return true;
}

bool coopmatrix_code_generator::operator()(for_inst &in) {
    if (std::any_of(in.result_begin(), in.result_end(),
                    [](tinytc_value &val) { return isa<coopmatrix_data_type>(*val.ty()); })) {
        auto new_for =
            make_for(in.loop_var().ty(), &in.from(), &in.to(), in.has_step() ? &in.step() : nullptr,
                     vals_copy_expand(in.loc(), in.iter_init()),
                     get_return_types(in, core_cfg_.subgroup_size), in.loc());
        declare_copy_expand(in.child_region(0).params(), new_for->child_region(0).params());
        declare_copy_expand(in.results(), new_for->results());
        new_for->child_region(0).insts() = std::move(in.child_region(0).insts());
        for (auto &sub_in : new_for->child_region(0)) {
            sub_in.subs(&in.child_region(0).param(0), &new_for->child_region(0).param(0));
        }
        run_on_region(new_for->child_region(0));
        bb_.add(std::move(new_for));
        return true;
    }
    return false;
}
bool coopmatrix_code_generator::operator()(if_inst &in) {
    if (std::any_of(in.result_begin(), in.result_end(),
                    [](tinytc_value &val) { return isa<coopmatrix_data_type>(*val.ty()); })) {
        auto new_if = std::make_unique<if_inst>(
            &in.condition(), get_return_types(in, core_cfg_.subgroup_size), in.loc());
        declare_copy_expand(in.results(), new_if->results());

        new_if->then().insts() = std::move(in.then().insts());
        new_if->otherwise().insts() = std::move(in.otherwise().insts());
        run_on_region(new_if->then());
        run_on_region(new_if->otherwise());

        bb_.add(inst{new_if.release()});
        return true;
    }
    return false;
}
bool coopmatrix_code_generator::operator()(yield_inst &in) {
    if (std::any_of(in.op_begin(), in.op_end(),
                    [](tinytc_value &val) { return isa<coopmatrix_data_type>(*val.ty()); })) {
        bb_.add(make_yield(vals_copy_expand(in.loc(), in.operands()), in.loc()));
        return true;
    }
    return false;
}

void coopmatrix_code_generator::run_on_region(region_node &reg) {
    // Move all instructions to a temporary ilist.
    // We later move the instructions back, except those that are lowered remain in old_ilist
    // and are cleaned up at the end of the function.
    auto old_ilist = std::move(reg.insts());

    auto old_reg = bb_.get_region();
    bb_ = region_builder{&reg};

    auto it = old_ilist.begin();
    while (it != old_ilist.end()) {
        bool replaced = visit(*this, *it);
        if (!replaced) {
            auto instr = it.get();
            it = old_ilist.unlink(it);
            reg.insts().push_back(instr);
            for (auto &subreg : instr->child_regions()) {
                run_on_region(subreg);
            }
        } else {
            ++it;
        }
    }
    it = old_ilist.end();
    while (it != old_ilist.begin()) {
        --it;
        for (auto &result : it->results()) {
            if (result.has_uses()) {
                throw compilation_error(result.loc(), status::ir_value_still_has_uses);
            }
        }
        it = old_ilist.erase(it);
    }

    bb_ = region_builder{old_reg};
}

lower_coopmatrix_pass::lower_coopmatrix_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void lower_coopmatrix_pass::run_on_function(function_node &fn) {
    auto const subgroup_size = fn.subgroup_size();
    core_config core_cfg = {};
    try {
        core_cfg = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }

    run_on_region(fn.body(), core_cfg);
}

void lower_coopmatrix_pass::run_on_region(region_node &reg, core_config const &core_cfg) {
    auto gen = coopmatrix_code_generator{core_cfg, reg};
    gen.run_on_region(reg);
}

} // namespace tinytc
