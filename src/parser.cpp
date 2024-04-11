// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/parser.hpp"
#include "parser/lexer.hpp"
#include "parser/parse_context.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

namespace tinytc {

source_manager::source_manager(std::ostream *oerr) : oerr_{oerr} {}

auto source_manager::add_source(source src) -> location {
    sources_.emplace_back(std::move(src));
    std::int32_t sid = static_cast<std::int32_t>(sources_.size());
    return location{position{sid, 1, 1}, position{sid, 1, 1}};
}

auto source_manager::parse_file(std::string const &filename) -> prog {
    auto ir_stream = std::ifstream(filename);
    if (!ir_stream.good()) {
        if (oerr_) {
            *oerr_ << "Could not open " << filename << " for reading." << std::endl;
        }
        return nullptr;
    }
    auto ir = std::string(std::istreambuf_iterator<char>{ir_stream}, {});
    auto initial_loc = add_source(source{filename, std::move(ir)});
    auto p = parse(sources_.back().text, initial_loc, oerr_);
    return p;
}
auto source_manager::parse_stdin() -> prog {
    auto ir = std::string(std::istreambuf_iterator<char>{std::cin}, {});
    auto filename = "<stdin:" + std::to_string(stdin_counter_++) + ">";
    auto initial_loc = add_source(source{std::move(filename), std::move(ir)});
    auto p = parse(sources_.back().text, initial_loc, oerr_);
    return p;
}
auto source_manager::parse_string(std::string ir) -> prog {
    auto filename = "<memory:" + std::to_string(memory_counter_++) + ">";
    auto initial_loc = add_source(source{std::move(filename), std::move(ir)});
    auto p = parse(sources_.back().text, initial_loc, oerr_);
    return p;
}

void source_manager::report_error(location const &l, std::string const &what) {
    if (l.begin.source_id < 1 || static_cast<std::size_t>(l.begin.source_id) > sources_.size()) {
        if (oerr_) {
            *oerr_ << "Source manager does not know the source id " << l.begin.source_id
                   << std::endl;
        }
        return;
    }
    auto &src = sources_[l.begin.source_id - 1];
    const char *begin = src.text.c_str();
    report_error_with_context(oerr_, begin, src.text.size(), l, what);
}

auto source_manager::error_reporter() -> error_reporter_function {
    return std::bind(&source_manager::report_error, this, std::placeholders::_1,
                     std::placeholders::_2);
}

auto parse(std::string const &input, location const &initial_loc, std::ostream *oerr) -> prog {
    auto lex = lexer(input, initial_loc, oerr);
    auto ctx = parse_context{};
    auto p = parser(lex, ctx);
    if (p() == 0) {
        return ctx.program();
    }
    return nullptr;
}

} // namespace tinytc
