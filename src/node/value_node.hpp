// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VALUE_NODE_20230309_HPP
#define VALUE_NODE_20230309_HPP

#include "location.hpp"
#include "node/data_type_node.hpp"
#include "support/util.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

namespace tinytc {
class use;
class use_iterator;
class const_use_iterator;
}; // namespace tinytc

struct tinytc_value final {
  public:
    tinytc_value(tinytc_data_type_t ty = nullptr, tinytc_inst_t def_inst_ = nullptr,
                 tinytc::location const &lc = {});

    tinytc_value(tinytc_value const &) = delete;
    tinytc_value(tinytc_value &&) = default;
    tinytc_value &operator=(tinytc_value const &) = delete;
    tinytc_value &operator=(tinytc_value &&) = default;

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto ty() const -> tinytc_data_type_t { return ty_; }

    inline auto context() const -> tinytc_compiler_context_t { return ty_->context(); }

    inline auto name() const -> char const * { return name_.c_str(); }
    inline void name(std::string name) { name_ = std::move(name); }
    auto has_name() const -> bool { return !name_.empty(); }

    auto use_begin() -> tinytc::use_iterator;
    auto use_end() -> tinytc::use_iterator;
    auto uses() -> tinytc::iterator_range_wrapper<tinytc::use_iterator>;
    auto use_begin() const -> tinytc::const_use_iterator;
    auto use_end() const -> tinytc::const_use_iterator;
    auto uses() const -> tinytc::iterator_range_wrapper<tinytc::const_use_iterator>;

    // Can be nullptr, e.g. if value is a region parameter
    auto defining_inst() const -> tinytc_inst_t { return def_inst_; }

  private:
    tinytc_data_type_t ty_;
    tinytc::location loc_;
    tinytc_inst_t def_inst_ = nullptr;
    std::string name_;

    friend class tinytc::use;
    tinytc::use *first_use_ = nullptr;
};

namespace tinytc {

using value_node = ::tinytc_value;

class use {
  public:
    use() = default;
    use(tinytc_inst_t owner);
    ~use();

    use(use &&other) = delete;
    use(use const &other) = delete;
    use &operator=(use &&other) = delete;
    use &operator=(use const &other) = delete;

    use &operator=(value_node *val);

    inline auto get() -> value_node * { return value_; }
    inline auto get() const -> value_node const * { return value_; }
    void set(value_node *value);

    inline auto owner() const -> tinytc_inst_t { return owner_; }
    inline void owner(tinytc_inst_t owner) { owner_ = owner; }

    inline auto next() -> use * { return next_; }
    inline auto next() const -> use const * { return next_; }

  private:
    void add_use_to_list(use **next);
    void remove_use_from_current_list();

    tinytc_inst_t owner_ = nullptr;
    value_node *value_ = nullptr;
    use **prev_ = nullptr;
    use *next_ = nullptr;
};

namespace detail {
template <typename Derived, bool Const> class use_iterator_base {
  public:
    using value_type = std::conditional_t<Const, const use, use>;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = std::ptrdiff_t;

    use_iterator_base() : pos_{nullptr} {}
    use_iterator_base(pointer pos) : pos_{std::move(pos)} {}

    auto operator*() const -> reference { return *pos_; }
    auto operator->() const -> pointer { return pos_; }
    auto operator++() -> Derived & {
        pos_ = pos_->next();
        return *static_cast<Derived *>(this);
    }
    auto operator++(int) -> Derived {
        auto old_pos = pos_;
        pos_ = pos_->next();
        return Derived{old_pos};
    }
    auto operator==(use_iterator_base const &other) const -> bool { return pos_ == other.pos_; }
    auto operator!=(use_iterator_base const &other) const -> bool { return pos_ != other.pos_; }

  private:
    pointer pos_;
};
} // namespace detail

class use_iterator : public detail::use_iterator_base<use_iterator, false> {};
class const_use_iterator : public detail::use_iterator_base<const_use_iterator, true> {};

static_assert(std::forward_iterator<use_iterator>);
static_assert(std::forward_iterator<const_use_iterator>);

} // namespace tinytc

#endif // VALUE_NODE_20230309_HPP
