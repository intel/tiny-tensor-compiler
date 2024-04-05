// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOCATION_20240209_HPP
#define LOCATION_20240209_HPP

#include "tinytc/export.hpp"

#include <algorithm>
#include <iosfwd>
#include <string>

namespace tinytc::ir {

//! Position in code string
class TINYTC_EXPORT position {
  public:
    constexpr static int count_start = 1; ///< Counting starts with 1

    //! File-name or pseudo-file-name of code file
    std::string filename;
    //! Line number; counting starts at 1
    int line = count_start;
    //! Column number; counting start at 1
    int column = count_start;

    //! Advance position by nline number of lines
    inline void lines(int nline = 1) {
        if (nline > 0) {
            line = std::max(count_start, line + nline);
            column = 1;
        }
    }
    //! Advance position by ncol number of columns
    inline void columns(int ncol = 1) { column = std::max(count_start, column + ncol); }
};

//! Advance position by ncol number of columns
inline position &operator+=(position &p, int ncol) {
    p.columns(ncol);
    return p;
}
//! Add ncol columns to position and return new position
inline position operator+(position p, int ncol) { return p += ncol; }
//! Subtract ncol number of columns from position
inline position &operator-=(position &p, int ncol) {
    p.columns(-ncol);
    return p;
}
//! Subtract ncol columns from position and return new position
inline position operator-(position p, int ncol) { return p -= ncol; }

//! Code location
class TINYTC_EXPORT location {
  public:
    //! Staring position
    position begin;
    //! End position
    position end;

    //! Set begin = end
    inline void step() { begin = end; }
    //! Advance end by count columns
    inline void columns(int count = 1) { end += count; }
    //! Advance end by count lines
    inline void lines(int count = 1) { end.lines(count); }
};

} // namespace tinytc::ir

namespace std {

//! Write position to ostream
TINYTC_EXPORT ostream &operator<<(ostream &os, ::tinytc::ir::position const &p);
//! Write location to ostream
TINYTC_EXPORT ostream &operator<<(ostream &os, ::tinytc::ir::location const &loc);

} // namespace std

#endif // LOCATION_20240209_HPP
