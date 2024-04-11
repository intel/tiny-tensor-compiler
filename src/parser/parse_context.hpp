// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSE_CONTEXT_20231221_HPP
#define PARSE_CONTEXT_20231221_HPP

#include "tinytc/ir/func.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/tinytc.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class parse_context {
  public:
    inline parse_context() { id_map_.push_back({}); }
    inline auto program() { return program_; }
    inline void program(prog p) { program_ = std::move(p); }

    void push_scope();
    void pop_scope();
    void val(std::string const &id, value val, location const &l);
    value val(std::string const &id, location const &l);

    void prototype(std::string const &id, func p, location const &l);
    func prototype(std::string const &id, location const &l);

  private:
    std::vector<std::unordered_map<std::string, value>> id_map_;
    std::unordered_map<std::string, func> prototype_map_;
    prog program_;
};

} // namespace tinytc

#endif // PARSE_CONTEXT_20231221_HPP
