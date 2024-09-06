// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/insert_barrier.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "visitor/alias_analysis.hpp"

#include <clir/builtin_type.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace tinytc {

/* Data type nodes */
bool insert_barrier::operator()(void_data_type &) { return false; }
bool insert_barrier::operator()(group_data_type &b) { return visit(*this, *b.ty()); }
bool insert_barrier::operator()(memref_data_type &m) {
    return m.addrspace() == clir::address_space::local_t;
}
bool insert_barrier::operator()(scalar_data_type &) { return false; }

/* Value nodes */
value_node *insert_barrier::operator()(float_imm &) { return nullptr; }
value_node *insert_barrier::operator()(int_imm &) { return nullptr; }
value_node *insert_barrier::operator()(val &v) {
    if (visit(*this, *v.ty())) {
        return &v;
    }
    return nullptr;
}

/* Inst nodes */
std::unordered_set<value_node *> insert_barrier::operator()(inst_node &) { return {}; }

std::unordered_set<value_node *> insert_barrier::operator()(blas_a2_inst &g) {
    auto rw = std::unordered_set<value_node *>{};
    rw.emplace(visit(*this, *g.A()));
    rw.emplace(visit(*this, *g.B()));
    return rw;
}

std::unordered_set<value_node *> insert_barrier::operator()(blas_a3_inst &inst) {
    auto rw = std::unordered_set<value_node *>{};
    rw.emplace(visit(*this, *inst.A()));
    rw.emplace(visit(*this, *inst.B()));
    rw.emplace(visit(*this, *inst.C()));
    return rw;
}

std::unordered_set<value_node *> insert_barrier::operator()(loop_inst &p) {
    return visit(*this, *p.body());
}

std::unordered_set<value_node *> insert_barrier::operator()(alloca_inst &) { return {}; }

std::unordered_set<value_node *> insert_barrier::operator()(barrier_inst &) {
    last_instruction_was_barrier_ = true;
    return {};
}

std::unordered_set<value_node *> insert_barrier::operator()(expand_inst &) { return {}; }
std::unordered_set<value_node *> insert_barrier::operator()(fuse_inst &) { return {}; }

std::unordered_set<value_node *> insert_barrier::operator()(load_inst &e) {
    auto rw = std::unordered_set<value_node *>{};
    auto t = dyn_cast<memref_data_type>(e.operand()->ty().get());
    if (t) {
        rw.emplace(visit(*this, *e.operand()));
    }
    return rw;
}

std::unordered_set<value_node *> insert_barrier::operator()(if_inst &in) {
    auto s = visit(*this, *in.then());
    if (in.otherwise()) {
        s.merge(visit(*this, *in.otherwise()));
    }
    return s;
}

std::unordered_set<value_node *> insert_barrier::operator()(lifetime_stop_inst &) { return {}; }

std::unordered_set<value_node *> insert_barrier::operator()(parallel_inst &p) {
    return visit(*this, *p.body());
}

std::unordered_set<value_node *> insert_barrier::operator()(size_inst &) { return {}; }

std::unordered_set<value_node *> insert_barrier::operator()(store_inst &s) {
    auto rw = std::unordered_set<value_node *>{};
    rw.emplace(visit(*this, *s.operand()));
    return rw;
}

std::unordered_set<value_node *> insert_barrier::operator()(subview_inst &) { return {}; }
std::unordered_set<value_node *> insert_barrier::operator()(yield_inst &) { return {}; }

/* Region nodes */
std::unordered_set<value_node *> insert_barrier::operator()(rgn &b) {
    auto const intersects = [this](std::unordered_set<value_node *> const &a,
                                   std::unordered_set<value_node *> const &b) {
        for (auto &aa : a) {
            if (aa != nullptr) {
                for (auto &bb : b) {
                    if (aa_.alias(*aa, *bb)) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    auto rw = std::unordered_set<value_node *>{};
    auto insts = std::vector<inst>{};
    insts.reserve(b.insts().size());
    for (auto &s : b.insts()) {
        auto my_rw = visit(*this, *s);
        if (intersects(my_rw, rw)) {
            insts.emplace_back(inst{std::make_unique<barrier_inst>().release()});
            rw.clear();
        }
        insts.emplace_back(s);
        if (last_instruction_was_barrier_) {
            last_instruction_was_barrier_ = false;
            rw.clear();
        }
        rw.merge(my_rw);
    }
    b.insts(std::move(insts));
    return rw;
}

/* Function nodes */
void insert_barrier::operator()(prototype &) {}

void insert_barrier::operator()(function &fn) {
    auto aa = alias_analyser{};
    aa(fn);
    aa_ = aa.get_result();
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void insert_barrier::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
