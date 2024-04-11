// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "ir/node/value_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"

#include <memory>
#include <utility>

namespace {
template <typename ImmT, typename T>
tinytc_status_t create_imm(tinytc_value_t *val, T imm, tinytc_scalar_type_t type) {
    if (val == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        auto st = tinytc::scalar_type{std::underlying_type_t<tinytc::scalar_type>(type)};
        *val = std::make_unique<ImmT>(imm, st).release();
    });
}
} // namespace

extern "C" {
tinytc_status_t tinytc_value_create(tinytc_value_t *val, tinytc_data_type_t type) {
    if (val == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code(
        [&] { *val = std::make_unique<tinytc::val>(tinytc::data_type(type, true)).release(); });
}

tinytc_status_t tinytc_float_imm_create(tinytc_value_t *val, double imm,
                                        tinytc_scalar_type_t type) {
    return create_imm<tinytc::float_imm>(val, imm, type);
}
tinytc_status_t tinytc_int_imm_create(tinytc_value_t *val, int64_t imm, tinytc_scalar_type_t type) {
    return create_imm<tinytc::int_imm>(val, imm, type);
}

tinytc_status_t tinytc_value_release(tinytc_value_t val) {
    if (val == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        auto ref_count = val->dec_ref();
        if (ref_count == 0) {
            delete val;
        }
    });
}

tinytc_status_t tinytc_value_retain(tinytc_value_t val) {
    if (val == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { val->inc_ref(); });
}

tinytc_status_t tinytc_value_set_name(tinytc_value_t val, char const *name) {
    if (val == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { val->name(std::string(name)); });
}

tinytc_status_t tinytc_value_get_name(tinytc_value_t val, char const **name) {
    if (val == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { return val->name(); });
}
}
