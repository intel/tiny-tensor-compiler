// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/parser.hpp"
#include "parser/lexer.hpp"
#include "parser/parse_context.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/ir/location.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

namespace tinytc {

source_manager::source_manager(std::ostream *oerr) : oerr_{oerr} {}

auto source_manager::parse_file(std::string const &filename) -> prog {
    auto ir_stream = std::ifstream(filename);
    if (!ir_stream.good()) {
        if (oerr_) {
            *oerr_ << "Could not open " << filename << " for reading." << std::endl;
        }
        return nullptr;
    }
    auto ir = std::string(std::istreambuf_iterator<char>{ir_stream}, {});
    auto p = parse(ir, oerr_, filename);
    sources_[filename] = std::move(ir);
    return p;
}
auto source_manager::parse_stdin() -> prog {
    auto ir = std::string(std::istreambuf_iterator<char>{std::cin}, {});
    auto filename = "<stdin:" + std::to_string(stdin_counter_++) + ">";
    auto p = parse(ir, oerr_, filename);
    sources_[filename] = std::move(ir);
    return p;
}
auto source_manager::parse_string(std::string ir) -> prog {
    auto filename = "<memory:" + std::to_string(memory_counter_++) + ">";
    auto p = parse(ir, oerr_, filename);
    sources_[filename] = std::move(ir);
    return p;
}

void source_manager::report_error(location const &l, std::string const &what) {
    auto const source = sources_.find(l.begin.filename);
    if (source == sources_.end()) {
        if (oerr_) {
            *oerr_ << "Source manager does not know the file " << l.begin.filename << std::endl;
        }
        return;
    }
    const char *begin = source->second.c_str();
    report_error_with_context(oerr_, begin, source->second.size(), l, what);
}

auto source_manager::error_reporter() -> error_reporter_function {
    return std::bind(&source_manager::report_error, this, std::placeholders::_1,
                     std::placeholders::_2);
}

auto parse(std::string const &input, std::ostream *oerr, std::string const &filename) -> prog {
    auto lex = parser::lexer(input, oerr, filename);
    auto ctx = parser::parse_context{};
    auto p = parser::parser(lex, ctx);
    if (p() == 0) {
        return ctx.program();
    }
    return nullptr;
}

} // namespace tinytc
