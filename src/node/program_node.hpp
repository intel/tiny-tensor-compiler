// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROGRAM_NODE_20240208_HPP
#define PROGRAM_NODE_20240208_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {
using program_nodes = type_list<class program>;
}

struct tinytc_prog : tinytc::reference_counted {
  public:
    enum prog_kind { PK_prog };
    using leaves = tinytc::program_nodes;

    inline tinytc_prog(std::int64_t tid) : tid_(tid) {}
    inline auto type_id() const -> std::int64_t { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

  private:
    std::int64_t tid_;
    tinytc::location loc_;
};

namespace tinytc {

using program_node = ::tinytc_prog;

class program : public program_node {
  public:
    inline static bool classof(program_node const &p) { return p.type_id() == PK_prog; }
    inline program(std::vector<func> decls, location const &lc = {})
        : program_node(PK_prog), decls_(std::move(decls)) {
        loc(lc);
    }
    inline auto declarations() -> std::vector<func> & { return decls_; }
    inline auto declarations() const -> std::vector<func> const & { return decls_; }

  private:
    std::vector<func> decls_;
};

} // namespace tinytc

#endif // PROGRAM_NODE_20240208_HPP
