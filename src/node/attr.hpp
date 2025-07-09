// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ATTR_20250626_HPP
#define ATTR_20250626_HPP

#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace tinytc {
enum class AK;
} // namespace tinytc

struct tinytc_attr {
  public:
    inline tinytc_attr(tinytc::AK tid, tinytc_compiler_context_t ctx) : tid_(tid), ctx_(ctx) {}
    virtual ~tinytc_attr() = default;
    inline auto type_id() const -> tinytc::AK { return tid_; }
    inline auto context() const -> tinytc_compiler_context_t { return ctx_; }

  private:
    tinytc::AK tid_;
    tinytc_compiler_context_t ctx_;
};

namespace tinytc {

enum class AK { AK_array, AK_boolean, AK_dictionary, AK_integer, AK_string };

class array_attr : public tinytc_attr {
  public:
    inline static bool classof(tinytc_attr const &a) { return a.type_id() == AK::AK_array; }
    static auto get(tinytc_compiler_context_t ctx, array_view<tinytc_attr_t> values)
        -> tinytc_attr_t;

    inline auto begin() const -> std::vector<tinytc_attr_t>::const_iterator {
        return values_.begin();
    }
    inline auto end() const -> std::vector<tinytc_attr_t>::const_iterator { return values_.end(); }
    inline auto size() const { return values_.size(); }
    inline auto const &values() const { return values_; }
    inline auto value(std::size_t i) const -> tinytc_attr_t { return values_[i]; }

  protected:
    array_attr(tinytc_compiler_context_t ctx, std::vector<tinytc_attr_t> values);

  private:
    std::vector<tinytc_attr_t> values_;
};

class boolean_attr : public tinytc_attr {
  public:
    inline static bool classof(tinytc_attr const &a) { return a.type_id() == AK::AK_boolean; }
    static auto get(tinytc_compiler_context_t ctx, bool value) -> tinytc_attr_t;

    inline auto value() const { return value_; }

  protected:
    boolean_attr(tinytc_compiler_context_t ctx, bool value);
    friend class compiler_context_cache;

  private:
    bool value_;
};

class dictionary_attr : public tinytc_attr {
  public:
    inline static bool classof(tinytc_attr const &a) { return a.type_id() == AK::AK_dictionary; }
    static auto get(tinytc_compiler_context_t ctx, array_view<tinytc_named_attr_t> sorted_attrs)
        -> tinytc_attr_t;
    static void sort(mutable_array_view<tinytc_named_attr_t> unsorted_attrs);

    inline auto begin() const -> std::vector<tinytc_named_attr_t>::const_iterator {
        return attrs_.begin();
    }
    inline auto end() const -> std::vector<tinytc_named_attr_t>::const_iterator {
        return attrs_.end();
    }
    inline auto const &attrs() const { return attrs_; }

    auto find(tinytc_attr_t name) -> tinytc_attr_t;
    auto find(std::string_view name) -> tinytc_attr_t;

  protected:
    dictionary_attr(tinytc_compiler_context_t ctx, std::vector<tinytc_named_attr_t> sorted_attrs);

  private:
    static auto get_name_string(tinytc_attr_t name) -> std::string_view;

    std::vector<tinytc_named_attr_t> attrs_;
};

class integer_attr : public tinytc_attr {
  public:
    inline static bool classof(tinytc_attr const &a) { return a.type_id() == AK::AK_integer; }
    static auto get(tinytc_compiler_context_t ctx, std::int64_t value) -> tinytc_attr_t;

    inline auto value() const -> std::int64_t { return value_; }

  protected:
    integer_attr(tinytc_compiler_context_t ctx, std::int64_t value);

  private:
    std::int64_t value_;
};

class string_attr : public tinytc_attr {
  public:
    inline static bool classof(tinytc_attr const &a) { return a.type_id() == AK::AK_string; }
    static auto get(tinytc_compiler_context_t ctx, std::string_view str) -> tinytc_attr_t;

    inline auto str() const -> std::string_view { return str_; }
    inline auto c_str() const -> char const * { return str_.c_str(); }

  protected:
    string_attr(tinytc_compiler_context_t ctx, std::string str);

  private:
    std::string str_;
};

auto get_attr(tinytc_attr_t dict, tinytc_attr_t name) -> tinytc_attr_t;
auto get_attr(tinytc_attr_t dict, std::string_view name) -> tinytc_attr_t;

template <std::integral T> auto get_array_attr_as(tinytc_attr_t a) -> std::vector<T> {
    auto aa = dyn_cast<array_attr>(a);
    if (!aa) {
        throw status::ir_expected_array_attribute;
    }
    auto result = std::vector<T>{};
    result.reserve(aa->size());
    for (auto const &va : *aa) {
        auto val = dyn_cast_or_throw<integer_attr>(va, [&] {
                       return status::ir_expected_integer_attribute;
                   })->value();
        result.emplace_back(static_cast<T>(val));
    }
    return result;
}

} // namespace tinytc

#endif // ATTR_20250626_HPP
