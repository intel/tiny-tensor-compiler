#!/usr/bin/env python3
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

from argparse import ArgumentParser
from yaml import load, dump, Loader, Dumper
import re

parser = ArgumentParser()
parser.add_argument('input_yaml')
parser.add_argument('output_rst')
args = parser.parse_args()

brief_map = {
    'define': 'definition',
    'enum': 'enumeration',
    'struct': 'structure',
    'class': 'classe'
}

def write_underline(file, string, char = '='):
    f.write(f'{string}\n')
    f.write(char * len(string) + '\n\n')

def title(brief_title):
    return brief_map[brief_title] if brief_title in brief_map else brief_title

def strip_symbol_name(symbol):
    symbol = symbol.replace('< ', '<')
    symbol = symbol.replace(' >', '>')
    symbol = symbol.replace('tinytc::', '')
    return symbol.replace('*', '\\*')

def escape_ref(symbol):
    symbol = symbol.replace('<', '\\<')
    symbol = symbol.replace('>', '\\>')
    return symbol.replace('*', '\\*')

def get_label_and_title(rst_title):
    m = re.match('([^<]+) <([^>]+)>', rst_title)
    if m:
        return m.groups()
    return (rst_title, rst_title)


api = dict()
with open(args.input_yaml, 'r') as y:
    api = load(y, Loader)

with open(args.output_rst, 'w') as f:
    f.write('.. Copyright (C) 2024 Intel Corporation\n')
    f.write('   SPDX-License-Identifier: BSD-3-Clause\n\n')

    for rst_title, categories in api.items():
        title_text, title_label = get_label_and_title(rst_title)
        f.write(f'.. _{title_label}:\n\n')
        f.write('=' * len(title_text) + '\n')
        write_underline(f, title_text, '=')
        for category_name, category in categories.items():
            write_underline(f, category_name, '=')
            for symbol_type, symbol_list in category.items():
                f.write(f'* {title(symbol_type).title()}s\n\n')
                for symbol in symbol_list:
                    f.write(f'  * :ref:`{escape_ref(symbol)}`\n\n')
            for symbol_type, symbol_list in category.items():
                write_underline(f, f'{category_name} {title(symbol_type).title()}s', '-')
                for symbol in symbol_list:
                    f.write(f'.. _{escape_ref(symbol)}:\n\n')
                    write_underline(f, strip_symbol_name(symbol), '.')
                    f.write(f'.. doxygen{symbol_type}:: {symbol}\n\n')
