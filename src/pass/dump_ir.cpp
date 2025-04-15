// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_ir.hpp"
#include "support/casting.hpp"
#include "support/fnv1a.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <complex>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <variant>
#include <vector>

namespace tinytc {

dump_ir_pass::dump_ir_pass(std::ostream &os, int level_limit) : os_(&os), lvl_limit_(level_limit) {}

/* Attribute nodes */
void dump_ir_pass::operator()(array_attr const &a) {
    *os_ << "[";
    do_with_infix(a.begin(), a.end(), [&](auto const &a) { visit(*this, *a); });
    *os_ << "]";
}
void dump_ir_pass::operator()(boolean_attr const &a) { *os_ << (a.value() ? "true" : "false"); }
void dump_ir_pass::operator()(dictionary_attr const &a) {
    auto const is_keyword = [](std::string_view str) {
        switch (fnv1a(str)) {
        case "alignment"_fnv1a:
        case "shape_gcd"_fnv1a:
        case "stride_gcd"_fnv1a:
        case "subgroup_size"_fnv1a:
        case "unroll"_fnv1a:
        case "work_group_size"_fnv1a:
            return true;
        default:
            return false;
        }
    };
    auto const dump_name = [&](attr a) {
        if (auto s = dyn_cast<string_attr>(a); s) {
            if (is_keyword(s->str())) {
                *os_ << s->str();
            } else {
                this->operator()(*s);
            }
        } else {
            throw status::ir_expected_string_attribute;
        }
    };
    *os_ << "{";
    do_with_infix(
        a.begin(), a.end(),
        [&](auto const &a) {
            dump_name(a.name);
            *os_ << "=";
            visit(*this, *a.attr);
        },
        ", ");
    *os_ << "}";
}
void dump_ir_pass::operator()(integer_attr const &a) { *os_ << a.value(); }
void dump_ir_pass::operator()(string_attr const &a) { *os_ << "\"" << a.str() << "\""; }

/* Data type nodes */
void dump_ir_pass::operator()(void_data_type const &) { *os_ << "void"; }
void dump_ir_pass::operator()(boolean_data_type const &) { *os_ << "bool"; }
void dump_ir_pass::operator()(coopmatrix_data_type const &ct) {
    *os_ << "coopmatrix<";
    visit(*this, *ct.ty());
    *os_ << "x" << ct.rows() << "x" << ct.cols() << "," << to_string(ct.use()) << ">";
}
void dump_ir_pass::operator()(group_data_type const &g) {
    auto const val = [&](std::int64_t v) -> std::ostream & {
        if (is_dynamic_value(v)) {
            return *os_ << "?";
        }
        return *os_ << v;
    };
    *os_ << "group<";
    visit(*this, *g.ty());
    *os_ << "x";
    val(g.size());
    if (g.offset() != 0) {
        *os_ << ", offset: ";
        val(g.offset());
    }
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
void dump_ir_pass::dump_val(value_node const &v) {
    *os_ << "%" << v.name();
    auto const slot = tracker_.get_slot(v);
    if (slot >= 0) {
        *os_ << slot;
    }
}

/* Inst nodes */
void dump_ir_pass::dump_blas_a2(blas_a2_inst const &g) {
    if (g.atomic()) {
        *os_ << ".atomic";
    }
    *os_ << ' ';
    dump_val(g.alpha());
    *os_ << ", ";
    dump_val(g.A());
    *os_ << ", ";
    dump_val(g.beta());
    *os_ << ", ";
    dump_val(g.B());
}

void dump_ir_pass::dump_blas_a3(blas_a3_inst const &g) {
    if (g.atomic()) {
        *os_ << ".atomic";
    }
    *os_ << ' ';
    dump_val(g.alpha());
    *os_ << ", ";
    dump_val(g.A());
    *os_ << ", ";
    dump_val(g.B());
    *os_ << ", ";
    dump_val(g.beta());
    *os_ << ", ";
    dump_val(g.C());
}

void dump_ir_pass::operator()(alloca_inst const &a) {
    dump_val(a.result(0));
    *os_ << " = alloca : ";
    visit(*this, *a.result()->ty());
}

void dump_ir_pass::operator()(axpby_inst const &a) {
    *os_ << "axpby";
    *os_ << "." << to_string(a.tA());
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void dump_ir_pass::operator()(arith_inst const &a) {
    dump_val(a.result(0));
    *os_ << " = arith." << to_string(a.operation()) << " ";
    dump_val(a.a());
    *os_ << ", ";
    dump_val(a.b());
    *os_ << " : ";
    visit(*this, *a.result(0).ty());
}

void dump_ir_pass::operator()(arith_unary_inst const &a) {
    dump_val(a.result(0));
    *os_ << " = arith." << to_string(a.operation()) << " ";
    dump_val(a.a());
    *os_ << " : ";
    visit(*this, *a.result(0).ty());
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

void dump_ir_pass::operator()(builtin_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = builtin." << to_string(in.builtin_type()) << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(cast_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = cast ";
    dump_val(c.a());
    *os_ << " : ";
    visit(*this, *c.result(0).ty());
}

void dump_ir_pass::operator()(compare_inst const &a) {
    dump_val(a.result(0));
    *os_ << " = cmp." << to_string(a.cond()) << " ";
    dump_val(a.a());
    *os_ << ", ";
    dump_val(a.b());
    *os_ << " : ";
    visit(*this, *a.result(0).ty());
}

void dump_ir_pass::operator()(constant_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = constant ";
    std::visit(overloaded{
                   [&](bool b) { *os_ << (b ? "true" : "false"); },
                   [&](std::int64_t i) {
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
                   },
                   [&](std::complex<double> d) {
                       auto flags = os_->flags();
                       *os_ << std::hexfloat << "[" << d.real() << "," << d.imag() << "]";
                       os_->flags(flags);
                   },
               },
               c.value());
    *os_ << " : ";
    visit(*this, *c.result()->ty());
}

void dump_ir_pass::operator()(cooperative_matrix_apply_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = cooperative_matrix_apply (";
    dump_val(c.row());
    *os_ << ",";
    dump_val(c.col());
    *os_ << ",";
    dump_val(c.val());
    *os_ << ") in ";
    dump_val(c.a());
    *os_ << " -> ";
    visit(*this, *c.result(0).ty());
    dump_region(c.body());
}

void dump_ir_pass::operator()(cooperative_matrix_load_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = cooperative_matrix_load";
    *os_ << "." << to_string(c.t());
    if (c.checked() != checked_flag::none) {
        *os_ << "." << to_string(c.checked());
    }
    *os_ << " ";
    dump_val(c.operand());
    *os_ << "[";
    dump_val(c.pos0());
    *os_ << ",";
    dump_val(c.pos1());
    *os_ << "] : ";
    visit(*this, *c.result(0).ty());
}

void dump_ir_pass::operator()(cooperative_matrix_mul_add_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = cooperative_matrix_mul_add ";
    dump_val(c.a());
    *os_ << ", ";
    dump_val(c.b());
    *os_ << ", ";
    dump_val(c.c());
    *os_ << " : ";
    visit(*this, *c.result(0).ty());
}

void dump_ir_pass::operator()(cooperative_matrix_prefetch_inst const &c) {
    *os_ << "cooperative_matrix_prefetch ";
    *os_ << c.cache_level();
    *os_ << ", ";
    dump_val(c.operand());
    *os_ << "[";
    dump_val(c.pos0());
    *os_ << ",";
    dump_val(c.pos1());
    *os_ << "], ";
    *os_ << c.rows();
    *os_ << ", ";
    *os_ << c.cols();
}

void dump_ir_pass::operator()(cooperative_matrix_scale_inst const &c) {
    dump_val(c.result(0));
    *os_ << " = cooperative_matrix_scale ";
    dump_val(c.a());
    *os_ << ", ";
    dump_val(c.b());
    *os_ << " : ";
    visit(*this, *c.result(0).ty());
}

void dump_ir_pass::operator()(cooperative_matrix_store_inst const &c) {
    *os_ << "cooperative_matrix_store";
    if (c.checked() != checked_flag::none) {
        *os_ << "." << to_string(c.checked());
    }
    if (c.flag() != store_flag::regular) {
        *os_ << '.' << to_string(c.flag());
    }
    *os_ << " ";
    dump_val(c.val());
    *os_ << ", ";
    dump_val(c.operand());
    *os_ << "[";
    dump_val(c.pos0());
    *os_ << ",";
    dump_val(c.pos1());
    *os_ << "]";
}

void dump_ir_pass::operator()(cumsum_inst const &in) {
    *os_ << "cumsum";
    if (in.atomic()) {
        *os_ << ".atomic";
    }
    *os_ << ' ';
    dump_val(in.alpha());
    *os_ << ", ";
    dump_val(in.A());
    *os_ << ", " << in.mode() << ", ";
    dump_val(in.beta());
    *os_ << ", ";
    dump_val(in.B());
}

void dump_ir_pass::operator()(expand_inst const &e) {
    dump_val(e.result(0));
    *os_ << " = expand ";
    dump_val(e.operand());
    *os_ << "[" << e.expanded_mode() << "->";
    auto const &ses = e.static_expand_shape();
    auto es = e.expand_shape();
    for (std::size_t i = 0, j = 0; i < ses.size(); ++i) {
        if (i != 0) {
            *os_ << " x ";
        }
        if (is_dynamic_value(ses[i])) {
            dump_val(es[j++]);
        } else {
            *os_ << ses[i];
        }
    }
    *os_ << "] : ";
    visit(*this, *e.result(0).ty());
}

void dump_ir_pass::operator()(fuse_inst const &f) {
    dump_val(f.result(0));
    *os_ << " = fuse ";
    dump_val(f.operand());
    *os_ << "[" << f.from() << "," << f.to() << "]";
    *os_ << " : ";
    visit(*this, *f.result(0).ty());
}

void dump_ir_pass::operator()(load_inst const &e) {
    dump_val(e.result(0));
    *os_ << " = load ";
    dump_val(e.operand());
    *os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { dump_val(i); });
    *os_ << "] : ";
    visit(*this, *e.result(0).ty());
}

void dump_ir_pass::operator()(lifetime_stop_inst const &l) {
    *os_ << "lifetime_stop ";
    dump_val(l.object());
}

void dump_ir_pass::operator()(gemm_inst const &g) {
    *os_ << "gemm";
    *os_ << "." << to_string(g.tA());
    *os_ << "." << to_string(g.tB());
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(gemv_inst const &g) {
    *os_ << "gemv";
    *os_ << "." << to_string(g.tA());
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(ger_inst const &g) {
    *os_ << "ger";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(for_inst const &in) {
    if (in.num_results() > 0) {
        do_with_infix(in.result_begin(), in.result_end(), [this](auto const &i) { dump_val(i); });
        *os_ << " = ";
    }
    *os_ << "for ";
    dump_val(in.loop_var());
    *os_ << ":";
    visit(*this, *in.loop_var().ty());
    *os_ << "=";
    dump_val(in.from());
    *os_ << ",";
    dump_val(in.to());
    if (in.has_step()) {
        *os_ << ",";
        dump_val(in.step());
    }
    if (in.num_results() > 0) {
        *os_ << " init(";
        for (std::int64_t i = 0; i < in.num_results(); ++i) {
            if (i != 0) {
                *os_ << ",";
            }
            dump_val(in.iter_arg(i));
            *os_ << "=";
            dump_val(in.iter_init(i));
        }
        *os_ << ") -> (";
        do_with_infix(in.result_begin(), in.result_end(),
                      [this](auto const &i) { visit(*this, *i.ty()); });
        *os_ << ")";
    }
    *os_ << " ";
    dump_region(in.body());
    if (in.attr()) {
        *os_ << " ";
        visit(*this, *in.attr());
    }
}

void dump_ir_pass::operator()(foreach_inst const &in) {
    *os_ << "foreach (";
    do_with_infix(in.loop_vars().begin(), in.loop_vars().end(),
                  [this](auto const &i) { dump_val(i); });
    *os_ << "):";
    visit(*this, *in.loop_vars().begin()->ty());
    *os_ << "=(";
    do_with_infix(in.from().begin(), in.from().end(), [this](auto const &i) { dump_val(i); });
    *os_ << "),(";
    do_with_infix(in.to().begin(), in.to().end(), [this](auto const &i) { dump_val(i); });
    *os_ << ") ";
    dump_region(in.body());
}

void dump_ir_pass::operator()(hadamard_inst const &g) {
    *os_ << "hadamard";
    dump_blas_a3(static_cast<blas_a3_inst const &>(g));
}

void dump_ir_pass::operator()(if_inst const &in) {

    if (in.num_results() > 0) {
        do_with_infix(in.result_begin(), in.result_end(), [this](auto const &i) { dump_val(i); });
        *os_ << " = ";
    }
    *os_ << "if ";
    dump_val(in.condition());
    *os_ << " ";
    if (in.num_results() > 0) {
        *os_ << "-> (";
        do_with_infix(in.result_begin(), in.result_end(),
                      [this](auto const &i) { visit(*this, *i.ty()); });
        *os_ << ") ";
    }
    dump_region(in.then());
    if (!in.is_otherwise_empty()) {
        *os_ << " else ";
        dump_region(in.otherwise());
    }
}

void dump_ir_pass::operator()(math_unary_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = math." << to_string(in.operation()) << " ";
    dump_val(in.a());
    *os_ << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(parallel_inst const &p) {
    *os_ << "parallel ";
    dump_region(p.body());
}

void dump_ir_pass::operator()(size_inst const &s) {
    dump_val(s.result(0));
    *os_ << " = size ";
    dump_val(s.operand());
    *os_ << "[" << s.mode() << "]";
    *os_ << " : ";
    visit(*this, *s.result(0).ty());
}

void dump_ir_pass::operator()(subgroup_add_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = subgroup_add." << to_string(in.operation()) << " ";
    dump_val(in.a());
    *os_ << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(subgroup_broadcast_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = subgroup_broadcast ";
    dump_val(in.a());
    *os_ << ", ";
    dump_val(in.idx());
    *os_ << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(subgroup_max_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = subgroup_max." << to_string(in.operation()) << " ";
    dump_val(in.a());
    *os_ << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(subgroup_min_inst const &in) {
    dump_val(in.result(0));
    *os_ << " = subgroup_min." << to_string(in.operation()) << " ";
    dump_val(in.a());
    *os_ << " : ";
    visit(*this, *in.result(0).ty());
}

void dump_ir_pass::operator()(subview_inst const &s) {
    dump_val(s.result(0));
    *os_ << " = subview ";
    dump_val(s.operand());
    *os_ << "[";
    auto dyn_offsets = s.offsets();
    auto dyn_sizes = s.sizes();
    for (std::size_t i = 0, joffset = 0, jsize = 0; i < s.static_offsets().size(); ++i) {
        if (i != 0) {
            *os_ << ",";
        }
        auto offset = s.static_offsets()[i];
        if (is_dynamic_value(offset)) {
            dump_val(dyn_offsets[joffset++]);
        } else {
            *os_ << offset;
        }
        auto size = s.static_sizes()[i];
        if (size > 0 || is_dynamic_value(size)) {
            *os_ << ":";
            if (is_dynamic_value(size)) {
                dump_val(dyn_sizes[jsize++]);
            } else {
                *os_ << size;
            }
        }
    }
    *os_ << "] : ";
    visit(*this, *s.result(0).ty());
}

void dump_ir_pass::operator()(store_inst const &e) {
    *os_ << "store";
    if (e.flag() != store_flag::regular) {
        *os_ << '.' << to_string(e.flag());
    }
    *os_ << ' ';
    dump_val(e.val());
    *os_ << ", ";
    dump_val(e.operand());
    *os_ << "[";
    do_with_infix(e.index_list().begin(), e.index_list().end(),
                  [this](auto const &i) { dump_val(i); });
    *os_ << "]";
}

void dump_ir_pass::operator()(sum_inst const &a) {
    *os_ << "sum";
    *os_ << "." << to_string(a.tA());
    dump_blas_a2(static_cast<blas_a2_inst const &>(a));
}

void dump_ir_pass::operator()(yield_inst const &y) {
    *os_ << "yield (";
    if (y.num_operands() > 0) {
        do_with_infix(y.op_begin(), y.op_end(), [this](auto const &i) { dump_val(i); }, ", ");
    }
    *os_ << ")";
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
    init_slot_tracker(fn);

    *os_ << "func @" << fn.name() << "(";
    std::string infix = ",\n       ";
    infix += std::string(fn.name().size(), ' ');
    do_with_infix_enumerated(
        fn.params().begin(), fn.params().end(),
        [this, &fn](std::int32_t arg_no, auto const &a) {
            dump_val(a);
            *os_ << ": ";
            visit(*this, *a.ty());
            if (auto pa = fn.param_attr(arg_no); pa) {
                *os_ << " ";
                visit(*this, *pa);
            }
        },
        infix);
    *os_ << ")";
    if (fn.attr()) {
        *os_ << " attributes";
        visit(*this, *fn.attr());
    }
    *os_ << " ";
    dump_region(fn.body());
    *os_ << std::endl;
}

void dump_ir_pass::run_on_region(region_node const &reg) { dump_region(reg); }
void dump_ir_pass::run_on_instruction(inst_node const &in) { visit(*this, in); }

void dump_ir_pass::init_slot_tracker(function_node const &fn) {
    tracker_ = slot_tracker{};
    tracker_.run_on_function(fn);
}

} // namespace tinytc
