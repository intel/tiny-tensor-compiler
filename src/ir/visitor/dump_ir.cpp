// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/dump_ir.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

using clir::visit;

namespace tinytc {

ir_dumper::ir_dumper(std::ostream &os) : os_(os) {}

/* Data type nodes */
void ir_dumper::operator()(void_data_type &) { os_ << "void"; }
void ir_dumper::operator()(group_data_type &g) {
    os_ << "group<";
    visit(*this, *g.ty());
    os_ << ">";
}
void ir_dumper::operator()(memref_data_type &d) {
    auto const val = [&](std::int64_t v) -> std::ostream & {
        if (is_dynamic_value(v)) {
            return os_ << "?";
        }
        return os_ << v;
    };
    os_ << "memref<" << to_string(d.element_ty());
    for (auto const &s : d.shape()) {
        os_ << "x";
        val(s);
    }
    if (!d.is_canonical_stride()) {
        os_ << ",strided<";
        do_with_infix(d.stride().begin(), d.stride().end(), [&](auto &a) { val(a); });
        os_ << ">";
    }
    os_ << ">";
}
void ir_dumper::operator()(scalar_data_type &s) { os_ << to_string(s.ty()); }

/* Value nodes */
void ir_dumper::operator()(float_imm &v) {
    auto flags = os_.flags();
    os_ << std::hexfloat << v.value();
    os_.flags(flags);
}
void ir_dumper::operator()(int_imm &v) {
    if (is_dynamic_value(v.value())) {
        os_ << "?";
    } else {
        os_ << v.value();
    }
}
void ir_dumper::operator()(val &v) { os_ << "%" << v.name(); }

/* Inst nodes */
void ir_dumper::dump_blas_a2(blas_a2_inst &g) {
    visit(*this, *g.alpha());
    os_ << ", ";
    visit(*this, *g.A());
    os_ << ", ";
    visit(*this, *g.beta());
    os_ << ", ";
    visit(*this, *g.B());
    os_ << " : ";
    visit(*this, *g.alpha()->ty());
    os_ << ", ";
    visit(*this, *g.A()->ty());
    os_ << ", ";
    visit(*this, *g.beta()->ty());
    os_ << ", ";
    visit(*this, *g.B()->ty());
}

void ir_dumper::dump_blas_a3(blas_a3_inst &g) {
    visit(*this, *g.alpha());
    os_ << ", ";
    visit(*this, *g.A());
    os_ << ", ";
    visit(*this, *g.B());
    os_ << ", ";
    visit(*this, *g.beta());
    os_ << ", ";
    visit(*this, *g.C());
    os_ << " : ";
    visit(*this, *g.alpha()->ty());
    os_ << ", ";
    visit(*this, *g.A()->ty());
    os_ << ", ";
    visit(*this, *g.B()->ty());
    os_ << ", ";
    visit(*this, *g.beta()->ty());
    os_ << ", ";
    visit(*this, *g.C()->ty());
}

void ir_dumper::operator()(alloca_inst &a) {
    visit(*this, *a.result());
    os_ << " = alloca -> ";
    visit(*this, *a.result()->ty());
}

void ir_dumper::operator()(axpby_inst &a) {
    os_ << "axpby";
    os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst &>(a));
}

void ir_dumper::operator()(barrier_inst &) { os_ << "barrier"; }

