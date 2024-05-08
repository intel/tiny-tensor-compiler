#!/bin/bash
# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

if [ -z $1 ]
then
    echo "Path to build directory must be given"
    exit
fi

echo $1

dir="$(dirname "$(realpath "$0")")"
$dir/check_c_symbols.py $1/src/libtinytc.so $dir/builder_capi.yaml $dir/core_capi.yaml
$dir/check_c_symbols.py $1/src/cl/libtinytc_cl.so $dir/cl/capi.yaml
$dir/check_c_symbols.py $1/src/ze/libtinytc_ze.so $dir/ze/capi.yaml
