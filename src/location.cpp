// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "location.hpp"

#include <ostream>

namespace tinytc {

void print_range(std::ostream &os, position const &begin, position const &end) {
    auto end_limit = std::max(0, end.column - 1);
    os << begin;
    if (begin.line < end.line) {
        os << '-' << end.line << '.' << end_limit;
    } else if (begin.column < end_limit) {
        os << '-' << end_limit;
    }
}

} // namespace tinytc

namespace std {

ostream &operator<<(ostream &os, ::tinytc::position const &p) {
    return os << p.line << '.' << p.column;
}

ostream &operator<<(ostream &os, ::tinytc::location const &loc) {
    os << loc.begin.source_id << ':';
    ::tinytc::print_range(os, loc.begin, loc.end);
    return os;
}

} // namespace std
