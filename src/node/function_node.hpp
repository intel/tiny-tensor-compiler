// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNCTION_NODE_20230310_HPP
#define FUNCTION_NODE_20230310_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
enum class FK { function, prototype };
using function_nodes = type_list<class prototype, class function>;
} // namespace tinytc

struct tinytc_func : tinytc::reference_counted {
  public:
    using leaves = tinytc::function_nodes;

    inline tinytc_func(tinytc::FK tid) : tid_(tid) {}
    inline virtual ~tinytc_func() {}
    inline auto type_id() const -> tinytc::FK { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    virtual auto name() const -> std::string_view = 0;

  private:
    tinytc::FK tid_;
    tinytc::location loc_;
};

namespace tinytc {

using function_node = ::tinytc_func;

class prototype : public function_node {
  public:
    inline static bool classof(function_node const &f) { return f.type_id() == FK::prototype; }
    inline prototype(std::string name, std::vector<value> args = {}, location const &lc = {})
        : function_node(FK::prototype), name_(std::move(name)), args_(std::move(args)) {
        loc(lc);
    }

    inline auto name() const -> std::string_view override { return name_; }
    inline auto args() const -> std::vector<value> const & { return args_; }

  private:
    std::string name_;
    std::vector<value> args_;
};

class function : public function_node {
  public:
    inline static bool classof(function_node const &f) { return f.type_id() == FK::function; }
    inline function(func prototype, region body, location const &lc = {})
        : function_node(FK::function), prototype_(std::move(prototype)), body_(std::move(body)),
          work_group_size_{0, 0}, subgroup_size_{0} {
        loc(lc);
    }

    inline auto name() const -> std::string_view override { return prototype_->name(); }

    inline auto prototype() const -> func const & { return prototype_; }
    inline auto body() const -> region const & { return body_; }
    inline auto work_group_size() const -> std::array<std::int32_t, 2> { return work_group_size_; }

    inline void work_group_size(std::array<std::int32_t, 2> const &work_group_size) {
        work_group_size_ = work_group_size;
    }
    inline auto subgroup_size() const -> std::int32_t { return subgroup_size_; }
    inline void subgroup_size(std::int32_t subgroup_size) { subgroup_size_ = subgroup_size; }

  private:
    func prototype_;
    region body_;
    std::array<std::int32_t, 2> work_group_size_;
    std::int32_t subgroup_size_;
};

} // namespace tinytc

#endif // FUNCTION_NODE_20230310_HPP