void ir_dumper::operator()(binary_op_inst &a) {
    visit(*this, *a.result());
    os_ << " = " << to_string(a.op()) << " ";
    visit(*this, *a.a());
    os_ << ", ";
    visit(*this, *a.b());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(cast_inst &c) {
    visit(*this, *c.result());
    os_ << " = cast ";
    visit(*this, *c.a());
    os_ << " : ";
    visit(*this, *c.a()->ty());
    os_ << " -> ";
    visit(*this, *c.result()->ty());
}

void ir_dumper::operator()(compare_inst &a) {
    visit(*this, *a.result());
    os_ << " = cmp." << to_string(a.cond()) << " ";
    visit(*this, *a.a());
    os_ << ", ";
    visit(*this, *a.b());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(expand_inst &e) {
    visit(*this, *e.result());
    os_ << " = expand ";
    visit(*this, *e.operand());
    os_ << "[" << e.mode() << "->";
    do_with_infix(
        e.expand_shape().begin(), e.expand_shape().end(), [this](auto &i) { visit(*this, *i); },
        "x");
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(fuse_inst &f) {
    visit(*this, *f.result());
    os_ << " = fuse ";
    visit(*this, *f.operand());
    os_ << "[" << f.from() << "," << f.to() << "]";
    os_ << " : ";
    visit(*this, *f.operand()->ty());
}

void ir_dumper::operator()(load_inst &e) {
    visit(*this, *e.result());
    os_ << " = load ";
    visit(*this, *e.operand());
    os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto &i) { visit(*this, *i); });
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(group_id_inst &g) {
    visit(*this, *g.result());
    os_ << " = group_id";
}

void ir_dumper::operator()(group_size_inst &g) {
    visit(*this, *g.result());
    os_ << " = group_size";
}

void ir_dumper::operator()(lifetime_stop_inst &l) {
    os_ << "lifetime_stop ";
    visit(*this, *l.object());
}

void ir_dumper::operator()(gemm_inst &g) {
    os_ << "gemm";
    os_ << "." << to_string(g.tA());
    os_ << "." << to_string(g.tB()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst &>(g));
}

void ir_dumper::operator()(gemv_inst &g) {
    os_ << "gemv";
    os_ << "." << to_string(g.tA()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst &>(g));
}

void ir_dumper::operator()(ger_inst &g) {
    os_ << "ger ";
    dump_blas_a3(static_cast<blas_a3_inst &>(g));
}

void ir_dumper::operator()(for_inst &p) {
    os_ << "for ";
    visit(*this, *p.loop_var());
    os_ << "=";
    visit(*this, *p.from());
    os_ << ",";
    visit(*this, *p.to());
    if (p.step()) {
        os_ << ",";
        visit(*this, *p.step());
    }
    os_ << " : ";
    visit(*this, *p.loop_var()->ty());
    os_ << " ";
    visit(*this, *p.body());
}

void ir_dumper::operator()(foreach_inst &p) {
    os_ << "foreach ";
    visit(*this, *p.loop_var());
    os_ << "=";
    visit(*this, *p.from());
    os_ << ",";
    visit(*this, *p.to());
    os_ << " : ";
    visit(*this, *p.loop_var()->ty());
    os_ << " ";
    visit(*this, *p.body());
}

void ir_dumper::operator()(hadamard_inst &g) {
    os_ << "hadamard ";
    dump_blas_a3(static_cast<blas_a3_inst &>(g));
}

void ir_dumper::operator()(if_inst &in) {
    os_ << "if ";
    visit(*this, *in.condition());
    os_ << " ";
    visit(*this, *in.then());
    if (in.otherwise()) {
        os_ << " else ";
        visit(*this, *in.otherwise());
    }
}

void ir_dumper::operator()(neg_inst &a) {
    os_ << "neg ";
    visit(*this, *a.a());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(size_inst &s) {
    visit(*this, *s.result());
    os_ << " = size ";
    visit(*this, *s.operand());
    os_ << "[" << s.mode() << "]";
    os_ << " : ";
    visit(*this, *s.operand()->ty());
}

void ir_dumper::operator()(subview_inst &s) {
    visit(*this, *s.result());
    os_ << " = subview ";
    visit(*this, *s.operand());
    os_ << "[";
    do_with_infix(s.slices().begin(), s.slices().end(), [this](auto &i) {
        visit(*this, *i.first);
        if (i.second) {
            os_ << ":";
            visit(*this, *i.second);
        }
    });
    os_ << "]";
    os_ << " : ";
    visit(*this, *s.operand()->ty());
    os_ << " ; -> ";
    visit(*this, *s.result()->ty());
}

void ir_dumper::operator()(store_inst &e) {
    os_ << "store ";
    visit(*this, *e.val());
    os_ << ", ";
    visit(*this, *e.operand());
    os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto &i) { visit(*this, *i); });
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(sum_inst &a) {
    os_ << "sum";
    os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst &>(a));
}

void ir_dumper::operator()(yield_inst &y) {
    os_ << "yield ";
    do_with_infix(y.vals().begin(), y.vals().end(), [this](auto &i) { visit(*this, *i); });
    os_ << " : ";
    do_with_infix(y.vals().begin(), y.vals().end(), [this](auto &i) { visit(*this, *i->ty()); });
}

/* Region nodes */
void ir_dumper::operator()(rgn &b) {
    os_ << "{" << std::endl;
    ++lvl_;
    auto ind = indent();
    for (auto &s : b.insts()) {
        os_ << ind;
        visit(*this, *s);
        os_ << std::endl;
    }
    --lvl_;
    os_ << indent() << "}";
}

/* Function nodes */
void ir_dumper::operator()(prototype &p) {
    os_ << "func @" << p.name() << "(";
    std::string infix = ",\n       ";
    infix += std::string(p.name().size(), ' ');
    do_with_infix(
        p.args().begin(), p.args().end(),
        [this](auto &a) {
            os_ << "%" << a->name();
            os_ << ": ";
            visit(*this, *a->ty());
        },
        infix);
    os_ << ")";
}

void ir_dumper::operator()(function &fn) {
    visit(*this, *fn.prototype());
    os_ << " ";
    auto const sgs = fn.subgroup_size();
    auto const wgs = fn.work_group_size();
    if (sgs != 0) {
        os_ << "subgroup_size(" << sgs << ") ";
    }
    if (wgs[0] != 0 && wgs[1] != 0) {
        os_ << "work_group_size(" << wgs[0] << "," << wgs[1] << ") ";
    }
    visit(*this, *fn.body());
    os_ << std::endl;
}

/* Program nodes */
void ir_dumper::operator()(program &p) {
    for (auto &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
