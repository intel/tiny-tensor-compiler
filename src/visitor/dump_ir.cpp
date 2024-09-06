// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/dump_ir.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace tinytc {

ir_dumper::ir_dumper(std::ostream &os) : os_(os) {}

/* Data type nodes */
void ir_dumper::operator()(void_data_type const &) { os_ << "void"; }
void ir_dumper::operator()(group_data_type const &g) {
    os_ << "group<";
    visit(*this, *g.ty());
    os_ << ">";
}
void ir_dumper::operator()(memref_data_type const &d) {
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
        do_with_infix(d.stride().begin(), d.stride().end(), [&](auto const &a) { val(a); });
        os_ << ">";
    }
    os_ << ">";
}
void ir_dumper::operator()(scalar_data_type const &s) { os_ << to_string(s.ty()); }

/* Value nodes */
void ir_dumper::operator()(float_imm const &v) {
    auto flags = os_.flags();
    os_ << std::hexfloat << v.value();
    os_.flags(flags);
}
void ir_dumper::operator()(int_imm const &v) {
    if (is_dynamic_value(v.value())) {
        os_ << "?";
    } else {
        os_ << v.value();
    }
}
void ir_dumper::operator()(val const &v) {
    os_ << "%" << v.name();
    auto const slot = tracker_.get_slot(v);
    if (slot >= 0) {
        os_ << slot;
    }
}

