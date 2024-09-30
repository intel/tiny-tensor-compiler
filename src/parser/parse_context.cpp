// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parse_context.hpp"
#include "compiler_context.hpp"
#include "location.hpp"
#include "node/value_node.hpp"
#include "parser/parser_impl.hpp"

#include <sstream>
#include <utility>

namespace tinytc {

parse_context::parse_context(compiler_context compiler_ctx) : compiler_ctx_(compiler_ctx) {}

void parse_context::push_scope() { id_map_.push_back({}); }
void parse_context::pop_scope() { id_map_.pop_back(); }

void parse_context::push_region(tinytc_region_t r) { regions_.push(r); }
void parse_context::pop_region() { regions_.pop(); }
auto parse_context::top_region() -> tinytc_region_t { return regions_.top(); }
auto parse_context::has_regions() -> bool { return !regions_.empty(); }

void parse_context::val(std::string const &id, value val, location const &l) {
    if (id_map_.empty()) {
        throw parser::syntax_error(l, "No active variable scope");
    }
    for (auto it = id_map_.rbegin(); it != id_map_.rend(); ++it) {
        if (auto other = it->find(id); other != it->end()) {
            auto oss = std::ostringstream{};
            oss << "Identifier %" << id << " was already used at " << other->second->loc();
            throw parser::syntax_error(l, oss.str());
        }
    }
    val->loc(l);
    id_map_.back()[id] = std::move(val);
}

value parse_context::val(std::string const &id, location const &l) {
    for (auto it = id_map_.rbegin(); it != id_map_.rend(); ++it) {
        if (auto j = it->find(id); j != it->end()) {
            return j->second;
        }
    }
    throw parser::syntax_error(l, "Undefined identifier %" + id);
}

void parse_context::report_error(location const &loc, std::string const &what) {
    compiler_ctx_->report_error(loc, what.c_str());
}

} // namespace tinytc
