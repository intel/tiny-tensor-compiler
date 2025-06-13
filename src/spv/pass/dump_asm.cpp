// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/dump_asm.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "tinytc/tinytc.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

enum class LinkageType;

dump_asm_pass::dump_asm_pass(std::ostream &os) : os_(&os) {}

void dump_asm_pass::pre_visit(spv_inst const &in) {
    auto const num_digits = [](std::int64_t number) {
        std::int64_t d = 1;
        while (number /= 10) {
            ++d;
        }
        return d;
    };
    *os_ << std::endl;
    if (in.has_result_id()) {
        const auto id = in.id();

        for (int i = 0; i < rhs_indent - 4 - num_digits(id); ++i) {
            *os_ << ' ';
        }
        *os_ << "%" << id << " = ";
    } else {
        for (int i = 0; i < rhs_indent; ++i) {
            *os_ << ' ';
        }
    }
    *os_ << "Op" << to_string(in.opcode());
}

void dump_asm_pass::operator()(DecorationAttr const &da) {
    std::visit(overloaded{[&](auto const &a) { this->operator()(a); },
                          [&](std::pair<std::string, LinkageType> const &a) {
                              *os_ << " \"" << a.first << '"';
                              this->operator()(a.second);
                          }},
               da);
}
void dump_asm_pass::operator()(ExecutionModeAttr const &ea) {
    std::visit(
        overloaded{[&](std::int32_t const &a) { *os_ << " " << static_cast<std::uint32_t>(a); },
                   [&](std::array<std::int32_t, 3u> const &a) {
                       for (auto const &s : a) {
                           *os_ << " " << static_cast<uint32_t>(s);
                       }
                   }},
        ea);
}
void dump_asm_pass::operator()(LiteralContextDependentNumber const &l) {
    std::visit(overloaded{[&](std::int8_t const &l) {
                              *os_ << " "
                                   << static_cast<std::uint32_t>(static_cast<std::uint8_t>(l));
                          },
                          [&](std::signed_integral auto const &l) {
                              using unsigned_t = std::make_unsigned_t<std::decay_t<decltype(l)>>;
                              *os_ << " " << static_cast<unsigned_t>(l);
                          },
                          [&](std::floating_point auto const &l) {
                              auto flags = os_->flags();
                              *os_ << " " << std::hexfloat << l;
                              os_->flags(flags);
                          },
                          [&](half const &l) {
                              auto flags = os_->flags();
                              *os_ << " " << std::hexfloat << l;
                              os_->flags(flags);
                          }},
               l);
}
void dump_asm_pass::operator()(LiteralInteger const &l) {
    *os_ << " " << static_cast<std::make_unsigned_t<LiteralInteger>>(l);
}
void dump_asm_pass::operator()(LiteralString const &l) { *os_ << " \"" << l << '"'; }

void dump_asm_pass::operator()(PairIdRefIdRef const &p) {
    this->operator()(p.first);
    this->operator()(p.second);
}
void dump_asm_pass::operator()(PairIdRefLiteralInteger const &p) {
    this->operator()(p.first);
    this->operator()(p.second);
}
void dump_asm_pass::operator()(PairLiteralIntegerIdRef const &p) {
    std::visit(overloaded{[&](auto const &l) {
                   using unsigned_t = std::make_unsigned_t<std::decay_t<decltype(l)>>;
                   *os_ << " " << static_cast<unsigned_t>(l);
               }},
               p.first);
    this->operator()(p.second);
}

void dump_asm_pass::operator()(spv_inst *const &in) { *os_ << " %" << in->id(); }
void dump_asm_pass::operator()(OpExtInst const &in) {
    pre_visit(in);
    this->operator()(in.type());
    visit_result(in);
    this->operator()(in.op0());

    if (auto extimport = dyn_cast<OpExtInstImport>(in.op0());
        extimport && extimport->op0() == OpenCLExt) {
        this->operator()(static_cast<OpenCLEntrypoint>(in.op1()));
    } else {
        this->operator()(in.op1());
    }

    for (auto const &op : in.op2()) {
        this->operator()(op);
    }
    post_visit(in);
}

void dump_asm_pass::run_on_module(tinytc_spv_mod const &m) {
    auto const visit_section = [&](section s) {
        for (auto const &i : m.insts(s)) {
            visit(*this, i);
        }
    };
    *os_ << "; SPIR-V" << std::endl
         << "; Version " << m.major_version() << '.' << m.minor_version() << std::endl
         << "; Generator: Tiny Tensor Compiler" << std::endl
         << "; Bound: " << m.bound() << std::endl
         << "; Schema: 0";
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        visit_section(enum_cast<section>(s));
    }
    *os_ << std::endl;
}

} // namespace tinytc::spv
