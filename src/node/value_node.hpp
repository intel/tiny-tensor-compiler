// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VALUE_NODE_20230309_HPP
#define VALUE_NODE_20230309_HPP

#include "node/data_type_node.hpp"
#include "reference_counted.hpp"
#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace tinytc {
enum class VK { float_, int_, val };
using value_nodes = type_list<class float_imm, class int_imm, class val>;
} // namespace tinytc

struct tinytc_value : tinytc::reference_counted {
  public:
    using leaves = tinytc::value_nodes;

    inline tinytc_value(tinytc::VK tid, tinytc_data_type_t ty) : tid_(tid), ty_(std::move(ty)) {}
    virtual ~tinytc_value() = default;
    inline auto type_id() const -> tinytc::VK { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline tinytc_data_type_t ty() const { return ty_; }
    inline void ty(tinytc_data_type_t ty) { ty_ = std::move(ty); }

    inline auto context() const -> tinytc_compiler_context_t { return ty_->context(); }

    virtual auto name() const -> char const * = 0;
    virtual void name(std::string name) = 0;
    virtual auto has_name() const -> bool = 0;

  private:
    tinytc::VK tid_;
    tinytc_data_type_t ty_;
    tinytc::location loc_;
};

namespace tinytc {

using value_node = ::tinytc_value;

class float_imm : public value_node {
  public:
    inline static bool classof(value_node const &v) { return v.type_id() == VK::float_; }
    inline float_imm(double v, tinytc_data_type_t ty, location const &lc = {})
        : value_node(VK::float_, ty), value_(v) {
        loc(lc);
    }

    inline auto name() const -> char const * override { return ""; }
    inline void name(std::string) override {}
    auto has_name() const -> bool override { return false; }

    inline double value() const { return value_; }

  private:
    double value_;
};

class int_imm : public value_node {
  public:
    inline static bool classof(value_node const &v) { return v.type_id() == VK::int_; }
    inline int_imm(std::int64_t v, tinytc_data_type_t ty, location const &lc = {})
        : value_node(VK::int_, ty), value_(v) {
        loc(lc);
    }

    inline auto name() const -> char const * override { return ""; }
    inline void name(std::string) override {}
    auto has_name() const -> bool override { return false; }

    inline std::int64_t value() const { return value_; }

  private:
    std::int64_t value_;
};

class val : public value_node {
  public:
    inline static bool classof(value_node const &v) { return v.type_id() == VK::val; }
    inline val(tinytc_data_type_t ty, location const &lc = {}) : value_node(VK::val, ty) {
        loc(lc);
    }

    inline auto name() const -> char const * override { return name_.c_str(); }
    inline void name(std::string name) override { name_ = std::move(name); }
    virtual auto has_name() const -> bool override { return !name_.empty(); }

  private:
    std::string name_;
};

} // namespace tinytc

#endif // VALUE_NODE_20230309_HPP
