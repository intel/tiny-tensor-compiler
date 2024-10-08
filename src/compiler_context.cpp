// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "compiler_context_cache.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <iostream>

namespace tinytc {
void default_error_reporter(char const *what, const tinytc_location_t *, void *) {
    std::cerr << what << std::endl;
}
} // namespace tinytc

using namespace tinytc;

extern "C" {

tinytc_compiler_context::tinytc_compiler_context()
    : cache_{std::make_unique<compiler_context_cache>(this)} {}

auto tinytc_compiler_context::source_name(std::int32_t source_id)
    -> std::pair<char const *, std::size_t> {
    if (has_source_id(source_id)) {
        auto &si = sources_[source_id - 1];
        return {si.name.c_str(), si.name.size()};
    }
    return {unavailable_source_name, sizeof(unavailable_source_name) / sizeof(char) - 1};
}
auto tinytc_compiler_context::source_text(std::int32_t source_id)
    -> std::pair<char const *, std::size_t> {
    if (has_source_id(source_id)) {
        auto &si = sources_[source_id - 1];
        return {si.text.c_str(), si.text.size()};
    }
    return {"", 0};
}
void tinytc_compiler_context::report_error(location const &l, char const *what) {
    auto [name, name_size] = source_name(l.begin.source_id);
    auto [text, text_size] = source_text(l.begin.source_id);
    auto err = report_error_with_context(text, text_size, name, l, what);
    reporter_(err.c_str(), &l, user_data_);
}

tinytc_status_t tinytc_compiler_context_create(tinytc_compiler_context_t *ctx) {
    if (ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *ctx = std::make_unique<tinytc_compiler_context>().release(); });
}

tinytc_status_t tinytc_compiler_context_add_source(tinytc_compiler_context_t ctx, char const *name,
                                                   char const *text, int32_t *source_id) {
    if (ctx == nullptr || name == nullptr || text == nullptr || source_id == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *source_id = ctx->add_source(name, text); });
}

tinytc_status_t tinytc_compiler_context_set_error_reporter(tinytc_compiler_context_t ctx,
                                                           tinytc_error_reporter_t reporter,
                                                           void *user_data) {
    if (ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { ctx->set_error_reporter(reporter, user_data); });
}

tinytc_status_t tinytc_compiler_context_set_optimization_level(tinytc_compiler_context_t ctx,
                                                               int32_t level) {
    if (ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    ctx->opt_level(level);
    return tinytc_status_success;
}

tinytc_status_t tinytc_compiler_context_report_error(tinytc_compiler_context_t ctx,
                                                     const tinytc_location_t *location,
                                                     char const *what) {
    if (ctx == nullptr || location == nullptr || what == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { ctx->report_error(*location, what); });
}

tinytc_status_t tinytc_compiler_context_release(tinytc_compiler_context_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_compiler_context_retain(tinytc_compiler_context_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
