// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSE_CONTEXT_20231221_HPP
#define PARSE_CONTEXT_20231221_HPP

#include "tinytc/ir/func.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/value.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc::ir {
class location;
}

namespace tinytc::parser {

class parse_context {
  public:
    inline parse_context() { id_map_.push_back({}); }
    inline auto program() { return program_; }
    inline void program(ir::prog p) { program_ = std::move(p); }

    void push_scope();
    void pop_scope();
    void val(std::string const &id, ir::value val, ir::location const &l);
    ir::value val(std::string const &id, ir::location const &l);

    void prototype(std::string const &id, ir::func p, ir::location const &l);
    ir::func prototype(std::string const &id, ir::location const &l);

  private:
    std::vector<std::unordered_map<std::string, ir::value>> id_map_;
    std::unordered_map<std::string, ir::func> prototype_map_;
    ir::prog program_;
};

} // namespace tinytc::parser

#endif // PARSE_CONTEXT_20231221_HPP
