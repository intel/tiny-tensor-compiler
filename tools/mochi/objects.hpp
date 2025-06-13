// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OBJECTS_20250611_HPP
#define OBJECTS_20250611_HPP

#include "inst.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace mochi {

class objects {
  public:
    void add(inst *parent, std::unique_ptr<inst> i);

    inline auto insts() const -> std::vector<std::unique_ptr<inst>> const & { return insts_; }

    auto find(std::string_view name) -> inst *;

  private:
    std::vector<std::unique_ptr<inst>> insts_;
};

} // namespace mochi

#endif // OBJECTS_20250611_HPP
