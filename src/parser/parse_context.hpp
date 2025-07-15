// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSE_CONTEXT_20231221_HPP
#define PARSE_CONTEXT_20231221_HPP

#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <complex>
#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc {

using def_rhs = std::variant<bool, std::int64_t, double, std::string, tinytc_type_t, tinytc_attr_t>;

class parse_context {
  public:
    parse_context(shared_handle<tinytc_compiler_context_t> compiler_ctx);
    inline auto program() { return program_; }

    void val(std::variant<std::int64_t, std::string> const &id, tinytc_value &val,
             location const &l);
    auto val(std::variant<std::int64_t, std::string> const &id, location const &l)
        -> tinytc_value_t;

    void report_error(location const &loc, std::string const &what);

    auto cctx() -> tinytc_compiler_context_t { return compiler_ctx_.get(); }

    void push_scope();
    void pop_scope();

    void push_region(tinytc_region_t r);
    void pop_region();
    auto top_region() -> tinytc_region_t;
    auto has_regions() -> bool;

    void add_function(std::string const &name, unique_handle<tinytc_func_t> fun);
    void add_def(std::string const &id, def_rhs &&rhs, location const &lc);

    auto def(std::string const &id, location const &lc) -> def_rhs const &;

  private:
    shared_handle<tinytc_compiler_context_t> compiler_ctx_;
    shared_handle<tinytc_prog_t> program_;
    std::vector<std::unordered_map<std::int64_t, tinytc_value_t>> unnamed_id_map_;
    std::vector<std::unordered_map<std::string, tinytc_value_t>> named_id_map_;
    std::stack<tinytc_region_t> regions_;
    std::unordered_map<std::string, location> global_names_;
    std::vector<std::unordered_map<std::string, std::pair<def_rhs, location>>> def_map_;
};

} // namespace tinytc

#endif // PARSE_CONTEXT_20231221_HPP
