// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNCTION_NODE_20230310_HPP
#define FUNCTION_NODE_20230310_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
using value_range = iterator_range_wrapper<value *>;
using const_value_range = iterator_range_wrapper<value const *>;
} // namespace tinytc

struct tinytc_func : tinytc::reference_counted {
  public:
    inline tinytc_func(std::string name, std::vector<tinytc::value> args, tinytc::region body,
                       tinytc::location const &lc = {})
        : name_(std::move(name)), args_(std::move(args)), body_(std::move(body)),
          work_group_size_{0, 0}, subgroup_size_{0} {
        loc(lc);
    }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto arg_begin() -> tinytc::value * { return args_.size() > 0 ? args_.data() : nullptr; }
    inline auto arg_end() -> tinytc::value * {
        return args_.size() > 0 ? args_.data() + args_.size() : nullptr;
    }
    inline auto args() -> tinytc::value_range {
        return tinytc::value_range{arg_begin(), arg_end()};
    }
    inline auto arg_begin() const -> tinytc::value const * {
        return args_.size() > 0 ? args_.data() : nullptr;
    }
    inline auto arg_end() const -> tinytc::value const * {
        return args_.size() > 0 ? args_.data() + args_.size() : nullptr;
    }
    inline auto args() const -> tinytc::const_value_range {
        return tinytc::const_value_range{arg_begin(), arg_end()};
    }

    inline auto name() const -> std::string_view { return name_; }
    inline auto body() const -> tinytc::region const & { return body_; }

    inline auto work_group_size() const -> std::array<std::int32_t, 2> { return work_group_size_; }
    inline void work_group_size(std::array<std::int32_t, 2> const &work_group_size) {
        work_group_size_ = work_group_size;
    }
    inline auto subgroup_size() const -> std::int32_t { return subgroup_size_; }
    inline void subgroup_size(std::int32_t subgroup_size) { subgroup_size_ = subgroup_size; }

  private:
    std::string name_;
    std::vector<tinytc::value> args_;
    tinytc::region body_;
    std::array<std::int32_t, 2> work_group_size_;
    std::int32_t subgroup_size_;
    tinytc::location loc_;
};

namespace tinytc {

using function = ::tinytc_func;

} // namespace tinytc

#endif // FUNCTION_NODE_20230310_HPP
