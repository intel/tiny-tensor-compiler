// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOCATION_20240209_HPP
#define LOCATION_20240209_HPP

#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstdint>
#include <iosfwd>

namespace tinytc {

//! Position counting starts with 1
constexpr static std::int32_t position_count_start = 1;

//! Advance position by nline number of lines
inline void lines(position &p, std::int32_t nline = 1) {
    if (nline > 0) {
        p.line = std::max(position_count_start, p.line + nline);
        p.column = 1;
    }
}
//! Advance position by ncol number of columns
inline void columns(position &p, std::int32_t ncol = 1) {
    p.column = std::max(position_count_start, p.column + ncol);
}

} // namespace tinytc

//! Advance position by ncol number of columns
inline tinytc_position &operator+=(tinytc_position &p, std::int32_t ncol) {
    tinytc::columns(p, ncol);
    return p;
}
//! Add ncol columns to position and return new position
inline tinytc_position operator+(tinytc_position p, std::int32_t ncol) { return p += ncol; }
//! Subtract ncol number of columns from position
inline tinytc_position &operator-=(tinytc_position &p, std::int32_t ncol) {
    tinytc::columns(p, -ncol);
    return p;
}
//! Subtract ncol columns from position and return new position
inline tinytc_position operator-(tinytc_position p, std::int32_t ncol) { return p -= ncol; }

inline bool operator<(tinytc_position const &x, tinytc_position const &y) {
    return x.line < y.line || (x.line == y.line && x.column < y.column);
}

namespace tinytc {

//! Set begin = end
inline void step(location &l) { l.begin = l.end; }
//! Advance end by count columns
inline void columns(location &l, std::int32_t count = 1) { l.end += count; }
//! Advance end by count lines
inline void lines(location &l, std::int32_t count = 1) { lines(l.end, count); }

inline auto get_optional(const tinytc_location_t *loc) -> tinytc_location_t {
    constexpr tinytc_location_t null_loc = {};
    return loc ? *loc : null_loc;
}

void print_range(std::ostream &os, position const &begin, position const &end);

} // namespace tinytc

namespace std {

//! Write position to ostream
ostream &operator<<(ostream &os, ::tinytc::position const &p);
//! Write location to ostream
ostream &operator<<(ostream &os, ::tinytc::location const &loc);

} // namespace std

#endif // LOCATION_20240209_HPP
