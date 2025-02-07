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
                tinytc_data_type_t ty, tinytc_location const &lc = {});

    inline auto loc() const noexcept -> tinytc_location const & { return loc_; }
    inline void loc(tinytc_location const &loc) noexcept { loc_ = loc; }

    inline auto ty() const noexcept -> tinytc_data_type_t { return ty_; }

    inline auto params() { return body_.params(); }
    inline auto params() const { return body_.params(); }
    inline auto num_params() const noexcept { return body_.num_params(); }

    inline auto name() const -> std::string_view { return name_; }
    inline auto body() -> tinytc_region & { return body_; }
    inline auto body() const -> tinytc_region const & { return body_; }

    inline void attr(tinytc_attr_t a) { attr_ = a; }
    inline auto attr() const -> tinytc_attr_t { return attr_; }

    void param_attr(std::int32_t param_no, tinytc_attr_t a);
    auto param_attr(std::int32_t param_no) const -> tinytc_attr_t;

    auto subgroup_size() const -> std::int32_t;
    auto work_group_size() const -> std::array<std::int32_t, 2u>;

  private:
    std::string name_;
    tinytc_data_type_t ty_;
    tinytc_region body_;
    tinytc_location loc_;
    tinytc_attr_t attr_ = nullptr;
    std::vector<tinytc_attr_t> param_attr_;
};

namespace tinytc {

using function_node = ::tinytc_func;

} // namespace tinytc

#endif // FUNCTION_NODE_20230310_HPP
