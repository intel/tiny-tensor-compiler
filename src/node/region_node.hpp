// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "reference_counted.hpp"
#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {
using region_nodes = type_list<class rgn>;
}

struct tinytc_region : tinytc::reference_counted {
  public:
    enum region_kind { RK_rgn };
    using leaves = tinytc::region_nodes;

    inline tinytc_region(std::int64_t tid) : tid_(tid) {}
    inline auto type_id() const -> std::int64_t { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

  private:
    std::int64_t tid_;
    tinytc::location loc_;
};

namespace tinytc {

using region_node = ::tinytc_region;

class rgn : public region_node {
  public:
    inline static bool classof(region_node const &r) { return r.type_id() == RK_rgn; }
    inline rgn(std::vector<inst> insts = {}, location const &lc = {})
        : region_node(RK_rgn), insts_(std::move(insts)) {
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
