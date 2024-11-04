// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context_cache.hpp"
#include "compiler_context.hpp"
#include "support/util.hpp"
#include "tinytc/types.hpp"

namespace tinytc {

compiler_context_cache::compiler_context_cache(tinytc_compiler_context_t ctx) {
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

