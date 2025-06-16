// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_20250611_HPP
#define INST_20250611_HPP

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace mochi {

enum class inst_kind { mixed, collective, spmd };
enum class quantifier { single, optional, many };

struct prop {
    quantifier quantity;
    std::string name, type;

    auto cxx_type() const -> std::string;
    auto cxx_storage_type() const -> std::string;
};
struct op {
    quantifier quantity;
    std::string name;
    bool has_offset_property = false;

    auto cxx_type() const -> char const *;
    auto offset_name() const -> std::string;
};
struct reg {
    std::string name;
};
struct ret {
    quantifier quantity;
    std::string name;

    auto cxx_type() const -> char const *;
};
struct raw_cxx {
    std::string code;
};
using member = std::variant<prop, op, reg, ret, raw_cxx>;

class inst {
  public:
    inst() = default;
    inst(std::string name, std::vector<member> members, inst *parent = nullptr);

    auto class_name() const -> std::string;
    auto ik_name(bool end = false) const -> std::string;

    inline auto name() const -> const std::string & { return name_; }
    inline auto props() const -> std::vector<prop> const & { return props_; }
    inline auto ops() const -> std::vector<op> const & { return ops_; }
    inline auto regs() const -> std::vector<reg> const & { return regs_; }
    inline auto rets() const -> std::vector<ret> const & { return rets_; }
    inline auto cxx() const -> std::vector<std::string> const & { return cxx_; }

    inline auto children() -> auto & { return children_; }
    inline auto children() const -> const auto & { return children_; }
    inline auto has_children() const -> bool { return children_.size() > 0; }
    inline auto parent() const -> inst * { return parent_; }

  private:
    std::string name_;
    inst *parent_ = nullptr;
    std::vector<prop> props_;
    std::vector<op> ops_;
    std::vector<reg> regs_;
    std::vector<ret> rets_;
    std::vector<std::string> cxx_;
    std::vector<std::unique_ptr<inst>> children_;
};

} // namespace mochi

#endif // INST_20250611_HPP
