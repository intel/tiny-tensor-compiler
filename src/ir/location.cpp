// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/location.hpp"

#include <ostream>

namespace std {

ostream &operator<<(ostream &os, ::tinytc::position const &p) {
    return os << p.line << '.' << p.column;
}

ostream &operator<<(ostream &os, ::tinytc::location const &loc) {
    auto end = std::max(0, loc.end.column - 1);
    os << loc.begin.filename << ':' << loc.begin;
    if (loc.begin.line < loc.end.line) {
        os << '-' << loc.end.line << '.' << end;
    } else if (loc.begin.column < end) {
        os << '-' << end;
    }
    return os;
}

} // namespace std