/* Inst nodes */
void ir_dumper::dump_blas_a2(blas_a2_inst const &g) {
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

void ir_dumper::dump_blas_a3(blas_a3_inst const &g) {
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

void ir_dumper::operator()(alloca_inst const &a) {
    visit(*this, *a.result());
    os_ << " = alloca -> ";
    visit(*this, *a.result()->ty());
}

void ir_dumper::operator()(axpby_inst const &a) {
    os_ << "axpby";
    os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void ir_dumper::operator()(arith_inst const &a) {
    visit(*this, *a.result());
    os_ << " = arith." << to_string(a.operation()) << " ";
    visit(*this, *a.a());
    os_ << ", ";
    visit(*this, *a.b());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(arith_unary_inst const &a) {
    visit(*this, *a.result());
    os_ << " = arith." << to_string(a.operation()) << " ";
    visit(*this, *a.a());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(barrier_inst const &) { os_ << "barrier"; }

void ir_dumper::operator()(cast_inst const &c) {
    visit(*this, *c.result());
    os_ << " = cast ";
    visit(*this, *c.a());
    os_ << " : ";
    visit(*this, *c.a()->ty());
    os_ << " -> ";
    visit(*this, *c.result()->ty());
}

void ir_dumper::operator()(compare_inst const &a) {
    visit(*this, *a.result());
    os_ << " = cmp." << to_string(a.cond()) << " ";
    visit(*this, *a.a());
    os_ << ", ";
    visit(*this, *a.b());
    os_ << " : ";
    visit(*this, *a.a()->ty());
}

void ir_dumper::operator()(expand_inst const &e) {
    visit(*this, *e.result());
    os_ << " = expand ";
    visit(*this, *e.operand());
    os_ << "[" << e.mode() << "->";
    do_with_infix(
        e.expand_shape().begin(), e.expand_shape().end(),
        [this](auto const &i) { visit(*this, *i); }, "x");
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(fuse_inst const &f) {
    visit(*this, *f.result());
    os_ << " = fuse ";
    visit(*this, *f.operand());
    os_ << "[" << f.from() << "," << f.to() << "]";
    os_ << " : ";
    visit(*this, *f.operand()->ty());
}

void ir_dumper::operator()(load_inst const &e) {
    visit(*this, *e.result());
    os_ << " = load ";
    visit(*this, *e.operand());
    os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { visit(*this, *i); });
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(group_id_inst const &g) {
    visit(*this, *g.result());
    os_ << " = group_id";
}

void ir_dumper::operator()(group_size_inst const &g) {
    visit(*this, *g.result());
    os_ << " = group_size";
}

void ir_dumper::operator()(lifetime_stop_inst const &l) {
    os_ << "lifetime_stop ";
    visit(*this, *l.object());
}

void ir_dumper::operator()(gemm_inst const &g) {
    os_ << "gemm";
    os_ << "." << to_string(g.tA());
    os_ << "." << to_string(g.tB()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void ir_dumper::operator()(gemv_inst const &g) {
    os_ << "gemv";
    os_ << "." << to_string(g.tA()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void ir_dumper::operator()(ger_inst const &g) {
    os_ << "ger ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void ir_dumper::operator()(for_inst const &p) {
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

void ir_dumper::operator()(foreach_inst const &p) {
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

void ir_dumper::operator()(hadamard_inst const &g) {
    os_ << "hadamard ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void ir_dumper::operator()(if_inst const &in) {
    os_ << "if ";
    visit(*this, *in.condition());
    os_ << " ";
    visit(*this, *in.then());
    if (in.otherwise()) {
        os_ << " else ";
        visit(*this, *in.otherwise());
    }
}

void ir_dumper::operator()(num_subgroups_inst const &sg) {
    visit(*this, *sg.result());
    os_ << " = num_subgroups";
}

void ir_dumper::operator()(parallel_inst const &p) {
    os_ << "parallel ";
    visit(*this, *p.body());
}

void ir_dumper::operator()(size_inst const &s) {
    visit(*this, *s.result());
    os_ << " = size ";
    visit(*this, *s.operand());
    os_ << "[" << s.mode() << "]";
    os_ << " : ";
    visit(*this, *s.operand()->ty());
}

void ir_dumper::operator()(subgroup_id_inst const &sg) {
    visit(*this, *sg.result());
    os_ << " = subgroup_id";
}

void ir_dumper::operator()(subgroup_local_id_inst const &sg) {
    visit(*this, *sg.result());
    os_ << " = subgroup_local_id";
}

void ir_dumper::operator()(subgroup_size_inst const &sg) {
    visit(*this, *sg.result());
    os_ << " = subgroup_size";
}

void ir_dumper::operator()(subview_inst const &s) {
    visit(*this, *s.result());
    os_ << " = subview ";
    visit(*this, *s.operand());
    os_ << "[";
    auto irange = std::ranges::iota_view{std::size_t{0}, s.offset_list().size()};
    do_with_infix(irange.begin(), irange.end(), [&](auto const &i) {
        visit(*this, *s.offset_list()[i]);
        auto &size = s.size_list()[i];
        if (size) {
            os_ << ":";
            visit(*this, *size);
        }
    });
    os_ << "]";
    os_ << " : ";
    visit(*this, *s.operand()->ty());
    os_ << " ; -> ";
    visit(*this, *s.result()->ty());
}

void ir_dumper::operator()(store_inst const &e) {
    os_ << "store ";
    visit(*this, *e.val());
    os_ << ", ";
    visit(*this, *e.operand());
    os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { visit(*this, *i); });
    os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void ir_dumper::operator()(sum_inst const &a) {
    os_ << "sum";
    os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void ir_dumper::operator()(yield_inst const &y) {
    os_ << "yield ";
    do_with_infix(y.op_begin(), y.op_end(), [this](auto const &i) { visit(*this, *i); });
    os_ << " : ";
    do_with_infix(y.op_begin(), y.op_end(), [this](auto const &i) { visit(*this, *i->ty()); });
}

/* Region nodes */
void ir_dumper::operator()(rgn const &b) {
    os_ << "{" << std::endl;
    ++lvl_;
    auto ind = indent();
    for (auto const &s : b.insts()) {
        os_ << ind;
        visit(*this, *s);
        os_ << std::endl;
    }
    --lvl_;
    os_ << indent() << "}";
}

/* Function nodes */
void ir_dumper::operator()(prototype const &p) {
    os_ << "func @" << p.name() << "(";
    std::string infix = ",\n       ";
    infix += std::string(p.name().size(), ' ');
    do_with_infix(
        p.args().begin(), p.args().end(),
        [this](auto const &a) {
            visit(*this, *a);
            os_ << ": ";
            visit(*this, *a->ty());
        },
        infix);
    os_ << ")";
}

void ir_dumper::operator()(function const &fn) {
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
void ir_dumper::operator()(program const &p) {
    visit(tracker_, p);
    for (auto const &decl : p.declarations()) {
        visit(*this, *decl);
    }
}

} // namespace tinytc
