// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/stack.hpp"
#include "error.hpp"
#include "ir/node/data_type_node.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/types.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <vector>

using clir::visit;

namespace tinytc {

/* Inst nodes */
void stack_ptr::operator()(inst_node &) {}
void stack_ptr::operator()(alloca_inst &a) {
    auto t = dynamic_cast<memref_data_type *>(a.result()->ty().get());
    if (t == nullptr) {
        throw compilation_error(a.loc(), status::ir_expected_memref);
    }
    auto size = t->size_in_bytes();
    std::size_t stack_ptr = 0;
    auto it = allocs_.begin();
    for (; it != allocs_.end(); ++it) {
        if (it->start - stack_ptr >= static_cast<std::size_t>(size)) {
            break;
        }
        stack_ptr = it->stop;
    }
    allocs_.insert(it, allocation{a.result().get(), stack_ptr, stack_ptr + size});
    a.stack_ptr(stack_ptr);
}
void stack_ptr::operator()(lifetime_stop_inst &s) {
    int num = 0;
    auto v = s.object().get();
    for (auto it = allocs_.begin(); it != allocs_.end();) {
        if (it->value == v) {
            it = allocs_.erase(it);
            ++num;
        } else {
            ++it;
        }
    }
    if (num != 1) {
        throw compilation_error(s.loc(), status::internal_compiler_error,
                                "Incorrect lifetime_stop: value not found in list of allocations");
    }
}
void stack_ptr::operator()(for_inst &p) { visit(*this, *p.body()); }

/* Region nodes */
void stack_ptr::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void stack_ptr::operator()(prototype &) {}
void stack_ptr::operator()(function &fn) {
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void stack_ptr::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        allocs_.clear();
        visit(*this, *decl);
    }
}

} // namespace tinytc
