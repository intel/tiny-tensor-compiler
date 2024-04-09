// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROGRAM_NODE_20240208_HPP
#define PROGRAM_NODE_20240208_HPP

#include "tinytc/ir/func.hpp"

#include "clir/virtual_type_list.hpp"

#include <utility>
#include <vector>

namespace tinytc::ir {

class program_node : public clir::virtual_type_list<class program> {};

class program : public clir::visitable<program, program_node> {
  public:
    inline program(std::vector<func> decls) : decls_(std::move(decls)) {}
    inline std::vector<func> &declarations() { return decls_; }

  private:
    std::vector<func> decls_;
};

} // namespace tinytc::ir

#endif // PROGRAM_NODE_20240208_HPP
