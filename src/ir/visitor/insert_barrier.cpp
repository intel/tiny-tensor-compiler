// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/insert_barrier.hpp"
#include "ir/visitor/alias_analysis.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/value.hpp"

#include <clir/builtin_type.hpp>
#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <bit>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

using clir::visit;

namespace tinytc {

/* Data type nodes */
bool insert_barrier::operator()(void_data_type &) { return false; }
bool insert_barrier::operator()(group_data_type &b) { return visit(*this, *b.ty()); }
bool insert_barrier::operator()(memref_data_type &m) {
    return m.addrspace() == clir::address_space::local_t;
}
bool insert_barrier::operator()(scalar_data_type &) { return false; }

/* Value nodes */
std::uintptr_t insert_barrier::operator()(float_imm &) {
    return std::bit_cast<std::uintptr_t>(nullptr);
}
std::uintptr_t insert_barrier::operator()(int_imm &) {
    return std::bit_cast<std::uintptr_t>(nullptr);
}
std::uintptr_t insert_barrier::operator()(val &v) {
    if (visit(*this, *v.ty())) {
        return std::bit_cast<std::uintptr_t>(aa_.root(v));
    }
    return std::bit_cast<std::uintptr_t>(nullptr);
}

/* Inst nodes */
std::unordered_set<std::uintptr_t> insert_barrier::operator()(blas_a2_inst &g) {
    auto rw = std::unordered_set<std::uintptr_t>{};
    rw.emplace(visit(*this, *g.A()));
    rw.emplace(visit(*this, *g.B()));
    return rw;
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(blas_a3_inst &inst) {
    auto rw = std::unordered_set<std::uintptr_t>{};
    rw.emplace(visit(*this, *inst.A()));
    rw.emplace(visit(*this, *inst.B()));
    rw.emplace(visit(*this, *inst.C()));
    return rw;
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(loop_inst &p) {
    return visit(*this, *p.body());
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(scalar_inst &) { return {}; }

std::unordered_set<std::uintptr_t> insert_barrier::operator()(alloca_inst &) { return {}; }

std::unordered_set<std::uintptr_t> insert_barrier::operator()(barrier_inst &) {
    last_instruction_was_barrier_ = true;
    return {};
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(expand_inst &) { return {}; }
std::unordered_set<std::uintptr_t> insert_barrier::operator()(fuse_inst &) { return {}; }

std::unordered_set<std::uintptr_t> insert_barrier::operator()(load_inst &e) {
    auto rw = std::unordered_set<std::uintptr_t>{};
    auto t = dynamic_cast<memref_data_type *>(e.operand()->ty().get());
    if (t) {
        rw.emplace(visit(*this, *e.operand()));
    }
    return rw;
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(if_inst &in) {
    auto s = visit(*this, *in.then());
    if (in.otherwise()) {
        s.merge(visit(*this, *in.otherwise()));
    }
    return s;
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(lifetime_stop_inst &) { return {}; }

std::unordered_set<std::uintptr_t> insert_barrier::operator()(size_inst &) { return {}; }

std::unordered_set<std::uintptr_t> insert_barrier::operator()(store_inst &s) {
    auto rw = std::unordered_set<std::uintptr_t>{};
    rw.emplace(visit(*this, *s.operand()));
    return rw;
}

std::unordered_set<std::uintptr_t> insert_barrier::operator()(subview_inst &) { return {}; }
std::unordered_set<std::uintptr_t> insert_barrier::operator()(yield_inst &) { return {}; }

/* Region nodes */
std::unordered_set<std::uintptr_t> insert_barrier::operator()(rgn &b) {
    auto const intersects = [](std::unordered_set<std::uintptr_t> const &a,
                               std::unordered_set<std::uintptr_t> const &b) {
        for (auto &aa : a) {
            if (aa != std::bit_cast<std::uintptr_t>(nullptr) && b.find(aa) != b.end()) {
                return true;
            }
        }
        return false;
    };

    auto rw = std::unordered_set<std::uintptr_t>{};
    auto insts = std::vector<inst>{};
    insts.reserve(b.insts().size());
    for (auto &s : b.insts()) {
        auto my_rw = visit(*this, *s);
        if (intersects(my_rw, rw)) {
            insts.emplace_back(inst(std::make_shared<barrier_inst>()));
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
