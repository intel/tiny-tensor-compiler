// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VALUE_NODE_20230309_HPP
#define VALUE_NODE_20230309_HPP

#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/virtual_type_list.hpp>

#include <cstdint>
#include <string>
#include <utility>

namespace tinytc {
using value_nodes = clir::virtual_type_list<class float_imm, class int_imm, class val>;
}

struct tinytc_value : tinytc::reference_counted, tinytc::value_nodes {
  public:
    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    virtual tinytc::data_type ty() const = 0;
    virtual void ty(tinytc::data_type ty) = 0;
    virtual auto name() const -> char const * = 0;
    virtual void name(std::string name) = 0;
    virtual auto has_name() const -> bool = 0;

  private:
    tinytc::location loc_;
};

namespace tinytc {

using value_node = ::tinytc_value;

class float_imm : public clir::visitable<float_imm, value_node> {
  public:
    inline float_imm(double v, scalar_type ty = scalar_type::f64)
        : ty_{make_scalar(ty)}, value_(v) {}

    inline data_type ty() const override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline auto name() const -> char const * override { return ""; }
    inline void name(std::string) override {}
    auto has_name() const -> bool override { return false; }

    inline double value() const { return value_; }

  private:
    data_type ty_;
    double value_;
};

class int_imm : public clir::visitable<int_imm, value_node> {
  public:
    inline int_imm(std::int64_t v, scalar_type ty = scalar_type::i64)
        : ty_{make_scalar(ty)}, value_(v) {}

    inline data_type ty() const override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline auto name() const -> char const * override { return ""; }
    inline void name(std::string) override {}
    auto has_name() const -> bool override { return false; }

    inline std::int64_t value() const { return value_; }

  private:
    data_type ty_;
    std::int64_t value_;
};

class val : public clir::visitable<val, value_node> {
  public:
    inline val(data_type ty) : ty_(std::move(ty)) {}

    inline data_type ty() const override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline auto name() const -> char const * override { return name_.c_str(); }
    inline void name(std::string name) override { name_ = std::move(name); }
    virtual auto has_name() const -> bool override { return !name_.empty(); }

  private:
    data_type ty_;
    std::string name_;
};

} // namespace tinytc

#endif // VALUE_NODE_20230309_HPP
