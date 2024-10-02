// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/value_node.hpp"

using namespace tinytc;

tinytc_value::tinytc_value(tinytc_data_type_t ty, location const &lc)
    : ty_{std::move(ty)}, loc_{lc} {}

auto tinytc_value::use_begin() -> use_iterator { return {first_use_}; }
auto tinytc_value::use_end() -> use_iterator { return {nullptr}; }
auto tinytc_value::uses() -> iterator_range_wrapper<use_iterator> {
    return {use_begin(), use_end()};
}
auto tinytc_value::use_begin() const -> const_use_iterator { return {first_use_}; }
auto tinytc_value::use_end() const -> const_use_iterator { return {nullptr}; }
auto tinytc_value::uses() const -> iterator_range_wrapper<const_use_iterator> {
    return {use_begin(), use_end()};
}

namespace tinytc {

use::use(tinytc_inst_t owner) : owner_{owner} {}

use::~use() {
    if (value_) {
        remove_use_from_current_list();
    }
}

use &use::operator=(value_node *val) {
    set(val);
    return *this;
}

void use::set(value_node *value) {
    if (value_) {
        remove_use_from_current_list();
    }
    value_ = value;
    if (value_) {
        add_use_to_list(&value_->first_use_);
    }
}

/*
 * Let next = &A.n and we have
 *
 * ...A|.p|.n-->B|.p|.n-->C|.p|.n...
 * ...----|  ^-------|  ^-------|
 *
 * After inserting T (T = this) we want the following new or adjusted pointers
 *
 * ...A|.p|.n==>T|.p|.n==>B|.p|.n-->C|.p|.n...
 * ...---|  ^======|  ^======|  ^------|
 *
 * We need to set
 * next_        = T.n -> B    = *next
 * next_->prev_ = B.p -> &T.n = &next_
 * prev_        = T.p -> &A.n = next
 * *next        = A.n -> T    = this
 */
void use::add_use_to_list(use **next) {
    next_ = *next;
    if (next_) {
        next_->prev_ = &next_;
    }
    prev_ = next;
    *next = this;
}

/*
 * We want to remove T (T = this):
 *
 * ...A|.p|.n-->T|.p|.n-->B|.p|.n-->C|.p|.n...
 * ...---|  ^------|  ^------|  ^------|
 *
 * After removing T we want the following adjusted pointers
 *
 * ...A|.p|.n==>B|.p|.n-->C|.p|.n...
 * ...---|  ^======|  ^------|
 *
 * We need to set
 * next_->prev_ = B.p -> &A.n = prev_
 * *prev_       = A.n -> B    = next_
 */
void use::remove_use_from_current_list() {
    if (next_) {
        next_->prev_ = prev_;
    }
    *prev_ = next_;
}

} // namespace tinytc
