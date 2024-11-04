#!/usr/bin/env python3
#
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause
#
# Very simple and stupid script to generate SPIR-V classes
#

import argparse
import datetime
import json
import os
import shutil
import subprocess

spv_enums = 'enums.hpp'
spv_names = 'names.hpp'
spv_names_cpp = 'names.cpp'
spv_names_cpp_includes = [spv_names, spv_enums]
spv_ops = 'instructions.hpp'
spv_visitor = 'visit.hpp'
spv_ops_includes = [
    spv_enums, 'error.hpp', 'support/ilist_base.hpp', None, '<array>',
    '<cstdint>', '<optional>', '<string>', '<utility>', '<variant>', '<vector>'
]

enumerant_subs = {
    '1D': 'Dim1D',
    '2D': 'Dim2D',
    '3D': 'Dim3D',
    '2x2': 'CooperativeMatrixReduce2x2'
}

spv_inst_class = """
class spv_inst : public ilist_node<spv_inst> {
  public:
    inline spv_inst(Op opcode, bool has_result_id) : opcode_{opcode}, has_result_id_{has_result_id} {}
    virtual ~spv_inst() = default;

    spv_inst(spv_inst const &other) = delete;
    spv_inst(spv_inst &&other) = delete;
    spv_inst &operator=(spv_inst const &other) = delete;
    spv_inst &operator=(spv_inst &&other) = delete;

    inline auto opcode() const -> Op { return opcode_; }
    inline auto has_result_id() const -> bool { return has_result_id_; }

  private:
    Op opcode_;
    bool has_result_id_;
};

using DecorationAttr = std::variant<std::pair<std::string, LinkageType>>;
using ExecutionModeAttr = std::variant<std::int32_t, std::array<std::int32_t, 3u>>;
using LiteralContextDependentNumber
    = std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double>;
using LiteralString = std::string;
using LiteralInteger = std::int32_t;
using LiteralExtInstInteger = std::int32_t;
using IdResultType = spv_inst*;
using IdRef = spv_inst*;
using IdScope = spv_inst*;
using IdMemorySemantics = spv_inst*;
using PairIdRefIdRef = std::pair<spv_inst*, spv_inst*>;
using PairLiteralIntegerIdRef
    = std::pair<std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t>, spv_inst*>;
using PairIdRefLiteralInteger = std::pair<spv_inst*, std::int32_t>;
"""


def get_opcode_name(instruction):
    return instruction['opname'][2:]


def get_class_name(instruction):
    return instruction['opname']


def generate_enums(f, grammar):
    print('enum class Op {', file=f)
    for inst in grammar['instructions']:
        print(f'{get_opcode_name(inst)} = {inst["opcode"]},', file=f)
    print('};', file=f)

    for opkind in grammar['operand_kinds']:
        category = opkind['category']
        if category != 'BitEnum' and category != 'ValueEnum':
            continue
        print(f'enum class {opkind["kind"]} {{', file=f)
        for enumerant in opkind['enumerants']:
            name = enumerant["enumerant"]
            print(f'{enumerant_subs.get(name, name)} = {enumerant["value"]},',
                  file=f)
        print('};', file=f)


def generate_names(f, grammar):
    print('auto to_string(Op op) -> char const*;', file=f)

    for opkind in grammar['operand_kinds']:
        category = opkind['category']
        if category != 'BitEnum' and category != 'ValueEnum':
            continue
        print(f'auto to_string({opkind["kind"]} e) -> char const*;', file=f)


def generate_names_cpp(f, grammar):
    print('auto to_string(Op op) -> char const* { switch(op) {', file=f)
    for inst in grammar['instructions']:
        name = get_opcode_name(inst)
        print(f'case Op::{name}: return "{name}";', file=f)
    print('} return "unknown";}', file=f)

    for opkind in grammar['operand_kinds']:
        category = opkind['category']
        if category != 'BitEnum' and category != 'ValueEnum':
            continue
        print(
            f'auto to_string({opkind["kind"]} e) -> char const* {{ switch(e) {{',
            file=f)
        for enumerant in opkind['enumerants']:
            name = enumerant["enumerant"]
            name = enumerant_subs.get(name, name)
            print(f'case {opkind["kind"]}::{name}: return "{name}";', file=f)
        print('} return "unknown";}', file=f)


