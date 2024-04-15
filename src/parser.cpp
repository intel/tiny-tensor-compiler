// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parser.hpp"

#include "error.hpp"
#include "location.hpp"
#include "parser/lexer.hpp"
#include "parser/parse_context.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <utility>

using namespace tinytc;

extern "C" {

tinytc_source_context::tinytc_source_context() {}

auto tinytc_source_context::add_source_input(source_input src) -> location {
    sources_.emplace_back(std::move(src));
    std::int32_t sid = static_cast<std::int32_t>(sources_.size());
    return location{position{sid, 1, 1}, position{sid, 1, 1}};
}

auto tinytc_source_context::parse_file(char const *filename) -> prog {
    auto ir_stream = std::ifstream(filename);
    if (!ir_stream.good()) {
        throw status::file_io_error;
    }
    auto ir = std::string(std::istreambuf_iterator<char>{ir_stream}, {});
    auto initial_loc = add_source_input(source_input{filename, std::move(ir)});
    auto p = parse(sources_.back(), initial_loc);
    return p;
}

auto tinytc_source_context::parse_stdin() -> prog {
    auto ir = std::string(std::istreambuf_iterator<char>{std::cin}, {});
    auto filename = "<stdin:" + std::to_string(stdin_counter_++) + ">";
    auto initial_loc = add_source_input(source_input{std::move(filename), std::move(ir)});
    auto p = parse(sources_.back(), initial_loc);
    return p;
}

auto tinytc_source_context::parse_string(std::uint64_t size, char const *src) -> prog {
    auto filename = "<memory:" + std::to_string(memory_counter_++) + ">";
    auto initial_loc = add_source_input(source_input{std::move(filename), std::string(src, size)});
    auto p = parse(sources_.back(), initial_loc);
    return p;
}

auto tinytc_source_context::parse(source_input const &input, location const &initial_loc) -> prog {
    auto lex = lexer(input.text, initial_loc);
    auto ctx = parse_context{};
    auto p = parser(lex, ctx);
    if (p() == 0) {
        return ctx.program();
    }
    last_error_log_.clear();
    for (auto const &err : ctx.errors()) {
        last_error_log_ = report_error_with_context(input.text.c_str(), input.text.size(),
                                                    input.name, err.first, err.second);
    }
    return nullptr;
}

void tinytc_source_context::report_error(location const &l, std::string const &what) {
    if (l.begin.source_id >= 1 && static_cast<std::size_t>(l.begin.source_id) <= sources_.size()) {
        auto const &src = sources_[l.begin.source_id - 1];
        last_error_log_ =
            report_error_with_context(src.text.c_str(), src.text.size(), src.name, l, what);
    } else {
        last_error_log_ =
            (std::ostringstream{} << "<Source context unavailable for unknown source id: "
                                  << l.begin.source_id << ">\n"
                                  << l << ": " << what)
                .str();
    }
}

tinytc_status_t tinytc_source_context_create(tinytc_source_context_t *ctx) {
    if (ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *ctx = std::make_unique<tinytc_source_context>().release(); });
}

tinytc_status_t tinytc_source_context_parse_file(tinytc_prog_t *prg, tinytc_source_context_t ctx,
                                                 char const *filename) {
    if (prg == nullptr || ctx == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto prog = ctx->parse_file(filename);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_source_context_parse_stdin(tinytc_prog_t *prg, tinytc_source_context_t ctx) {
    if (prg == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto prog = ctx->parse_stdin();
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_source_context_parse_string(tinytc_prog_t *prg, tinytc_source_context_t ctx,
                                                   uint64_t source_size, char const *source) {
    if (prg == nullptr || ctx == nullptr || source_size == 0 || source == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto prog = ctx->parse_string(source_size, source);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_source_context_get_error_log(tinytc_source_context_t ctx, char const **log) {
    if (ctx == nullptr || log == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *log = ctx->last_error_log().c_str(); });
}

void tinytc_source_context_destroy(tinytc_source_context_t ctx) { delete ctx; }
}
