// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "ir/node/value_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "util.hpp"

#include <memory>
#include <utility>

using namespace tinytc;

namespace {
template <typename ImmT, typename T>
tinytc_status_t create_imm(tinytc_value_t *vl, T imm, tinytc_scalar_type_t type) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *vl = std::make_unique<ImmT>(imm, enum_cast<scalar_type>(type)).release(); });
}
} // namespace

extern "C" {
tinytc_status_t tinytc_value_create(tinytc_value_t *vl, tinytc_data_type_t type) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *vl = std::make_unique<val>(data_type(type, true)).release(); });
}

tinytc_status_t tinytc_float_imm_create(tinytc_value_t *vl, double imm, tinytc_scalar_type_t type) {
    return create_imm<float_imm>(vl, imm, type);
}
tinytc_status_t tinytc_int_imm_create(tinytc_value_t *vl, int64_t imm, tinytc_scalar_type_t type) {
    return create_imm<int_imm>(vl, imm, type);
}

tinytc_status_t tinytc_value_release(tinytc_value_t vl) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ref_count = vl->dec_ref();
        if (ref_count == 0) {
            delete vl;
        }
    });
}

tinytc_status_t tinytc_value_retain(tinytc_value_t vl) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->inc_ref(); });
}

tinytc_status_t tinytc_value_set_name(tinytc_value_t vl, char const *name) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->name(std::string(name)); });
}

tinytc_status_t tinytc_value_get_name(tinytc_value_t vl, char const **name) {
    if (vl == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { return vl->name(); });
}
}