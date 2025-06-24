// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OBJECT_20250624_HPP
#define OBJECT_20250624_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace mochi {

struct case_ {
    std::string name, doc;
    std::int64_t value;
};

class enum_ {
  public:
    enum_() = default;
    inline enum_(std::string name, std::string doc, std::vector<case_> cases, bool doc_to_string)
        : name_(std::move(name)), doc_(std::move(doc)), cases_(std::move(cases)),
          doc_to_string_(doc_to_string) {}

    inline auto doc() const -> const std::string & { return doc_; }
    inline auto name() const -> const std::string & { return name_; }
    inline auto cases() const -> std::vector<case_> const & { return cases_; }
    inline auto doc_to_string() const -> bool { return doc_to_string_; }

    inline auto c_name() const -> std::string;
    inline auto cxx_name() const -> const std::string & { return name_; }

  private:
    std::string name_, doc_;
    std::vector<case_> cases_;
    bool doc_to_string_;
};

enum class inst_kind { mixed, collective, spmd };
enum class quantifier { single, optional, many };

enum class basic_type { bool_, i32, i64 };
auto to_c_type(basic_type ty) -> char const *;
auto to_cxx_type(basic_type ty) -> char const *;
using prop_type = std::variant<basic_type, enum_ *, std::string>;

struct prop {
    quantifier quantity;
    std::string name, doc;
    prop_type type;
    bool private_;

    auto c_type() const -> std::string;
    auto cxx_type() const -> std::string;
};
struct op {
    quantifier quantity;
    std::string name, doc;
    bool has_offset_property = false;

    auto offset_name() const -> std::string;
};
struct reg {
    std::string name, doc;
};
struct ret {
    quantifier quantity;
    std::string name, doc;
};
struct raw_cxx {
    std::string code;
};

using member = std::variant<prop, op, reg, ret, raw_cxx>;
class inst {
  public:
    inst() = default;
    inst(std::string name, std::string doc, std::vector<member> members, inst *parent = nullptr);

    auto class_name() const -> std::string;
    auto ik_name(bool end = false) const -> std::string;

    inline auto doc() const -> const std::string & { return doc_; }
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
    std::string name_, doc_;
    inst *parent_ = nullptr;
    std::vector<prop> props_;
    std::vector<op> ops_;
    std::vector<reg> regs_;
    std::vector<ret> rets_;
    std::vector<std::string> cxx_;
    std::vector<std::unique_ptr<inst>> children_;
};

} // namespace mochi

#endif // OBJECT_20250624_HPP
