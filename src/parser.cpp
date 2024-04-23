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

namespace tinytc {
auto parse(std::uint64_t size, char const *input) -> prog {
    auto const initial_loc = location{position{0, 1, 1}, position{0, 1, 1}};
    auto lex = lexer(size, input, initial_loc);
    auto ctx = parse_context{};
    auto p = parser(lex, ctx);
    if (p() == 0) {
        return ctx.program();
    }
    return nullptr;
}
} // namespace tinytc

using namespace tinytc;

extern "C" {

tinytc_source_context::tinytc_source_context() {}

auto tinytc_source_context::parse(std::string name, std::string text) -> prog {
    sources_.emplace_back(source_input{std::move(name), std::move(text)});
    std::int32_t source_id = static_cast<std::int32_t>(sources_.size());
    auto const initial_loc = location{position{source_id, 1, 1}, position{source_id, 1, 1}};

    auto const &input = sources_.back();
    auto lex = lexer(input.text.size(), input.text.c_str(), initial_loc);
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

void tinytc_source_context::report_error(location const &l, char const *what, bool append) {
    auto err = std::string{};
    if (l.begin.source_id >= 1 && static_cast<std::size_t>(l.begin.source_id) <= sources_.size()) {
        auto const &src = sources_[l.begin.source_id - 1];
        err = report_error_with_context(src.text.c_str(), src.text.size(), src.name, l, what);
    } else {
        err = (std::ostringstream{} << "<Source context unavailable for unknown source id: "
                                    << l.begin.source_id << ">\n"
                                    << l << ": " << what)
                  .str();
    }
    if (append) {
        last_error_log_ += std::move(err);
    } else {
        last_error_log_ = std::move(err);
    }
}

tinytc_status_t tinytc_parse_file(tinytc_prog_t *prg, char const *filename,
                                  tinytc_source_context_t source_ctx) {
    if (prg == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ir_stream = std::ifstream(filename);
        if (!ir_stream.good()) {
            throw status::file_io_error;
        }
        auto ir = std::string(std::istreambuf_iterator<char>{ir_stream}, {});

        auto prog = source_ctx ? source_ctx->parse(std::string(filename), std::move(ir))
                               : parse(ir.size(), ir.c_str());
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_parse_stdin(tinytc_prog_t *prg, tinytc_source_context_t source_ctx) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ir = std::string(std::istreambuf_iterator<char>{std::cin}, {});
        auto prog =
            source_ctx ? source_ctx->parse("<stdin>", std::move(ir)) : parse(ir.size(), ir.c_str());
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_parse_string(tinytc_prog_t *prg, size_t source_size, char const *source,
                                    tinytc_source_context_t source_ctx) {
    if (prg == nullptr || source_size == 0 || source == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto prog = source_ctx
                        ? source_ctx->parse("<memory>", std::string(source, source + source_size))
                        : parse(source_size, source);
        if (!prog) {
            throw status::parse_error;
        }
        *prg = prog.release();
    });
}

tinytc_status_t tinytc_source_context_create(tinytc_source_context_t *ctx) {
    if (ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *ctx = std::make_unique<tinytc_source_context>().release(); });
}

tinytc_status_t tinytc_source_context_add_source(tinytc_source_context_t ctx, char const *name,
                                                 char const *text, int32_t *source_id) {
    if (ctx == nullptr || name == nullptr || text == nullptr || source_id == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *source_id = ctx->add_source(name, text); });
}

tinytc_status_t tinytc_source_context_get_error_log(tinytc_source_context_t ctx, char const **log) {
    if (ctx == nullptr || log == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *log = ctx->last_error_log().c_str(); });
}

tinytc_status_t tinytc_source_context_report_error(tinytc_source_context_t ctx,
                                                   const tinytc_location_t *location,
                                                   char const *what, tinytc_bool_t append) {
    if (ctx == nullptr || location == nullptr || what == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { ctx->report_error(*location, what, bool(append)); });
}

void tinytc_source_context_destroy(tinytc_source_context_t ctx) { delete ctx; }
}
