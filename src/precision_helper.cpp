// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "precision_helper.hpp"
#include "scalar_type.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/builtin_function.hpp>
#include <clir/builtin_type.hpp>
#include <clir/data_type.hpp>
#include <clir/expr.hpp>

#include <utility>

using clir::address_space;
using clir::as_char;
using clir::as_double;
using clir::as_float;
using clir::as_int;
using clir::as_long;
using clir::as_short;
using clir::as_uchar;
using clir::as_uint;
using clir::as_ulong;
using clir::builtin_type;
using clir::cast;
using clir::expr;
using clir::get_sub_group_local_id;
using clir::intel_sub_group_block_read_ui;
using clir::intel_sub_group_block_read_ul;
using clir::intel_sub_group_block_read_us;
using clir::intel_sub_group_block_write_ui;
using clir::intel_sub_group_block_write_ul;
using clir::intel_sub_group_block_write_us;
using clir::pointer_to;

namespace tinytc {

precision_helper::precision_helper(scalar_type ty) : ty_(ty) {}
builtin_type precision_helper::base_type() const { return to_clir_builtin_ty(ty_); }
builtin_type precision_helper::block_rw_base_type() const {
    auto bt = base_type();
    switch (bt) {
    case builtin_type::short_t:
        return builtin_type::ushort_t;
    case builtin_type::int_t:
    case builtin_type::float_t:
        return builtin_type::uint_t;
    case builtin_type::long_t:
    case builtin_type::double_t:
        return builtin_type::ulong_t;
    default:
        break;
    }
    return bt;
}
expr precision_helper::as_type(builtin_type ty, expr e) const {
    switch (ty) {
    case builtin_type::char_t:
        return as_char(std::move(e));
    case builtin_type::uchar_t:
        return as_uchar(std::move(e));
    case builtin_type::short_t:
        return as_short(std::move(e));
    case builtin_type::ushort_t:
        return as_ushort(std::move(e));
    case builtin_type::int_t:
        return as_int(std::move(e));
    case builtin_type::uint_t:
        return as_uint(std::move(e));
    case builtin_type::long_t:
        return as_long(std::move(e));
    case builtin_type::ulong_t:
        return as_ulong(std::move(e));
    case builtin_type::float_t:
        return as_float(std::move(e));
    case builtin_type::double_t:
        return as_double(std::move(e));
    default:
        break;
    }
    return e;
}
short precision_helper::bits() const { return size(ty_) * 8; }
clir::data_type precision_helper::type(address_space as) const {
    return clir::data_type(base_type(), as);
}
clir::data_type precision_helper::type(short size, address_space as) const {
    return clir::data_type(base_type(), size, as);
}
// TODO: Think of something for integer constants
expr precision_helper::constant(double value) const { return expr(value, bits()); }
expr precision_helper::zero() const { return constant(0.0); }

expr precision_helper::sub_group_block_read(expr address, address_space as) const {
    auto const make_read = [](builtin_type bt, expr address) -> expr {
        switch (bt) {
        case builtin_type::short_t:
        case builtin_type::ushort_t:
            return intel_sub_group_block_read_us(std::move(address));
        case builtin_type::int_t:
        case builtin_type::uint_t:
        case builtin_type::float_t:
            return intel_sub_group_block_read_ui(std::move(address));
        case builtin_type::long_t:
        case builtin_type::ulong_t:
        case builtin_type::double_t:
            return intel_sub_group_block_read_ul(std::move(address));
        default:
            break;
        }
        return address[get_sub_group_local_id()];
    };
    auto bt = block_rw_base_type();
    address = cast(pointer_to(clir::data_type(bt, as)), std::move(address));
    auto inst = make_read(bt, std::move(address));
    if (bt != base_type()) {
        return as_type(base_type(), std::move(inst));
    }
    return inst;
}
expr precision_helper::sub_group_block_write(expr address, expr data, address_space as) const {
    auto const make_write = [](builtin_type bt, expr address, expr data) -> expr {
        switch (bt) {
        case builtin_type::short_t:
        case builtin_type::ushort_t:
            return intel_sub_group_block_write_us(std::move(address), std::move(data));
        case builtin_type::int_t:
        case builtin_type::uint_t:
        case builtin_type::float_t:
            return intel_sub_group_block_write_ui(std::move(address), std::move(data));
        case builtin_type::long_t:
        case builtin_type::ulong_t:
        case builtin_type::double_t:
            return intel_sub_group_block_write_ul(std::move(address), std::move(data));
        default:
            break;
        }
        return address[get_sub_group_local_id()] = std::move(data);
    };
    auto bt = block_rw_base_type();
    address = cast(pointer_to(clir::data_type(bt, as)), std::move(address));
    if (bt != base_type()) {
        data = as_type(bt, std::move(data));
    }
    return make_write(bt, std::move(address), std::move(data));
}

} // namespace tinytc
