// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_ir.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace tinytc {

dump_ir_pass::dump_ir_pass(std::ostream &os, int level_limit) : os_(&os), lvl_limit_(level_limit) {}

/* Data type nodes */
void dump_ir_pass::operator()(void_data_type const &) { *os_ << "void"; }
void dump_ir_pass::operator()(group_data_type const &g) {
    *os_ << "group<";
    visit(*this, *g.ty());
    *os_ << ">";
}
void dump_ir_pass::operator()(memref_data_type const &d) {
    auto const val = [&](std::int64_t v) -> std::ostream & {
        if (is_dynamic_value(v)) {
            return *os_ << "?";
        }
        return *os_ << v;
    };
    *os_ << "memref<" << to_string(d.element_ty());
    for (auto const &s : d.shape()) {
        *os_ << "x";
        val(s);
    }
    if (!d.is_canonical_stride()) {
        *os_ << ",strided<";
        do_with_infix(d.stride().begin(), d.stride().end(), [&](auto const &a) { val(a); });
        *os_ << ">";
    }
    if (d.addrspace() != address_space::global) {
        *os_ << "," << to_string(d.addrspace());
    }
    *os_ << ">";
}
void dump_ir_pass::operator()(scalar_data_type const &s) { *os_ << to_string(s.ty()); }

/* Value nodes */
void dump_ir_pass::operator()(float_imm const &v) {
    auto flags = os_->flags();
    *os_ << std::hexfloat << v.value();
    os_->flags(flags);
}
void dump_ir_pass::operator()(int_imm const &v) {
    if (is_dynamic_value(v.value())) {
        *os_ << "?";
    } else {
        *os_ << v.value();
    }
}
void dump_ir_pass::operator()(val const &v) {
    *os_ << "%" << v.name();
    auto const slot = tracker_.get_slot(v);
    if (slot >= 0) {
        *os_ << slot;
    }
}

/* Inst nodes */
void dump_ir_pass::dump_blas_a2(blas_a2_inst const &g) {
    visit(*this, *g.alpha());
    *os_ << ", ";
    visit(*this, *g.A());
    *os_ << ", ";
    visit(*this, *g.beta());
    *os_ << ", ";
    visit(*this, *g.B());
    *os_ << " : ";
    visit(*this, *g.alpha()->ty());
    *os_ << ", ";
    visit(*this, *g.A()->ty());
    *os_ << ", ";
    visit(*this, *g.beta()->ty());
    *os_ << ", ";
    visit(*this, *g.B()->ty());
}

void dump_ir_pass::dump_blas_a3(blas_a3_inst const &g) {
    visit(*this, *g.alpha());
    *os_ << ", ";
    visit(*this, *g.A());
    *os_ << ", ";
    visit(*this, *g.B());
    *os_ << ", ";
    visit(*this, *g.beta());
    *os_ << ", ";
    visit(*this, *g.C());
    *os_ << " : ";
    visit(*this, *g.alpha()->ty());
    *os_ << ", ";
    visit(*this, *g.A()->ty());
    *os_ << ", ";
    visit(*this, *g.B()->ty());
    *os_ << ", ";
    visit(*this, *g.beta()->ty());
    *os_ << ", ";
    visit(*this, *g.C()->ty());
}

void dump_ir_pass::operator()(alloca_inst const &a) {
    visit(*this, *a.result());
    *os_ << " = alloca -> ";
    visit(*this, *a.result()->ty());
}

