// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parse_context.hpp"
#include "compiler_context.hpp"
#include "location.hpp"
#include "node/value.hpp"
#include "parser/parser_impl.hpp"

#include <sstream>
#include <utility>

namespace tinytc {

parse_context::parse_context(shared_handle<tinytc_compiler_context_t> compiler_ctx)
    : compiler_ctx_(std::move(compiler_ctx)) {}

void parse_context::push_scope() {
    unnamed_id_map_.push_back({});
    named_id_map_.push_back({});
}
void parse_context::pop_scope() {
    named_id_map_.pop_back();
    unnamed_id_map_.pop_back();
}

void parse_context::push_region(tinytc_region_t r) { regions_.push(r); }
void parse_context::pop_region() { regions_.pop(); }
auto parse_context::top_region() -> tinytc_region_t { return regions_.top(); }
auto parse_context::has_regions() -> bool { return !regions_.empty(); }

void parse_context::add_global_name(std::string const &name, location const &l) {
    if (auto other = global_names_.find(name); other != global_names_.end()) {
        auto oss = std::ostringstream{};
        oss << "Identifier @" << name << " was already used at " << other->second;
        throw parser::syntax_error(l, std::move(oss).str());
    }
    global_names_[name] = l;
}

void parse_context::val(std::variant<std::int64_t, std::string> const &id, tinytc_value &val,
                        location const &l) {
    const auto handle_val =
        [&l, &val]<typename KeyT>(KeyT const &id,
                                  std::vector<std::unordered_map<KeyT, tinytc_value_t>> &map) {
            if (map.empty()) {
                throw parser::syntax_error(l, "No active scope");
            }
            for (auto it = map.rbegin(); it != map.rend(); ++it) {
                if (auto other = it->find(id); other != it->end()) {
                    auto oss = std::ostringstream{};
                    oss << "Identifier %" << id << " was already used at " << other->second->loc();
                    throw parser::syntax_error(l, std::move(oss).str());
                }
            }
            val.loc(l);
            map.back()[id] = &val;
        };
    if (std::holds_alternative<std::int64_t>(id)) {
        handle_val(std::get<std::int64_t>(id), unnamed_id_map_);
    } else {
        auto const &sid = std::get<std::string>(id);
        handle_val(sid, named_id_map_);
        val.name(sid);
    }
}

auto parse_context::val(std::variant<std::int64_t, std::string> const &id, location const &l)
    -> tinytc_value_t {
    const auto handle_val =
        [&l]<typename KeyT>(KeyT const &id,
                            std::vector<std::unordered_map<KeyT, tinytc_value_t>> &map) {
            for (auto it = map.rbegin(); it != map.rend(); ++it) {
                if (auto j = it->find(id); j != it->end()) {
                    return j->second;
                }
            }
            auto oss = std::ostringstream{};
            oss << "Undefined identifier %" << id;
            throw parser::syntax_error(l, std::move(oss).str());
        };
    if (std::holds_alternative<std::int64_t>(id)) {
        return handle_val(std::get<std::int64_t>(id), unnamed_id_map_);
    }
    return handle_val(std::get<std::string>(id), named_id_map_);
}

void parse_context::report_error(location const &loc, std::string const &what) {
    compiler_ctx_->report_error(loc, what.c_str());
}

} // namespace tinytc
