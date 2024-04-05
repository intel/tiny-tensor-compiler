// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/visitor/lifetime_analysis.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/visitor/alias_analysis.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <algorithm>
#include <iterator>
#include <memory>

using clir::visit;

namespace tinytc::ir::internal {

find_alloca::find_alloca(bool recursive) : recursive_(recursive) {}

/* Inst nodes */
value find_alloca::operator()(inst_node &) { return nullptr; }
value find_alloca::operator()(alloca_inst &a) { return a.result(); }
value find_alloca::operator()(for_inst &p) {
    if (recursive_) {
        visit(*this, *p.body());
    }
    return nullptr;
}

/* Region nodes */
value find_alloca::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        alloca_.emplace_back(visit(*this, *s));
    }
    return nullptr;
}

std::vector<value> find_alloca::allocas() const { return alloca_; }

/* Inst nodes */
std::unordered_set<value_node *> lifetime_inserter::operator()(blas_a2_inst &a) {
    return {a.A().get(), a.B().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(blas_a3_inst &inst) {
    return {inst.A().get(), inst.B().get(), inst.C().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(loop_inst &p) {
    return visit(*this, *p.body());
}

std::unordered_set<value_node *> lifetime_inserter::operator()(scalar_inst &) { return {}; }

std::unordered_set<value_node *> lifetime_inserter::operator()(alloca_inst &a) {
    return {a.result().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(barrier_inst &) { return {}; }

std::unordered_set<value_node *> lifetime_inserter::operator()(expand_inst &e) {
    return std::unordered_set<value_node *>{e.operand().get(), e.result().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(fuse_inst &f) {
    return std::unordered_set<value_node *>{f.operand().get(), f.result().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(load_inst &e) {
    return std::unordered_set<value_node *>{e.operand().get(), e.result().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(if_inst &in) {
    auto s = visit(*this, *in.then());
    if (in.otherwise()) {
        s.merge(visit(*this, *in.otherwise()));
    }
    return s;
}

std::unordered_set<value_node *> lifetime_inserter::operator()(lifetime_stop_inst &ls) {
    return {ls.object().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(size_inst &s) {
    return std::unordered_set<value_node *>{s.operand().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(store_inst &s) {
    return std::unordered_set<value_node *>{s.operand().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(subview_inst &s) {
    return {s.result().get(), s.operand().get()};
}

std::unordered_set<value_node *> lifetime_inserter::operator()(yield_inst &) { return {}; }

/* Region nodes */
std::unordered_set<value_node *> lifetime_inserter::operator()(rgn &b) {
    auto const intersects = [](std::vector<value> &a, std::unordered_set<value_node *> const &b) {
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

    auto rgn_ops = std::unordered_set<value_node *>{};

    auto s = b.insts().end();
    while (s != b.insts().begin()) {
        auto operands = visit(*this, **(s - 1));
        rgn_ops.insert(operands.begin(), operands.end());
        auto operands_root = decltype(operands){};
        std::transform(operands.begin(), operands.end(),
                       std::inserter(operands_root, operands_root.begin()),
                       [this](auto &op) { return aa_.root(*op); });
        std::vector<value>::iterator aa;
        while ((aa = intersects(allocas, operands_root)) != allocas.end()) {
            s = b.insts().insert(s, inst(std::make_shared<lifetime_stop_inst>(*aa)));
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

} // namespace tinytc::ir::internal
