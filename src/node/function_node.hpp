// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNCTION_NODE_20230310_HPP
#define FUNCTION_NODE_20230310_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/virtual_type_list.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
using function_nodes = clir::virtual_type_list<class prototype, class function>;
}

struct tinytc_func : tinytc::reference_counted, tinytc::function_nodes {
  public:
    inline auto loc() const -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) { loc_ = loc; }

    virtual auto name() const -> std::string_view = 0;

  private:
    tinytc::location loc_;
};

namespace tinytc {

using function_node = ::tinytc_func;

class prototype : public clir::visitable<prototype, function_node> {
  public:
    inline prototype(std::string name, std::vector<value> args = {}, location const &lc = {})
        : name_(std::move(name)), args_(std::move(args)) {
        loc(lc);
    }

    inline auto name() const -> std::string_view override { return name_; }
    inline std::vector<value> &args() { return args_; }

  private:
    std::string name_;
    std::vector<value> args_;
};

class function : public clir::visitable<function, function_node> {
  public:
    inline function(func prototype, region body, location const &lc = {})
        : prototype_(std::move(prototype)),
          body_(std::move(body)), work_group_size_{0, 0}, subgroup_size_{0} {
        loc(lc);
    }

    inline auto name() const -> std::string_view override { return prototype_->name(); }

    inline func &prototype() { return prototype_; }
    inline region &body() { return body_; }
    inline auto work_group_size() const -> std::array<std::uint32_t, 2> { return work_group_size_; }

    inline void work_group_size(std::array<std::uint32_t, 2> const &work_group_size) {
        work_group_size_ = work_group_size;
    }
    inline auto subgroup_size() const -> std::uint32_t { return subgroup_size_; }
    inline void subgroup_size(std::uint32_t subgroup_size) { subgroup_size_ = subgroup_size; }

  private:
    func prototype_;
    region body_;
    std::array<std::uint32_t, 2> work_group_size_;
    std::uint32_t subgroup_size_;
};

} // namespace tinytc

#endif // FUNCTION_NODE_20230310_HPP
