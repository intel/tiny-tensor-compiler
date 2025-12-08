// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/prog.hpp"
#include "error.hpp"
#include "location.hpp"
#include "pass/dump_ir.hpp"
#include "passes.hpp"
#include "tinytc/builder.h"
#include "tinytc/core.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

using namespace tinytc;

tinytc_prog::tinytc_prog(shared_handle<tinytc_compiler_context_t> ctx, tinytc_location const &lc)
    : ctx_{std::move(ctx)} {
    loc(lc);
}

extern "C" {

tinytc_status_t tinytc_prog_create(tinytc_prog_t *prg, tinytc_compiler_context_t ctx,
                                   const tinytc_location_t *loc) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *prg = std::make_unique<tinytc_prog>(shared_handle{ctx, true}, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_prog_add_function(tinytc_prog_t prg, tinytc_func_t fun) {
    if (prg == nullptr || fun == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { prg->push_back(tinytc::unique_handle(fun)); });
}

tinytc_status_t tinytc_prog_release(tinytc_prog_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_retain(tinytc_prog_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_dump(tinytc_prog_t prg) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { run_function_pass(dump_ir_pass{std::cerr}, *prg); });
}

tinytc_status_t tinytc_prog_get_compiler_context(const_tinytc_prog_t prg,
                                                 tinytc_compiler_context_t *ctx) {
    if (prg == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *ctx = prg->context(); });
}

tinytc_status_t tinytc_prog_print_to_file(tinytc_prog_t prg, char const *filename) {
    if (prg == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto stream = std::ofstream(filename);
        if (!stream.good()) {
            throw status::file_io_error;
        }
        run_function_pass(dump_ir_pass{stream}, *prg);
    });
}

tinytc_status_t tinytc_prog_print_to_string(tinytc_prog_t prg, char **str) {
    if (prg == nullptr || str == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const text = [&] {
            auto oss = std::ostringstream{};
            run_function_pass(dump_ir_pass{oss}, *prg);
            return std::move(oss).str();
        }();
        auto const length = text.size() + 1; // Need to include terminating null character
        *str = (char *)malloc(length * sizeof(char));
        if (!str) {
            throw status::bad_alloc;
        }
        std::strncpy(*str, text.c_str(), length);
    });
}

void tinytc_string_destroy(char *str) { free(str); }
}
