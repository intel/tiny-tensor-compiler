#!/bin/bash
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

files=('builder_capi' 'builder_cxxapi' 'core_capi' 'core_cxxapi' 'cl/capi' 'cl/cxxapi' 'sycl/cxxapi' 'ze/capi' 'ze/cxxapi')
dir="$(dirname "$(realpath "$0")")"
for f in "${files[@]}"
do
    $dir/api_gen.py $dir/${f}.yaml $dir/${f}.rst
done
