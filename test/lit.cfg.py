# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

import lit.formats

config.name = 'IR tests'
config.test_format = lit.formats.ShTest(False)

config.suffixes = ['.ir']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.my_exec_root

config.substitutions.append(('%tinytc-oc', config.tinytc_oc_path))
