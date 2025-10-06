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

enum class enum_flag { doc_to_string = 0x1 };

class enum_ {
  public:
    enum_() = default;
    inline enum_(std::string name, std::string doc, std::vector<case_> cases)
        : name_(std::move(name)), doc_(std::move(doc)), cases_(std::move(cases)) {}

    inline auto doc() const -> const std::string & { return doc_; }
    inline auto name() const -> const std::string & { return name_; }
    inline auto cases() const -> std::vector<case_> const & { return cases_; }
    inline auto is_set(enum_flag flag) const -> bool {
        return flags_ & static_cast<std::uint32_t>(flag);
    }
    inline void flags(std::uint32_t f) { flags_ = f; }

  private:
    std::string name_, doc_;
    std::vector<case_> cases_;
    std::uint32_t flags_ = 0x0;
};

enum class inst_kind { mixed, collective, spmd };
enum class quantifier { single, optional, many };

enum class builtin_type { bool_, compiler_context_t, i32, i64, type_t, value_t };
using cxx_type = std::variant<builtin_type, enum_ *, std::string>;

struct prop {
    quantifier quantity;
    std::string name, doc;
    cxx_type type;
    bool private_;
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

enum class inst_flag { skip_builder = 0x1, collective = 0x2, spmd = 0x4 };

using inst_member = std::variant<prop, op, reg, ret, raw_cxx>;
class inst {
  public:
    inst() = default;
    inst(std::string name, std::string doc, std::vector<inst_member> members,
         inst *parent = nullptr);

    auto class_name() const -> std::string;
    auto kind_name(bool end = false) const -> std::string;

    inline auto doc() const -> const std::string & { return doc_; }
    inline auto name() const -> const std::string & { return name_; }
    inline auto props() const -> std::vector<prop> const & { return props_; }
    inline auto ops() const -> std::vector<op> const & { return ops_; }
    inline auto regs() const -> std::vector<reg> const & { return regs_; }
    inline auto rets() const -> std::vector<ret> const & { return rets_; }
    inline auto cxx() const -> std::vector<std::string> const & { return cxx_; }
    inline auto is_set(inst_flag flag) const -> bool {
        return flags_ & static_cast<std::uint32_t>(flag);
    }
    inline void flags(std::uint32_t f) { flags_ = f; }

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
    std::uint32_t flags_ = 0x0;
};

using type_member = std::variant<prop, raw_cxx>;
class type {
  public:
    type() = default;
    type(std::string name, std::string doc, std::vector<type_member> members,
         type *parent = nullptr);

    auto class_name() const -> std::string;
    auto kind_name(bool end = false) const -> std::string;

    inline auto doc() const -> const std::string & { return doc_; }
    inline auto name() const -> const std::string & { return name_; }
    inline auto props() const -> std::vector<prop> const & { return props_; }
    inline auto cxx() const -> std::vector<std::string> const & { return cxx_; }
    inline auto is_set(inst_flag flag) const -> bool {
        return flags_ & static_cast<std::uint32_t>(flag);
    }
    inline void flags(std::uint32_t f) { flags_ = f; }

    inline auto children() -> auto & { return children_; }
    inline auto children() const -> const auto & { return children_; }
    inline auto has_children() const -> bool { return children_.size() > 0; }
    inline auto parent() const -> type * { return parent_; }

  private:
    std::string name_, doc_;
    type *parent_ = nullptr;
    std::vector<prop> props_;
    std::vector<std::string> cxx_;
    std::vector<std::unique_ptr<type>> children_;
    std::uint32_t flags_ = 0x0;
};

} // namespace mochi

#endif // OBJECT_20250624_HPP
