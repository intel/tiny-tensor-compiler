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
import shutil

from gen import generate_cpp, generate_header

spv_ext = 'opencl.std.hpp'
spv_ext_name = 'OpenCL.std'
spv_ext_entrypoint = 'OpenCLEntrypoint'
spv_ext_includes = []
spv_ext_cpp = 'opencl.std.cpp'
spv_ext_cpp_includes = [spv_ext]


def generate_enums(f, grammar):
    print(f'constexpr char const* OpenCLExt = "{spv_ext_name}";', file=f)
    print(file=f)
    print(f'enum class {spv_ext_entrypoint} {{', file=f)
    for inst in grammar['instructions']:
        print(f'{inst["opname"]} = {inst["opcode"]},', file=f)
    print('};', file=f)
    print(file=f)
    print(f'auto to_string({spv_ext_entrypoint} op) -> char const*;', file=f)


def generate_enums_cpp(f, grammar):
    print(
        f'auto to_string({spv_ext_entrypoint} ep) -> char const* {{ switch(ep) {{',
        file=f)
    for inst in grammar['instructions']:
        print(
            f'case {spv_ext_entrypoint}::{inst["opname"]}: return "{inst["opname"]}";',
            file=f)
    print('} return "unknown";}', file=f)


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.realpath(__file__))
    parser = argparse.ArgumentParser()
    parser.add_argument('-c',
                        help='clang-format binary',
                        default='clang-format'),
    parser.add_argument('-o', help='output directory', default=''),
    parser.add_argument(
        'grammar',
        help=
        'extinst.opencl.std.100.grammar.json file from SPIRV-Headers project')
    args = parser.parse_args()

    if shutil.which(args.c):
        with open(args.grammar) as f:
            grammar = json.load(f)
            generate_header(args, spv_ext, grammar, generate_enums,
                            spv_ext_includes)
            generate_cpp(args, spv_ext_cpp, grammar, generate_enums_cpp,
                         spv_ext_cpp_includes)
    else:
        print(f'Could not find clang-format: {args.c}')