def get_kind(operand):
    kind = operand['kind']
    quant = operand.get('quantifier')
    if quant:
        if quant == '?':
            return f'std::optional<{kind}>'
        elif quant == '*':
            return f'std::vector<{kind}>'
        else:
            raise NotImplementedError
    return kind


def has_result_id(instruction):
    for operand in instruction.get('operands', []):
        if operand['kind'] == 'IdResult':
            return True
    return False


class Operand:

    def __init__(self, name, kind, quantifier):
        self.name = name
        self.kind = kind
        self.quantifier = quantifier


def get_operands(instruction):
    operands = []
    opno = 0
    for num, operand in enumerate(instruction.get('operands', [])):
        if operand['kind'] == 'IdResult':
            pass
        elif operand['kind'] == 'IdResultType':
            operands.append(Operand('type', get_kind(operand), ''))
        else:
            operands.append(
                Operand(f'op{opno}', get_kind(operand),
                        operand.get('quantifier', '')))
            opno = opno + 1
    return operands


def generate_op_classes(f, grammar):
    print(spv_inst_class, file=f)

    for instruction in grammar['instructions']:
        operands = get_operands(instruction)

        print(f'class {get_class_name(instruction)} : public spv_inst {{',
              file=f)
        print(f'public:', file=f)
        print(
            f'inline static bool classof(spv_inst const& s) {{ return s.opcode() == Op::{get_opcode_name(instruction)};}}',
            file=f)
        if 'capabilities' in instruction:
            caps = instruction['capabilities']
            cap_str = ','.join([f'Capability::{cap}' for cap in caps])
            print(
                f'constexpr static std::array<Capability, {len(caps)}> required_capabilities = {{{cap_str}}};',
                file=f)
        f.write(f'{get_class_name(instruction)}(')
        f.write(','.join([f'{o.kind} {o.name}' for o in operands]))
        f.write(') : ')
        initializer_list = [
            f'spv_inst{{Op::{get_opcode_name(instruction)}, {"true" if has_result_id(instruction) else "false"}}}'
        ]
        initializer_list += [
            f'{o.name}_(std::move({o.name}))' for o in operands
        ]
        f.write(','.join(initializer_list))
        f.write('{}')
        for o in operands:
            print(
                f'inline auto {o.name}() const -> {o.kind} const& {{ return {o.name}_; }}',
                file=f)
        print(f'private:', file=f)
        for o in operands:
            print(f'{o.kind} {o.name}_;', file=f)
        print('};', file=f)


def generate_visitor(f, grammar):
    format_call = lambda op: f'static_cast<Derived*>(this)->operator()({op});'

    print("""template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

        template <typename Visitor> auto visit(Visitor&& visitor, spv_inst const& inst) {
    switch (inst.opcode()) {""",
          file=f)
    for instruction in grammar['instructions']:
        print(f"""case Op::{get_opcode_name(instruction)}:
                  return visitor(static_cast<{get_class_name(instruction)} const&>(inst));""",
              file=f)
    print("""}
    throw internal_compiler_error();
}""", file=f)

    print('template <typename Derived> class default_visitor { public:',
          file=f)
    print('auto pre_visit(spv_inst const&) {}', file=f)
    for instruction in grammar['instructions']:
        print(
            f"""auto operator()({get_class_name(instruction)} const& in) {{""",
            file=f)
        print(f'static_cast<Derived*>(this)->pre_visit(in);', file=f)
        for o in get_operands(instruction):
            if o.quantifier == '*':
                print(f"""for (auto const& op : in.{o.name}()) {{
    {format_call('op')}
}}
""",
                      file=f)
            elif o.quantifier == '?':
                print(f"""if (in.{o.name}()) {{
    {format_call(f'*in.{o.name}()')}
}}
""",
                      file=f)
            else:
                print(format_call(f'in.{o.name}()'), file=f)
        print('}', file=f)
    print('};', file=f)


