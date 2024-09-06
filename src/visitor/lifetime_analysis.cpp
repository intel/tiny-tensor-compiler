// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/lifetime_analysis.hpp"
#include "node/value_node.hpp"
#include "support/visit.hpp"
#include "visitor/alias_analysis.hpp"

#include <algorithm>
#include <iterator>
#include <memory>

namespace tinytc {

find_alloca::find_alloca(bool recursive) : recursive_(recursive) {}

/* Inst nodes */
value find_alloca::operator()(inst_node &) { return value{}; }
value find_alloca::operator()(alloca_inst &a) { return a.result(); }
value find_alloca::operator()(for_inst &p) {
    if (recursive_) {
        visit(*this, *p.body());
    }
    return value{};
}
value find_alloca::operator()(if_inst &in) {
    if (recursive_) {
        visit(*this, *in.then());
        if (in.otherwise()) {
            visit(*this, *in.otherwise());
        }
    }
    return value{};
}

/* Region nodes */
value find_alloca::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        alloca_.emplace_back(visit(*this, *s));
    }
    return value{};
}

std::vector<value> find_alloca::allocas() const { return alloca_; }

/* Inst nodes */
auto lifetime_inserter::operator()(inst_node &) -> std::unordered_set<value_node const *> {
    return {};
}

auto lifetime_inserter::operator()(blas_a2_inst &a) -> std::unordered_set<value_node const *> {
    return {a.A().get(), a.B().get()};
}

auto lifetime_inserter::operator()(blas_a3_inst &inst) -> std::unordered_set<value_node const *> {
    return {inst.A().get(), inst.B().get(), inst.C().get()};
}

auto lifetime_inserter::operator()(loop_inst &p) -> std::unordered_set<value_node const *> {
    return visit(*this, *p.body());
}

auto lifetime_inserter::operator()(alloca_inst &a) -> std::unordered_set<value_node const *> {
    return {a.result().get()};
}

auto lifetime_inserter::operator()(barrier_inst &) -> std::unordered_set<value_node const *> {
    return {};
}

auto lifetime_inserter::operator()(expand_inst &e) -> std::unordered_set<value_node const *> {
    return std::unordered_set<value_node const *>{e.operand().get(), e.result().get()};
}

auto lifetime_inserter::operator()(fuse_inst &f) -> std::unordered_set<value_node const *> {
    return std::unordered_set<value_node const *>{f.operand().get(), f.result().get()};
}

auto lifetime_inserter::operator()(load_inst &e) -> std::unordered_set<value_node const *> {
    return std::unordered_set<value_node const *>{e.operand().get(), e.result().get()};
}

auto lifetime_inserter::operator()(if_inst &in) -> std::unordered_set<value_node const *> {
    auto s = visit(*this, *in.then());
    if (in.otherwise()) {
        s.merge(visit(*this, *in.otherwise()));
    }
    return s;
}

auto lifetime_inserter::operator()(lifetime_stop_inst &ls)
    -> std::unordered_set<value_node const *> {
    return {ls.object().get()};
}

auto lifetime_inserter::operator()(parallel_inst &p) -> std::unordered_set<value_node const *> {
    return visit(*this, *p.body());
}

auto lifetime_inserter::operator()(size_inst &s) -> std::unordered_set<value_node const *> {
    return std::unordered_set<value_node const *>{s.operand().get()};
}

auto lifetime_inserter::operator()(store_inst &s) -> std::unordered_set<value_node const *> {
    return std::unordered_set<value_node const *>{s.operand().get()};
}

auto lifetime_inserter::operator()(subview_inst &s) -> std::unordered_set<value_node const *> {
    return {s.result().get(), s.operand().get()};
}

auto lifetime_inserter::operator()(yield_inst &) -> std::unordered_set<value_node const *> {
    return {};
}

/* Region nodes */
auto lifetime_inserter::operator()(rgn &b) -> std::unordered_set<value_node const *> {
    auto const intersects = [](std::vector<value> &a,
                               std::unordered_set<value_node const *> const &b) {
        for (auto aa = a.begin(); aa != a.end(); ++aa) {
            if (b.find(aa->get()) != b.end()) {
                return aa;
            }
        }
        return a.end();
    };

    auto fa = find_alloca{};
    fa(b);
    auto allocas = fa.allocas();

    auto rgn_ops = std::unordered_set<value_node const *>{};

    auto s = b.insts().end();
    while (s != b.insts().begin()) {
        auto operands = visit(*this, **(s - 1));
        rgn_ops.insert(operands.begin(), operands.end());
        auto operands_root = decltype(operands){};
        std::transform(operands.begin(), operands.end(),
                       std::inserter(operands_root, operands_root.begin()),
                       [this](auto const &op) { return aa_.root(*op); });
        std::vector<value>::iterator aa;
        while ((aa = intersects(allocas, operands_root)) != allocas.end()) {
            s = b.insts().insert(s, inst{std::make_unique<lifetime_stop_inst>(*aa).release()});
            allocas.erase(aa);
        }
        --s;
    }

    return rgn_ops;
}

/* Function nodes */
void lifetime_inserter::operator()(prototype &) {}

void lifetime_inserter::operator()(function &fn) {
    auto aa = alias_analyser{};
    aa(fn);
    aa_ = aa.get_result();

    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

void lifetime_inserter::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
