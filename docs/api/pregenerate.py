#!/usr/bin/env python3
# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

from argparse import ArgumentParser
from yaml import load, dump, Loader
import os

import subprocess

parser = ArgumentParser()
parser.add_argument('mochi_exe')
args = parser.parse_args()

include_dir = '../../include'


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
    with open('gen.core_capi.yaml', 'w') as f:
        dump(y, f)

# C-builder
def c_builder(obj):
    y = dict()
    y['Instruction'] = dict()
    y['Instruction']['function'] = list(f'tinytc_{i}_inst_create' for i in obj['inst'])
    y = {'Builder C-API': y}
    with open('gen.builder_capi.yaml', 'w') as f:
        dump(y, f)

c_core(y1)
c_builder(y1)