def print_includes(f, includes):
    for include in includes:
        if include:
            if include[0] != '<' and include[0] != '"':
                print(f'#include "{include}"', file=f)
            else:
                print(f'#include {include}', file=f)
        else:
            print('', file=f)


def generate_header(args, filename, grammar, generator, includes=[]):
    filename = os.path.join(args.o, filename)
    with open(filename, 'w') as f:
        now = datetime.datetime.now()
        basename = os.path.splitext(os.path.basename(filename))[0].upper()
        headerguard_name = f'GENERATED_{basename}_{now.year}{now.month}{now.day}_HPP'

        print(f"""// Copyright (C) {now.year} Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef {headerguard_name}
#define {headerguard_name}

""",
              file=f)
        print_includes(f, includes)
        print("""
namespace tinytc::spv {
""", file=f)

        generator(f, grammar)

        print(f"""
}}

#endif // {headerguard_name}
""", file=f)

    subprocess.call([args.c, '-i', filename])


def generate_cpp(args, filename, grammar, generator, includes=[]):
    filename = os.path.join(args.o, filename)
    with open(filename, 'w') as f:
        now = datetime.datetime.now()
        print(f"""// Copyright (C) {now.year} Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

""",
              file=f)
        print_includes(f, includes)
        print("""
namespace tinytc::spv {
""", file=f)

        generator(f, grammar)

        print(f"""
}}

#endif // {headerguard_name}
""", file=f)

    subprocess.call([args.c, '-i', filename])


def filter_grammar(grammar, filt):
    filtered_instructions = []
    for instruction in grammar['instructions']:
        opcode = instruction['opcode']
        for i in filt['include']:
            if i[0] <= opcode and opcode <= i[1]:
                filtered_instructions.append(instruction)
    grammar['instructions'] = filtered_instructions
    return grammar


def patch_grammar(grammar):
    for instruction in grammar['instructions']:
        if instruction['opname'] == 'OpDecorate':
            if instruction['operands'][-1]['kind'] == 'Decoration':
                instruction['operands'].append({'kind': 'DecorationAttr'})
        elif instruction['opname'] == 'OpExecutionMode':
            if instruction['operands'][-1]['kind'] == 'ExecutionMode':
                instruction['operands'].append({'kind': 'ExecutionModeAttr'})
    return grammar


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.realpath(__file__))
    parser = argparse.ArgumentParser()
    parser.add_argument('-c',
                        help='clang-format binary',
                        default='clang-format'),
    parser.add_argument('-f',
                        help='Filter JSON file',
                        default=os.path.join(script_dir, 'filter.json')),
    parser.add_argument('-o', help='output directory', default=''),
    parser.add_argument(
        'grammar',
        help='spirv.core.grammar.json file from SPIRV-Headers project')
    args = parser.parse_args()

    if shutil.which(args.c):
        grammar = dict()
        filt = dict()
        with open(args.grammar) as f:
            grammar = json.load(f)
        with open(args.f) as f:
            filt = json.load(f)

        grammar = filter_grammar(grammar, filt)
        grammar = patch_grammar(grammar)
        generate_header(args, spv_enums, grammar, generate_enums)
        generate_header(args, spv_names, grammar, generate_names)
        generate_header(args, spv_names_cpp, grammar, generate_names_cpp,
                        spv_names_cpp_includes)
        generate_header(args, spv_ops, grammar, generate_op_classes,
                        spv_ops_includes)
        generate_header(args, spv_visitor, grammar, generate_visitor)
    else:
        print(f'Could not find clang-format: {args.c}')
