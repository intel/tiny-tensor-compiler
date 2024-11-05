# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

import datetime
import os
import subprocess

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
        basename = basename.replace('.', '_')
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
""", file=f)

    subprocess.call([args.c, '-i', filename])

