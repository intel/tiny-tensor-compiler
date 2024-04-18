// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROGRAM_NODE_20240208_HPP
#define PROGRAM_NODE_20240208_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/virtual_type_list.hpp>

#include <utility>
#include <vector>

namespace tinytc {
using program_nodes = clir::virtual_type_list<class program>;
}

struct tinytc_prog : tinytc::reference_counted, tinytc::program_nodes {
  public:
    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

  private:
    tinytc::location loc_;
};

namespace tinytc {

using program_node = ::tinytc_prog;

class program : public clir::visitable<program, program_node> {
  public:
    inline program(std::vector<func> decls, location const &lc = {}) : decls_(std::move(decls)) {
        loc(lc);
    }
    inline auto declarations() -> std::vector<func> & { return decls_; }
    inline auto declarations() const -> std::vector<func> const & { return decls_; }

  private:
    std::vector<func> decls_;
};

} // namespace tinytc

#endif // PROGRAM_NODE_20240208_HPP
