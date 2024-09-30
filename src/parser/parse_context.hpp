// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSE_CONTEXT_20231221_HPP
#define PARSE_CONTEXT_20231221_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class parse_context {
  public:
    parse_context(compiler_context compiler_ctx);
    inline auto program() { return program_; }
    inline void program(prog p) { program_ = std::move(p); }

    void val(std::string const &id, value val, location const &l);
    value val(std::string const &id, location const &l);

    void report_error(location const &loc, std::string const &what);

    auto cctx() -> compiler_context const & { return compiler_ctx_; }

    void push_scope();
    void pop_scope();

    void push_region(tinytc_region_t r);
    void pop_region();
    auto top_region() -> tinytc_region_t;
    auto has_regions() -> bool;

  private:
    compiler_context compiler_ctx_;
    std::vector<std::unordered_map<std::string, value>> id_map_;
    std::stack<tinytc_region_t> regions_;
    prog program_;
};

} // namespace tinytc

#endif // PARSE_CONTEXT_20231221_HPP
