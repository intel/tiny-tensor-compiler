// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/gcd.hpp"
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
void gcd_analysis_result::set(::tinytc_value const &a, std::int64_t g) { gcd_[&a] = g; }

class gcd_helper {
  public:
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(constant_inst const &in);
    void operator()(for_inst const &in);
    void operator()(subgroup_broadcast_inst const &in);

    auto get_result() && { return std::move(gcd_); }

  private:
    gcd_analysis_result gcd_;
};

void gcd_helper::operator()(inst_node const &) {}
void gcd_helper::operator()(arith_inst const &in) {
    auto compute_gcd = [&]() -> std::optional<std::int64_t> {
        const auto ga = gcd_.get(in.a());
        const auto gb = gcd_.get(in.b());
        switch (in.operation()) {
        case arithmetic::add:
            return std::gcd(ga, gb);
        case arithmetic::mul:
            return ga * gb;
        case arithmetic::div: {
            return ga % gb == 0 ? ga / gb : 1;
        }
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

auto gcd_analysis::run_on_function(function_node const &fn) -> gcd_analysis_result {
    auto visitor = gcd_helper{};

    walk<walk_order::pre_order>(fn, [&visitor](inst_node const &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
