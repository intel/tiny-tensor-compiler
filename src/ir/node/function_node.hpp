// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FUNCTION_NODE_20230310_HPP
#define FUNCTION_NODE_20230310_HPP

#include "location.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/tinytc.hpp"

#include "clir/virtual_type_list.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {

class function_node : public clir::virtual_type_list<class prototype, class function> {
  public:
    inline location const &loc() const { return loc_; }
    inline void loc(location const &loc) { loc_ = loc; }

  private:
    location loc_;
};

class prototype : public clir::visitable<prototype, function_node> {
  public:
    inline prototype(std::string name, std::vector<value> args = {})
        : name_(std::move(name)), args_(std::move(args)) {}

    inline std::string_view name() const { return name_; }
    inline std::vector<value> &args() { return args_; }

  private:
    std::string name_;
    std::vector<value> args_;
};

class function : public clir::visitable<function, function_node> {
  public:
    inline function(func prototype, region body,
                    std::array<std::uint32_t, 2> const &work_group_size = {0, 0},
                    std::uint32_t subgroup_size = 0)
        : prototype_(std::move(prototype)), body_(std::move(body)),
          work_group_size_(work_group_size), subgroup_size_(subgroup_size) {}

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
