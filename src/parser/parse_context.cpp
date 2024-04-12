// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parse_context.hpp"
#include "node/function_node.hpp"
#include "node/value_node.hpp"
#include "parser/parser_impl.hpp"

#include <clir/handle.hpp>

#include <sstream>
#include <utility>

namespace tinytc {

void parse_context::push_scope() { id_map_.push_back({}); }
void parse_context::pop_scope() { id_map_.pop_back(); }

void parse_context::val(std::string const &id, value val, location const &l) {
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

void parse_context::prototype(std::string const &id, func p) {
    if (auto other = prototype_map_.find(id); other != prototype_map_.end()) {
        auto oss = std::ostringstream{};
        oss << "Identifier @" << id << " was already used at " << other->second->loc();
        throw parser::syntax_error(p->loc(), oss.str());
    }
    prototype_map_[id] = std::move(p);
}

func parse_context::prototype(std::string const &id, location const &l) {
    if (auto j = prototype_map_.find(id); j != prototype_map_.end()) {
        return j->second;
    }
    throw parser::syntax_error(l, "Undefined identifier @" + id);
}

} // namespace tinytc
