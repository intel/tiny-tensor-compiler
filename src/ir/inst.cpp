// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/inst.hpp"

namespace tinytc {

char const *to_string(binary_op op) {
    switch (op) {
    case binary_op::add:
        return "add";
    case binary_op::sub:
        return "sub";
    case binary_op::mul:
        return "mul";
    case binary_op::div:
        return "div";
    case binary_op::rem:
        return "rem";
    }
    return "unknown";
}

char const *to_string(cmp_condition cond) {
    switch (cond) {
    case cmp_condition::eq:
        return "eq";
    case cmp_condition::ne:
        return "ne";
    case cmp_condition::gt:
        return "gt";
    case cmp_condition::ge:
        return "ge";
    case cmp_condition::lt:
        return "lt";
    case cmp_condition::le:
        return "le";
    }
    return "unknown";
}

char const *to_string(transpose t) {
    switch (t) {
    case transpose::T:
        return "t";
    case transpose::N:
        return "n";
    }
    return "unknown";
}

} // namespace tinytc
