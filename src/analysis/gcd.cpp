// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "node/attr_node.hpp"
#include "node/inst_node.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/types.hpp"

#include <cstdlib> // IWYU pragma: keep
#include <functional>
#include <numeric>
#include <utility>
#include <variant>

namespace tinytc {

tensor_gcd::tensor_gcd(std::vector<std::int64_t> shape_gcd, std::vector<std::int64_t> stride_gcd)
    : shape_gcd_(std::move(shape_gcd)), stride_gcd_(std::move(stride_gcd)) {}

auto gcd_analysis_result::get(::const_tinytc_value_t a) const -> std::int64_t {
    const auto g = get_if(a);
    return g ? *g : 1;
}
auto gcd_analysis_result::get(::tinytc_value const &a) const -> std::int64_t { return get(&a); }
auto gcd_analysis_result::get_if(::const_tinytc_value_t a) const -> std::optional<std::int64_t> {
    if (auto it = gcd_.find(a); it != gcd_.end()) {
        return it->second;
    }
    return std::nullopt;
}
auto gcd_analysis_result::get_if(::tinytc_value const &a) const -> std::optional<std::int64_t> {
    return get_if(&a);
}
auto gcd_analysis_result::get_tensor_if(::const_tinytc_value_t a) const -> tensor_gcd const * {
    if (auto it = tensor_gcd_.find(a); it != tensor_gcd_.end()) {
        return &it->second;
    }
    return nullptr;
}
auto gcd_analysis_result::get_tensor_if(::tinytc_value const &a) const -> tensor_gcd const * {
    return get_tensor_if(&a);
}
void gcd_analysis_result::set(::tinytc_value const &a, std::int64_t g) { gcd_[&a] = g; }
void gcd_analysis_result::set_tensor(::tinytc_value const &a, tensor_gcd g) {
    tensor_gcd_[&a] = std::move(g);
}

class gcd_helper {
  public:
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(constant_inst const &in);
    void operator()(for_inst const &in);
    void operator()(subgroup_broadcast_inst const &in);

    void set_from_attributes(function_node const &fn);

    auto get_result() && { return std::move(gcd_); }

  private:
    gcd_analysis_result gcd_;
};

void gcd_helper::operator()(inst_node const &) {}
void gcd_helper::operator()(arith_inst const &in) {
    auto compute_gcd = [&]() -> std::optional<std::int64_t> {
        switch (in.operation()) {
        case arithmetic::add:
            return std::gcd(gcd_.get(in.a()), gcd_.get(in.b()));
        case arithmetic::mul:
            return gcd_.get(in.a()) * gcd_.get(in.b());
        default:
            break;
        }
        return std::nullopt;
    };
    auto g = compute_gcd();
    if (g) {
        gcd_.set(in.result(0), *g);
    }
}
void gcd_helper::operator()(arith_unary_inst const &in) {
    auto compute_gcd = [&]() -> std::optional<std::int64_t> {
        switch (in.operation()) {
        case arithmetic_unary::abs:
        case arithmetic_unary::not_:
            return gcd_.get(in.a());
        default:
            break;
        }
        return std::nullopt;
    };
    auto g = compute_gcd();
    if (g) {
        gcd_.set(in.result(0), *g);
    }
}
void gcd_helper::operator()(cast_inst const &in) {
    auto g = gcd_.get_if(in.a());
    if (g) {
        gcd_.set(in.result(0), *g);
    }
}
void gcd_helper::operator()(constant_inst const &in) {
    if (std::holds_alternative<std::int64_t>(in.value())) {
        gcd_.set(in.result(0), std::abs(std::get<std::int64_t>(in.value())));
    }
}
void gcd_helper::operator()(for_inst const &in) {
    if (in.has_step()) {
        auto g = std::gcd(gcd_.get(in.from()), gcd_.get(in.step()));
        gcd_.set(in.loop_var(), g);
    }
}
void gcd_helper::operator()(subgroup_broadcast_inst const &in) {
    auto g = gcd_.get_if(in.a());
    if (g) {
        gcd_.set(in.result(0), *g);
    }
}

void gcd_helper::set_from_attributes(function_node const &fn) {
    auto known_tensor_gcd = [&](memref_data_type const *mr, tinytc_attr_t dict) -> tensor_gcd {
        auto shape_gcd = std::vector<std::int64_t>{};
        if (auto shape_attr = get_attr(dict, "shape_gcd"); shape_attr) {
            shape_gcd = get_array_attr_as<std::int64_t>(shape_attr);
        }
        if (shape_gcd.size() < static_cast<std::size_t>(mr->dim())) {
            std::size_t i = shape_gcd.size();
            shape_gcd.resize(mr->dim());
            for (; i < shape_gcd.size(); ++i) {
                const auto s = mr->shape(i);
                shape_gcd[i] = !is_dynamic_value(s) ? s : 1;
            }
        }

        auto stride_gcd = std::vector<std::int64_t>{};
        if (auto stride_attr = get_attr(dict, "stride_gcd"); stride_attr) {
            stride_gcd = get_array_attr_as<std::int64_t>(stride_attr);
        }
        if (stride_gcd.size() < static_cast<std::size_t>(mr->dim())) {
            std::size_t i = stride_gcd.size();
            stride_gcd.resize(mr->dim());
            for (; i < stride_gcd.size(); ++i) {
                const auto s = mr->stride(i);
                stride_gcd[i] = !is_dynamic_value(s) ? s : 1;
            }
        }

        return tensor_gcd(std::move(shape_gcd), std::move(stride_gcd));
    };
    for (std::int32_t arg_no = 0; arg_no < fn.num_params(); ++arg_no) {
        auto ty = fn.params()[arg_no].ty();
        if (auto g = dyn_cast<group_data_type>(ty); g) {
            ty = g->ty();
        }
        if (auto mr = dyn_cast<memref_data_type>(ty); mr) {
            gcd_.set_tensor(fn.params()[arg_no], known_tensor_gcd(mr, fn.param_attr(arg_no)));
        }
    }
}

auto gcd_analysis::run_on_function(function_node const &fn) -> gcd_analysis_result {
    auto visitor = gcd_helper{};

    visitor.set_from_attributes(fn);
    walk<walk_order::pre_order>(fn, [&visitor](inst_node const &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
