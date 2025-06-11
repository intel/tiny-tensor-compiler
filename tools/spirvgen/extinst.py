#!/usr/bin/env python3
#
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause
#
# Very simple and stupid script to generate SPIR-V classes
#

import argparse
import json
import os
import re
import shutil

from gen import generate_cpp, generate_header


def spv_ext_cpp_name(spv_ext_name):
    return spv_ext_name.replace('.', '')


def generate_enums(f, grammar, spv_ext_name):
    cpp_name = spv_ext_cpp_name(spv_ext_name)
    print(f'constexpr char const* {cpp_name}_name = "{spv_ext_name}";', file=f)
    print(file=f)
    print(f'enum class {cpp_name} {{', file=f)
    for inst in grammar['instructions']:
        print(f'{inst["opname"]} = {inst["opcode"]},', file=f)
    print('};', file=f)
    print(file=f)
    print(f'auto to_string({cpp_name} op) -> char const*;', file=f)


def generate_enums_cpp(f, grammar, spv_ext_name):
    cpp_name = spv_ext_cpp_name(spv_ext_name)
    print(f'auto to_string({cpp_name} ep) -> char const* {{ switch(ep) {{',
          file=f)
    for inst in grammar['instructions']:
        print(f'case {cpp_name}::{inst["opname"]}: return "{inst["opname"]}";',
              file=f)
    print('} return "unknown";}', file=f)


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.realpath(__file__))
    parser = argparse.ArgumentParser()
    parser.add_argument('-c',
                        help='clang-format binary',
                        default='clang-format'),
    parser.add_argument('-o', help='output directory', default=''),
    parser.add_argument('name', help='Extension name (e.g. OpenCL.std)')
    parser.add_argument(
        'grammar',
        help=
        'Grammar file from SPIRV-Headers project (e.g. extinst.opencl.std.100.grammar.json)'
    )
    args = parser.parse_args()

    spv_ext = f'{args.name.lower()}.hpp'
    spv_ext_name = args.name
    spv_ext_includes = []
    spv_ext_cpp = f'{args.name.lower()}.cpp'
    spv_ext_cpp_includes = [spv_ext]

    if shutil.which(args.c):
        with open(args.grammar) as f:
            grammar = json.load(f)
            generate_header(
                args, spv_ext, grammar,
                lambda f, grammar: generate_enums(f, grammar, spv_ext_name),
                spv_ext_includes)
            generate_cpp(
                args, spv_ext_cpp,
                grammar, lambda f, grammar: generate_enums_cpp(
                    f, grammar, spv_ext_name), spv_ext_cpp_includes)
    else:
        print(f'Could not find clang-format: {args.c}')
