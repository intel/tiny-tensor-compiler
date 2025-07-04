// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parser.hpp"

#include "compiler_context.hpp"
#include "error.hpp"
#include "parser/lexer.hpp"
#include "parser/parse_context.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

namespace tinytc {

auto parse(std::string name, std::string text,
           shared_handle<tinytc_compiler_context_t> compiler_ctx) -> shared_handle<tinytc_prog_t> {
    std::int32_t source_id = compiler_ctx->add_source(std::move(name), std::move(text));
    auto const initial_loc = location{position{source_id, 1, 1}, position{source_id, 1, 1}};

    auto [ir, ir_size] = compiler_ctx->source_text(source_id);
    auto lex = lexer(ir_size, ir, initial_loc);
    auto parse_ctx = parse_context{std::move(compiler_ctx)};
    auto p = parser(lex, parse_ctx);
    if (p() == 0) {
        return parse_ctx.program();
    }
    return {};
}

} // namespace tinytc

using namespace tinytc;

tinytc_status_t tinytc_parse_file(tinytc_prog_t *prg, char const *filename,
                                  tinytc_compiler_context_t ctx) {
    if (prg == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ir_stream = std::ifstream(filename);
        if (!ir_stream.good()) {
            throw status::file_io_error;
        }
        auto ir = std::string(std::istreambuf_iterator<char>{ir_stream}, {});
        auto ctx_ = ctx ? shared_handle{ctx, true} : make_compiler_context();
        auto prog = parse(std::string(filename), std::move(ir), ctx_);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_parse_stdin(tinytc_prog_t *prg, tinytc_compiler_context_t ctx) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ir = std::string(std::istreambuf_iterator<char>{std::cin}, {});
        auto ctx_ = ctx ? shared_handle{ctx, true} : make_compiler_context();
        auto prog = parse("<stdin>", std::move(ir), ctx_);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_parse_string(tinytc_prog_t *prg, size_t source_size, char const *source,
                                    tinytc_compiler_context_t ctx) {
    if (prg == nullptr || source_size == 0 || source == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ctx_ = ctx ? shared_handle{ctx, true} : make_compiler_context();
        auto prog = parse("<memory>", std::string(source, source + source_size), ctx_);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}
