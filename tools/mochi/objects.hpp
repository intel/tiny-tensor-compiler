// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OBJECTS_20250611_HPP
#define OBJECTS_20250611_HPP

#include "object.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace mochi {

class objects {
  public:
    void add(std::unique_ptr<enum_> e);
    void add(inst *parent, std::unique_ptr<inst> i);
    void add(type *parent, std::unique_ptr<type> t);
    void add(objects &&other);

    inline auto enums() const -> std::vector<std::unique_ptr<enum_>> const & { return enums_; }
    inline auto insts() const -> std::vector<std::unique_ptr<inst>> const & { return insts_; }
    inline auto types() const -> std::vector<std::unique_ptr<type>> const & { return types_; }

    auto find_enum(std::string_view name) -> enum_ *;
    auto find_inst(std::string_view name) -> inst *;
    auto find_type(std::string_view name) -> type *;

  private:
    std::vector<std::unique_ptr<enum_>> enums_;
    std::vector<std::unique_ptr<inst>> insts_;
    std::vector<std::unique_ptr<type>> types_;
};

} // namespace mochi

#endif // OBJECTS_20250611_HPP
