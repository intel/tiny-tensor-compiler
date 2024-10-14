// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "source.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"

#include <cstdint>

using namespace tinytc;

tinytc_source::tinytc_source(compiler_context ctx, std::string code,
                             tinytc_location const &code_loc,
                             std::vector<char const *> required_extensions,
                             tinytc_core_feature_flags_t core_features)
    : ctx_{std::move(ctx)}, code_(std::move(code)), code_loc_(code_loc),
      required_extensions_(std::move(required_extensions)), core_features_(core_features) {}

extern "C" {
tinytc_status_t tinytc_source_get_code(const_tinytc_source_t src, size_t *length,
                                       char const **code) {
    if (src == nullptr || length == nullptr || code == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *length = src->size();
        *code = src->code();
    });
}

tinytc_status_t tinytc_source_get_location(const_tinytc_source_t src, tinytc_location_t *loc) {
    if (src == nullptr || loc == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *loc = src->code_loc(); });
}

tinytc_status_t tinytc_source_get_compiler_context(const_tinytc_source_t src,
                                                   tinytc_compiler_context_t *ctx) {
    if (src == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *ctx = src->share_context().release(); });
}

tinytc_status_t tinytc_source_get_core_features(const_tinytc_source_t src,
                                                tinytc_core_feature_flags_t *core_features) {
    if (src == nullptr || core_features == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *core_features = src->core_features(); });
}

tinytc_status_t tinytc_source_get_extensions(const_tinytc_source_t src, uint32_t *extensions_size,
                                             char const *const **extensions) {
    if (src == nullptr || extensions_size == nullptr || extensions == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *extensions_size = src->required_extensions().size();
        *extensions = src->required_extensions().data();
    });
}

tinytc_status_t tinytc_source_release(tinytc_source_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_source_retain(tinytc_source_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
