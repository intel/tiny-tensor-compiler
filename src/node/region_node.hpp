// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/virtual_type_list.hpp>

#include <utility>
#include <vector>

namespace tinytc {
using region_nodes = clir::virtual_type_list<class rgn>;
}

struct tinytc_region : tinytc::reference_counted, tinytc::region_nodes {
  public:
    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

  private:
    tinytc::location loc_;
};

namespace tinytc {

using region_node = ::tinytc_region;

class rgn : public clir::visitable<rgn, region_node> {
  public:
    inline rgn(std::vector<inst> insts = {}, location const &lc = {}) : insts_(std::move(insts)) {
        loc(lc);
    }

    inline auto insts() -> std::vector<inst> & { return insts_; }
    inline auto insts() const -> std::vector<inst> const & { return insts_; }
    inline void insts(std::vector<inst> insts) { insts_ = std::move(insts); }

  private:
    std::vector<inst> insts_;
};

} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
