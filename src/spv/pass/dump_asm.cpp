// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/dump_asm.hpp"
#include "spv/enums.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <concepts>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

dump_asm_pass::dump_asm_pass(std::ostream &os) : os_(&os) {}

auto dump_asm_pass::declare(spv_inst const *in) -> std::int64_t {
    auto s = slot_map_.find(in);
    if (s == slot_map_.end()) {
        const auto slot = slot_++;
        slot_map_[in] = slot;
        return slot;
    }
    return s->second;
}

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
        const auto slot = declare(&in);

        for (int i = 0; i < rhs_indent - 4 - num_digits(slot); ++i) {
            *os_ << ' ';
        }
        *os_ << "%" << slot << " = ";
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
    std::visit(overloaded{[&](std::signed_integral auto const &l) {
                              using unsigned_t = std::make_unsigned_t<std::decay_t<decltype(l)>>;
                              *os_ << " " << static_cast<unsigned_t>(l);
                          },
                          [&](auto const &l) { *os_ << " " << l; }},
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

void dump_asm_pass::operator()(spv_inst *const &in) {
    if (auto s = slot_map_.find(in); s != slot_map_.end()) {
        *os_ << " %" << s->second;
    } else if (isa<OpFunction>(*in)) {
        *os_ << " %" << declare(in);
    } else if (isa<OpVariable>(*in)) {
        *os_ << " %" << declare(in);
    } else if (isa<OpLabel>(*in)) {
        *os_ << " %" << declare(in);
    } else if (isa<OpTypePointer>(*in)) {
        *os_ << " %" << declare(in);
    } else {
        throw status::spirv_forbidden_forward_declaration;
    }
}
auto dump_asm_pass::operator()(OpExtInst const &in) {
    pre_visit(in);
    this->operator()(in.type());
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
}

auto dump_asm_pass::operator()(OpPhi const &in) {
    pre_visit(in);
    this->operator()(in.type());
    for (auto const &op : in.op0()) {
        // Forward references are allowed in phi instructions
        declare(op.first);
        this->operator()(op);
    }
}

void dump_asm_pass::run_on_module(mod const &m) {
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
    visit_section(section::capability);
    visit_section(section::ext_inst);
    visit_section(section::memory_model);
    visit_section(section::entry_point);
    visit_section(section::execution_mode);
    visit_section(section::decoration);
    visit_section(section::type_const_var);
    visit_section(section::function);
    *os_ << std::endl;
}

} // namespace tinytc::spv
