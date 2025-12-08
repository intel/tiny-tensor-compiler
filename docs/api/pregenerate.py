#!/usr/bin/env python3
# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

from argparse import ArgumentParser
from yaml import load, dump, Loader
import os

import subprocess

parser = ArgumentParser()
parser.add_argument('mochi_exe')
parser.add_argument('output_dir', default='.')
args = parser.parse_args()

current_dir = os.path.dirname(os.path.abspath(__file__))
include_dir = os.path.join(current_dir, '../../include')


def get_yaml(filename):
    result = subprocess.run([
        args.mochi_exe, '-g', 'class_list_yaml', '-I', include_dir,
        os.path.join(include_dir, filename)
    ],
                            capture_output=True)
    return load(result.stdout, Loader)

y1 = get_yaml('tinytc/types.anko')
y2 = get_yaml('tinytc/instructions.anko')

for key, value in y2.items():
    if value:
        if y1[key]:
            y1[key] = list(sorted(set(y1[key]) | set(value)))
        else:
            y1[key] = list(sorted(value))

# C-core
def c_core(obj):
    y = dict()
    y['Common'] = dict()
    y['Common']['enum'] = list(f'tinytc_{e}_t' for e in obj['enum'])
    y['Common']['function'] = list(f'tinytc_{e}_to_string' for e in obj['enum'])
    y = {'Core C-API': y}
    with open(os.path.join(args.output_dir, 'gen.core_capi.yaml'), 'w') as f:
        dump(y, f)

# C-builder
def c_builder(obj):
    y = dict()
    y['Instruction Builder'] = dict()
    y['Instruction Builder']['function'] = list(f'tinytc_{i}_inst_create' for i in obj['inst'])
    y['Data Type Builder'] = dict()
    y['Data Type Builder']['function'] = list(f'tinytc_{i}_type_get' for i in obj['type'])
    y = {'Builder C-API': y}
    with open(os.path.join(args.output_dir, 'gen.builder_capi.yaml'), 'w') as f:
        dump(y, f)

# C++-core
def cpp_core(obj):
    y = dict()
    y['Common'] = dict()
    y['Common']['enum'] = list(f'tinytc::{e}' for e in obj['enum'])
    y['Common']['function'] = list(f'tinytc::to_string({e})' for e in obj['enum'])
    y = {'Core C++-API': y}
    with open(os.path.join(args.output_dir, 'gen.core_cxxapi.yaml'), 'w') as f:
        dump(y, f)

# C++-builder
def cpp_builder(obj):
    y = dict()
    y['Instruction Builder'] = dict()
    y['Instruction Builder']['struct'] = list(f'tinytc::creator< {i}_inst >' for i in obj['inst'])
    y['Data Type Builder'] = dict()
    y['Data Type Builder']['struct'] = list(f'tinytc::getter< {i}_type >' for i in obj['type'])
    y = {'Builder C++-API': y}
    with open(os.path.join(args.output_dir, 'gen.builder_cxxapi.yaml'), 'w') as f:
        dump(y, f)

c_core(y1)
c_builder(y1)
cpp_core(y1)
cpp_builder(y1)
