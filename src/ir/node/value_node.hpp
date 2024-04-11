// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VALUE_NODE_20230309_HPP
#define VALUE_NODE_20230309_HPP

#include "location.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/tinytc.hpp"

#include "clir/virtual_type_list.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace tinytc {

class value_node : public clir::virtual_type_list<class float_imm, class int_imm, class val> {
  public:
    inline location const &loc() const { return loc_; }
    inline void loc(location const &loc) { loc_ = loc; }

    virtual data_type ty() = 0;
    virtual void ty(data_type ty) = 0;
    virtual std::string_view name() const = 0;
    virtual void name(std::string name) = 0;

  private:
    location loc_;
};

class float_imm : public clir::visitable<float_imm, value_node> {
  public:
    inline float_imm(double v, scalar_type ty = scalar_type::f64) : ty_(ty), value_(v) {}

    inline data_type ty() override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline std::string_view name() const override { return ""; }
    inline void name(std::string) override {}

    inline double value() const { return value_; }

  private:
    data_type ty_;
    double value_;
};

class int_imm : public clir::visitable<int_imm, value_node> {
  public:
    inline int_imm(std::int64_t v, scalar_type ty = scalar_type::i64) : ty_(ty), value_(v) {}

    inline data_type ty() override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline std::string_view name() const override { return ""; }
    inline void name(std::string) override {}

    inline std::int64_t value() const { return value_; }

  private:
    data_type ty_;
    std::int64_t value_;
};

class val : public clir::visitable<val, value_node> {
  public:
    inline val(data_type ty, std::string prefix = "")
        : ty_(std::move(ty)), name_(std::move(prefix)) {}

    inline data_type ty() override { return ty_; }
    inline void ty(data_type ty) override { ty_ = std::move(ty); }
    inline std::string_view name() const override { return name_; }
    inline void name(std::string name) override { name_ = std::move(name); }

  private:
    data_type ty_;
    std::string name_;
};

} // namespace tinytc

#endif // VALUE_NODE_20230309_HPP
