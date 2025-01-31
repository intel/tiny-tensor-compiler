# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

import lit.formats
import os

config.name = 'IR tests'
config.test_format = lit.formats.ShTest(False)

config.suffixes = ['.ir']

config.test_source_root = os.path.join(os.path.dirname(__file__), 'lit')
config.test_exec_root = config.my_exec_root

config.substitutions.append(('%tinytc-oc', config.tinytc_oc_path))
config.substitutions.append(('%tinytc-opt', config.tinytc_opt_path))
