// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {
using inst_range = iterator_range_wrapper<inst *>;
using const_inst_range = iterator_range_wrapper<inst const *>;
} // namespace tinytc

struct tinytc_region : tinytc::reference_counted {
  public:
    inline tinytc_region(std::vector<tinytc::inst> insts = {}, tinytc::location const &lc = {})
        : insts_(std::move(insts)) {
        loc(lc);
    }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> tinytc::inst * { return insts_.size() > 0 ? insts_.data() : nullptr; }
    inline auto end() -> tinytc::inst * {
        return insts_.size() > 0 ? insts_.data() + insts_.size() : nullptr;
    }
    inline auto insts() -> tinytc::inst_range { return tinytc::inst_range{begin(), end()}; }
    inline auto begin() const -> tinytc::inst const * {
        return insts_.size() > 0 ? insts_.data() : nullptr;
    }
    inline auto end() const -> tinytc::inst const * {
        return insts_.size() > 0 ? insts_.data() + insts_.size() : nullptr;
    }
    inline auto insts() const -> tinytc::const_inst_range {
        return tinytc::const_inst_range{begin(), end()};
    }
    inline void insts(std::vector<tinytc::inst> insts) { insts_ = std::move(insts); }

  private:
    std::vector<tinytc::inst> insts_;
    tinytc::location loc_;
};

namespace tinytc {
using rgn = ::tinytc_region;
} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
