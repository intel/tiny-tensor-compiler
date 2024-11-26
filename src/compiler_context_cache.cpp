// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context_cache.hpp"
#include "node/data_type_node.hpp"
#include "support/util.hpp"

namespace tinytc {

enum class scalar_type;

compiler_context_cache::compiler_context_cache(tinytc_compiler_context_t ctx) {
    bool_ty = std::unique_ptr<boolean_data_type>(new boolean_data_type(ctx));
    void_ty = std::unique_ptr<void_data_type>(new void_data_type(ctx));
    for (int i = 0; i < TINYTC_NUMBER_OF_SCALAR_TYPES; ++i) {
        scalar_tys[i] =
            std::unique_ptr<scalar_data_type>(new scalar_data_type(ctx, enum_cast<scalar_type>(i)));
    }
}

compiler_context_cache::~compiler_context_cache() {
    for (auto &mt : memref_tys) {
        delete mt.second;
    }
    for (auto &gt : group_tys) {
        delete gt.second;
    }
}

} // namespace tinytc

