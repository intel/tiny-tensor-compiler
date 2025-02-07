// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNCTION_NODE_20230310_HPP
#define FUNCTION_NODE_20230310_HPP

#include "node/region_node.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct tinytc_func final {
  public:
    tinytc_func(std::string name, tinytc::array_view<tinytc_data_type_t> params,
                tinytc_location const &lc = {});

    inline auto loc() const noexcept -> tinytc_location const & { return loc_; }
    inline void loc(tinytc_location const &loc) noexcept { loc_ = loc; }

    inline auto params() { return body_.params(); }
    inline auto params() const { return body_.params(); }
    inline auto num_params() const noexcept { return body_.num_params(); }

    inline auto name() const -> std::string_view { return name_; }
    inline auto body() -> tinytc_region & { return body_; }
    inline auto body() const -> tinytc_region const & { return body_; }

    inline auto work_group_size() const -> std::array<std::int32_t, 2> { return work_group_size_; }
    inline void work_group_size(std::array<std::int32_t, 2> const &work_group_size) {
        work_group_size_ = work_group_size;
    }
    inline auto subgroup_size() const -> std::int32_t { return subgroup_size_; }
    inline void subgroup_size(std::int32_t subgroup_size) { subgroup_size_ = subgroup_size; }

    void param_attr(std::int32_t param_no, tinytc_attr_t a);
    auto param_attr(std::int32_t param_no) const -> tinytc_attr_t;

  private:
    std::string name_;
    tinytc_region body_;
    std::array<std::int32_t, 2> work_group_size_;
    std::int32_t subgroup_size_;
    tinytc_location loc_;
    std::vector<tinytc_attr_t> param_attr_;
};

namespace tinytc {

using function_node = ::tinytc_func;

} // namespace tinytc

#endif // FUNCTION_NODE_20230310_HPP