void dump_ir_pass::operator()(axpby_inst const &a) {
    *os_ << "axpby";
    *os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void dump_ir_pass::operator()(arith_inst const &a) {
    visit(*this, *a.result());
    *os_ << " = arith." << to_string(a.operation()) << " ";
    visit(*this, *a.a());
    *os_ << ", ";
    visit(*this, *a.b());
    *os_ << " : ";
    visit(*this, *a.a()->ty());
}

void dump_ir_pass::operator()(arith_unary_inst const &a) {
    visit(*this, *a.result());
    *os_ << " = arith." << to_string(a.operation()) << " ";
    visit(*this, *a.a());
    *os_ << " : ";
    visit(*this, *a.a()->ty());
}

void dump_ir_pass::operator()(barrier_inst const &b) {
    *os_ << "barrier";
    if (b.has_fence(address_space::global)) {
        *os_ << ".global";
    }
    if (b.has_fence(address_space::local)) {
        *os_ << ".local";
    }
}

void dump_ir_pass::operator()(cast_inst const &c) {
    visit(*this, *c.result());
    *os_ << " = cast ";
    visit(*this, *c.a());
    *os_ << " : ";
    visit(*this, *c.a()->ty());
    *os_ << " -> ";
    visit(*this, *c.result()->ty());
}

void dump_ir_pass::operator()(compare_inst const &a) {
    visit(*this, *a.result());
    *os_ << " = cmp." << to_string(a.cond()) << " ";
    visit(*this, *a.a());
    *os_ << ", ";
    visit(*this, *a.b());
    *os_ << " : ";
    visit(*this, *a.a()->ty());
}

void dump_ir_pass::operator()(constant_inst const &c) {
    visit(*this, *c.result());
    *os_ << " = constant ";
    std::visit(overloaded{[&](std::int64_t i) {
                              if (is_dynamic_value(i)) {
                                  *os_ << "?";
                              } else {
                                  *os_ << i;
                              }
                          },
                          [&](double d) {
                              auto flags = os_->flags();
                              *os_ << std::hexfloat << d;
                              os_->flags(flags);
                          }},
               c.value());
    *os_ << " -> ";
    visit(*this, *c.result()->ty());
}

void dump_ir_pass::operator()(expand_inst const &e) {
    visit(*this, *e.result());
    *os_ << " = expand ";
    visit(*this, *e.operand());
    *os_ << "[" << e.expanded_mode() << "->";
    auto const &ses = e.static_expand_shape();
    auto es = e.expand_shape();
    for (std::size_t i = 0, j = 0; i < ses.size(); ++i) {
        if (i != 0) {
            *os_ << " x ";
        }
        if (is_dynamic_value(ses[i])) {
            visit(*this, *es[j++]);
        } else {
            *os_ << ses[i];
        }
    }
    *os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void dump_ir_pass::operator()(fuse_inst const &f) {
    visit(*this, *f.result());
    *os_ << " = fuse ";
    visit(*this, *f.operand());
    *os_ << "[" << f.from() << "," << f.to() << "]";
    *os_ << " : ";
    visit(*this, *f.operand()->ty());
}

void dump_ir_pass::operator()(load_inst const &e) {
    visit(*this, *e.result());
    *os_ << " = load ";
    visit(*this, *e.operand());
    *os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { visit(*this, *i); });
    *os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void dump_ir_pass::operator()(group_id_inst const &g) {
    visit(*this, *g.result());
    *os_ << " = group_id";
}

void dump_ir_pass::operator()(group_size_inst const &g) {
    visit(*this, *g.result());
    *os_ << " = group_size";
}

void dump_ir_pass::operator()(lifetime_stop_inst const &l) {
    *os_ << "lifetime_stop ";
    visit(*this, *l.object());
}

void dump_ir_pass::operator()(gemm_inst const &g) {
    *os_ << "gemm";
    *os_ << "." << to_string(g.tA());
    *os_ << "." << to_string(g.tB()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(gemv_inst const &g) {
    *os_ << "gemv";
    *os_ << "." << to_string(g.tA()) << " ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(ger_inst const &g) {
    *os_ << "ger ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(for_inst const &p) {
    *os_ << "for ";
    visit(*this, *p.loop_var());
    *os_ << "=";
    visit(*this, *p.from());
    *os_ << ",";
    visit(*this, *p.to());
    if (p.step()) {
        *os_ << ",";
        visit(*this, *p.step());
    }
    *os_ << " : ";
    visit(*this, *p.loop_var()->ty());
    *os_ << " ";
    dump_region(p.body());
}

void dump_ir_pass::operator()(foreach_inst const &p) {
    *os_ << "foreach ";
    visit(*this, *p.loop_var());
    *os_ << "=";
    visit(*this, *p.from());
    *os_ << ",";
    visit(*this, *p.to());
    *os_ << " : ";
    visit(*this, *p.loop_var()->ty());
    *os_ << " ";
    dump_region(p.body());
}

void dump_ir_pass::operator()(hadamard_inst const &g) {
    *os_ << "hadamard ";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(if_inst const &in) {
    *os_ << "if ";
    visit(*this, *in.condition());
    *os_ << " ";
    dump_region(in.then());
    if (in.has_otherwise()) {
        *os_ << " else ";
        dump_region(in.otherwise());
    }
}

void dump_ir_pass::operator()(num_subgroups_inst const &sg) {
    visit(*this, *sg.result());
    *os_ << " = num_subgroups";
}

void dump_ir_pass::operator()(parallel_inst const &p) {
    *os_ << "parallel ";
    dump_region(p.body());
}

void dump_ir_pass::operator()(size_inst const &s) {
    visit(*this, *s.result());
    *os_ << " = size ";
    visit(*this, *s.operand());
    *os_ << "[" << s.mode() << "]";
    *os_ << " : ";
    visit(*this, *s.operand()->ty());
}

void dump_ir_pass::operator()(subgroup_id_inst const &sg) {
    visit(*this, *sg.result());
    *os_ << " = subgroup_id";
}

void dump_ir_pass::operator()(subgroup_local_id_inst const &sg) {
    visit(*this, *sg.result());
    *os_ << " = subgroup_local_id";
}

void dump_ir_pass::operator()(subgroup_size_inst const &sg) {
    visit(*this, *sg.result());
    *os_ << " = subgroup_size";
}

void dump_ir_pass::operator()(subview_inst const &s) {
    visit(*this, *s.result());
    *os_ << " = subview ";
    visit(*this, *s.operand());
    *os_ << "[";
    auto dyn_offsets = s.offsets();
    auto dyn_sizes = s.sizes();
    for (std::size_t i = 0, joffset = 0, jsize = 0; i < s.static_offsets().size(); ++i) {
        if (i != 0) {
            *os_ << ",";
        }
        auto offset = s.static_offsets()[i];
        if (is_dynamic_value(offset)) {
            visit(*this, *dyn_offsets[joffset++]);
        } else {
            *os_ << offset;
        }
        auto size = s.static_sizes()[i];
        if (size > 0 || is_dynamic_value(size)) {
            *os_ << ":";
            if (is_dynamic_value(size)) {
                visit(*this, *dyn_sizes[jsize++]);
            } else {
                *os_ << size;
            }
        }
    }
    *os_ << "]";
    *os_ << " : ";
    visit(*this, *s.operand()->ty());
    *os_ << " ; -> ";
    visit(*this, *s.result()->ty());
}

void dump_ir_pass::operator()(store_inst const &e) {
    *os_ << "store ";
    visit(*this, *e.val());
    *os_ << ", ";
    visit(*this, *e.operand());
    *os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { visit(*this, *i); });
    *os_ << "] : ";
    visit(*this, *e.operand()->ty());
}

void dump_ir_pass::operator()(sum_inst const &a) {
    *os_ << "sum";
    *os_ << "." << to_string(a.tA()) << " ";
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void dump_ir_pass::operator()(yield_inst const &y) {
    *os_ << "yield ";
    if (y.num_operands() > 0) {
        do_with_infix(y.op_begin(), y.op_end(), [this](auto const &i) { visit(*this, *i); }, ", ");
        *os_ << " : ";
        do_with_infix(
            y.op_begin(), y.op_end(), [this](auto const &i) { visit(*this, *i->ty()); }, ", ");
    } else {
        *os_ << ":";
    }
}

void dump_ir_pass::dump_region(region_node const &reg) {
    if (lvl_ < lvl_limit_) {
        *os_ << "{" << std::endl;
        ++lvl_;
        auto ind = indent();
        for (auto const &i : reg) {
            *os_ << ind;
            visit(*this, i);
            *os_ << std::endl;
        }
        --lvl_;
        *os_ << indent() << "}";
    } else {
        *os_ << "{...}";
    }
}

void dump_ir_pass::run_on_function(function_node const &fn) {
    *os_ << "func @" << fn.name() << "(";
    std::string infix = ",\n       ";
    infix += std::string(fn.name().size(), ' ');
    do_with_infix(
        fn.args().begin(), fn.args().end(),
        [this](auto const &a) {
            visit(*this, *a);
            *os_ << ": ";
            visit(*this, *a->ty());
        },
        infix);
    *os_ << ") ";
    auto const sgs = fn.subgroup_size();
    auto const wgs = fn.work_group_size();
    if (sgs != 0) {
        *os_ << "subgroup_size(" << sgs << ") ";
    }
    if (wgs[0] != 0 && wgs[1] != 0) {
        *os_ << "work_group_size(" << wgs[0] << "," << wgs[1] << ") ";
    }
    dump_region(fn.body());
    *os_ << std::endl;
}

void dump_ir_pass::run_on_region(region_node const &reg) { dump_region(reg); }
void dump_ir_pass::run_on_instruction(inst_node const &in) { visit(*this, in); }

} // namespace tinytc
