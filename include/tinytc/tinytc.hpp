// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240403_HPP
#define TINYTC_20240403_HPP

#include "tinytc/binary.hpp"
#include "tinytc/bundle_format.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/ir/builder.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/gemm_generator.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/passes.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/ir/tiling.hpp"
#include "tinytc/ir/value.hpp"
#include "tinytc/opencl_cc.hpp"
#include "tinytc/parser.hpp"
#include "tinytc/recipe/small_gemm_batched.hpp"
#include "tinytc/recipe/tall_and_skinny.hpp"
#include "tinytc/tensor_kernel.hpp"
#include "tinytc/version.h"

#endif // TINYTC_20240403_HPP
