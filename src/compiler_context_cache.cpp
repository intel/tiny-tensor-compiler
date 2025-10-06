// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context_cache.hpp"
#include "node/attr.hpp"
#include "node/type.hpp"
#include "tinytc/types.hpp"

namespace tinytc {

compiler_context_cache::compiler_context_cache(tinytc_compiler_context_t ctx) {
    bool_ty = std::unique_ptr<boolean_type>(new boolean_type(ctx));
    void_ty = std::unique_ptr<void_type>(new void_type(ctx));

    i8_ty = std::unique_ptr<i8_type>(new i8_type(ctx));
    i16_ty = std::unique_ptr<i16_type>(new i16_type(ctx));
    i32_ty = std::unique_ptr<i32_type>(new i32_type(ctx));
    i64_ty = std::unique_ptr<i64_type>(new i64_type(ctx));
    index_ty = std::unique_ptr<index_type>(new index_type(ctx));
    bf16_ty = std::unique_ptr<bf16_type>(new bf16_type(ctx));
    f16_ty = std::unique_ptr<f16_type>(new f16_type(ctx));
    f32_ty = std::unique_ptr<f32_type>(new f32_type(ctx));
    f64_ty = std::unique_ptr<f64_type>(new f64_type(ctx));
    c32_ty = std::unique_ptr<c32_type>(new c32_type(ctx));
    c64_ty = std::unique_ptr<c64_type>(new c64_type(ctx));

    false_attr = std::unique_ptr<boolean_attr>(new boolean_attr(ctx, false));
    true_attr = std::unique_ptr<boolean_attr>(new boolean_attr(ctx, true));
}

compiler_context_cache::~compiler_context_cache() {}

} // namespace tinytc

