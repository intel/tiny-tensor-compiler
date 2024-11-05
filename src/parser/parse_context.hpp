// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSE_CONTEXT_20231221_HPP
#define PARSE_CONTEXT_20231221_HPP

#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc {

class parse_context {
  public:
    parse_context(compiler_context compiler_ctx);
    inline auto program() { return program_; }
    inline void program(prog p) { program_ = std::move(p); }

    void val(std::variant<std::int64_t, std::string> const &id, tinytc_value &val,
             location const &l);
    auto val(std::variant<std::int64_t, std::string> const &id,
             location const &l) -> tinytc_value_t;

    void report_error(location const &loc, std::string const &what);

    auto cctx() -> compiler_context const & { return compiler_ctx_; }

    void push_scope();
    void pop_scope();

    void push_region(tinytc_region_t r);
    void pop_region();
    auto top_region() -> tinytc_region_t;
    auto has_regions() -> bool;

    void add_global_name(std::string const &name, location const &l);

  private:
    compiler_context compiler_ctx_;
    std::vector<std::unordered_map<std::int64_t, tinytc_value_t>> unnamed_id_map_;
    std::vector<std::unordered_map<std::string, tinytc_value_t>> named_id_map_;
    std::stack<tinytc_region_t> regions_;
    std::unordered_map<std::string, location> global_names_;
    prog program_;
};

} // namespace tinytc

#endif // PARSE_CONTEXT_20231221_HPP
