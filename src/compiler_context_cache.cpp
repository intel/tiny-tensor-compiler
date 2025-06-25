// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context_cache.hpp"
#include "node/attr_node.hpp"
#include "node/data_type_node.hpp"
#include "util/casting.hpp"

namespace tinytc {

enum class scalar_type;

compiler_context_cache::compiler_context_cache(tinytc_compiler_context_t ctx) {
    bool_ty = std::unique_ptr<boolean_data_type>(new boolean_data_type(ctx));
    void_ty = std::unique_ptr<void_data_type>(new void_data_type(ctx));
    for (int i = 0; i < TINYTC_ENUM_NUM_SCALAR_TYPE; ++i) {
        scalar_tys[i] =
            std::unique_ptr<scalar_data_type>(new scalar_data_type(ctx, enum_cast<scalar_type>(i)));
    }

    false_attr = std::unique_ptr<boolean_attr>(new boolean_attr(ctx, false));
    true_attr = std::unique_ptr<boolean_attr>(new boolean_attr(ctx, true));
}

compiler_context_cache::~compiler_context_cache() {}

} // namespace tinytc

