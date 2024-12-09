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
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class coopmatrix_code_generator {
  public:
    coopmatrix_code_generator(core_config core_cfg) : core_cfg_{std::move(core_cfg)} {}
    void operator()(inst_node &in);
    void operator()(cooperative_matrix_load_inst &in);
    void operator()(cooperative_matrix_scale_inst &in);
    void operator()(cooperative_matrix_store_inst &in);

    inline auto insertion_point() const -> region_node::iterator { return ip_; }
    inline auto insertion_point(region_node::iterator ip) { ip_ = ip; }

    inline auto add(inst in) -> value {
        auto result = value{};
        in.get_values(result);
        ip_ = ip_->parent()->insts().insert(++ip_, in.release());
        return result;
    }
    inline auto add_multivalued(inst in) -> std::vector<value> {
        auto num_results = in.get_values({});
        auto results = std::vector<value>(static_cast<std::size_t>(num_results));
        results.resize(in.get_values(results));
        ip_ = ip_->parent()->insts().insert(++ip_, in.release());
        return results;
    }

  private:
    auto declare(tinytc_value const &v, std::vector<value> vals);
    auto vals(tinytc_value const &v, std::int64_t expected_size) -> std::vector<value> &;

    core_config core_cfg_ = {};
    region_node::iterator ip_;
    std::unordered_map<const_tinytc_value_t, std::vector<value>> vals_;
};

auto coopmatrix_code_generator::declare(value_node const &v, std::vector<value> vals) {
    vals_[&v] = std::move(vals);
}
auto coopmatrix_code_generator::vals(value_node const &v,
                                     std::int64_t expected_size) -> std::vector<value> & {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        if (static_cast<std::int64_t>(it->second.size()) == expected_size) {
            return it->second;
        }
    }
    throw compilation_error(v.loc(), status::internal_compiler_error);
}

void coopmatrix_code_generator::operator()(inst_node &) {}

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
void coopmatrix_code_generator::operator()(cooperative_matrix_load_inst &in) {
    auto ctx = in.operand().context();
    auto bool_ty = boolean_data_type::get(ctx);
    auto index_ty = scalar_data_type::get(ctx, scalar_type::index);
    auto i32_ty = scalar_data_type::get(ctx, scalar_type::i32);
    auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));

    const auto checked = normalize_checked_flag(in.checked(), rt->use());
    const auto shape = normalize_shape(rt->shape(), rt->use());
    const auto trans = normalize_transpose(in.t(), rt->use());

    const int omode = trans == transpose::T ? 1 : 0;
    const bool check_m = checked == checked_flag::both || checked == checked_flag::rows;
    const bool check_k = checked == checked_flag::both || checked == checked_flag::cols;
    const std::int32_t alignment = std::max(in.align(), ot->alignment());

    auto tmp = add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
    auto subgroup_local_id = add(make_cast(tmp, index_ty, in.loc()));

    auto pos = std::array<value, 2u>{&in.pos0(), &in.pos1()};
    auto check_gen = check_condition_generator(&in.operand(), pos);
    auto fibre =
        get_matrix_fibre(*this, &in.operand(), pos, omode, shape, subgroup_local_id, in.loc());
    auto const load_block = [&](auto &bb) -> std::vector<value> {
        auto loaded_vals = std::vector<value>{};
        loaded_vals.reserve(shape[1]);
        for (std::int64_t k = 0; k < shape[1]; ++k) {
            auto cst_k = add(make_constant(k, index_ty, in.loc()));
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
        auto cond = check_gen(*this, subgroup_local_id, omode, in.loc());
        if (shape[0] < core_cfg_.subgroup_size) {
            auto csgs = add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
            auto check3 =
                add(make_cmp(cmp_condition::lt, subgroup_local_id, csgs, bool_ty, in.loc()));
            cond = add(make_arith(arithmetic::and_, cond, check3, bool_ty, in.loc()));
        }
        declare(in.result(0),
                make_conditional_execution(*this, cond, load_block, shape[1], rt->ty(), in.loc()));
    } else {
        declare(in.result(0), load_block(*this));
    }
}

void coopmatrix_code_generator::operator()(cooperative_matrix_scale_inst &in) {
    auto bt = get_coopmatrix_type(in.b());
    auto lowered_vals = vals(in.b(), bt->length(core_cfg_.subgroup_size));
    auto scaled_vals = std::vector<value>{};
    scaled_vals.reserve(lowered_vals.size());
    for (auto &val : lowered_vals) {
        scaled_vals.emplace_back(
            add(make_arith(arithmetic::mul, &in.a(), val, val->ty(), in.loc())));
    }
    declare(in.result(0), std::move(scaled_vals));
}

void coopmatrix_code_generator::operator()(cooperative_matrix_store_inst &in) {
    auto ctx = in.operand().context();
    auto bool_ty = boolean_data_type::get(ctx);
    auto index_ty = scalar_data_type::get(ctx, scalar_type::index);
    auto i32_ty = scalar_data_type::get(ctx, scalar_type::i32);
    auto ot = get_memref_type(in.operand());
    auto vt = get_coopmatrix_type(in.val());

    const auto checked = normalize_checked_flag(in.checked(), vt->use());
    const auto shape = normalize_shape(vt->shape(), vt->use());
    auto &lowered_vals = vals(in.val(), shape[1]);

    const bool check_m = checked == checked_flag::both || checked == checked_flag::rows;
    const bool check_k = checked == checked_flag::both || checked == checked_flag::cols;
    const std::int32_t alignment = std::max(in.align(), ot->alignment());

    auto tmp = add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
    auto subgroup_local_id = add(make_cast(tmp, index_ty, in.loc()));

    auto pos = std::array<value, 2u>{&in.pos0(), &in.pos1()};
    auto check_gen = check_condition_generator(&in.operand(), pos);
    auto fibre = get_matrix_fibre(*this, &in.operand(), pos, 0, shape, subgroup_local_id, in.loc());
    auto const store_block = [&](auto &bb) {
        for (std::int64_t k = 0; k < shape[1]; ++k) {
            auto cst_k = add(make_constant(k, index_ty, in.loc()));
            auto const store_val = [&](auto &bb) {
                return bb.add(
                    make_store(in.flag(), lowered_vals[k], fibre, {cst_k}, in.align(), in.loc()));
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
        auto cond = check_gen(*this, subgroup_local_id, 0, in.loc());
        if (shape[0] < core_cfg_.subgroup_size) {
            auto csgs = add(make_constant(core_cfg_.subgroup_size, index_ty, in.loc()));
            auto check3 =
                add(make_cmp(cmp_condition::lt, subgroup_local_id, csgs, bool_ty, in.loc()));
            cond = add(make_arith(arithmetic::and_, cond, check3, bool_ty, in.loc()));
        }
        make_conditional_execution(*this, cond, store_block, in.loc());
    } else {
        store_block(*this);
    }
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

    auto gen = coopmatrix_code_generator{core_cfg};
    walk<walk_order::post_order>(fn, [&](region_node &reg) {
        for (auto it = reg.begin(); it != reg.end(); ++it) {
            gen.insertion_point(it);
            visit(gen, *it);
            // delete the lowered instruction if we have new instructions
            if (it != gen.insertion_point()) {
                reg.insts().erase(it);
                it = gen.insertion_point();
            }
        }
    });
}

} // namespace tinytc
