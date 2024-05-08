#!/usr/bin/env python3
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

from argparse import ArgumentParser
from yaml import load, Loader
import os
import re
import subprocess

parser = ArgumentParser()
parser.add_argument('lib')
parser.add_argument('input_yamls', nargs='+')
args = parser.parse_args()

doc_symbols = set()

for input_yaml in args.input_yamls:
    with open(input_yaml, 'r') as y:
        api = load(y, Loader)
        for _, categories in api.items():
            for _, category in categories.items():
                for symbol_type, symbol_list in category.items():
                    if symbol_type == 'function':
                        for symbol in symbol_list:
                            doc_symbols.add(symbol)

lib_symbols = set()

if os.path.exists(args.lib):
    nm = subprocess.check_output(['nm', '-gDUC', args.lib])
    pattern = re.compile(r'[0-9a-fA-F]* T (.*)')
    for line in nm.decode('utf-8').split('\n'):
        m = re.match(pattern, line)
        if m:
            lib_symbols.add(m.group(1))
else:
    print(f'Library does not exist: {args.lib}')
    exit(-1)

doc_non_existent = doc_symbols - lib_symbols
if doc_non_existent:
    print(f'The following non-existent symbols are in the doc: {doc_non_existent}')

missing_symbols = lib_symbols - doc_symbols
if missing_symbols:
    print(f'The following symbols are not documented: {missing_symbols}')
