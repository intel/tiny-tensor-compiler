// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "util.hpp"

#include <memory>
#include <utility>

using namespace tinytc;

namespace {
template <typename ImmT, typename T>
tinytc_status_t create_imm(tinytc_value_t *vl, T imm, tinytc_scalar_type_t type,
                           const tinytc_location_t *lc) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *vl = std::make_unique<ImmT>(imm, enum_cast<scalar_type>(type)).release();
        if (lc) {
            (*vl)->loc(*lc);
        }
    });
}
} // namespace

extern "C" {
tinytc_status_t tinytc_value_create(tinytc_value_t *vl, tinytc_data_type_t type,
                                    const tinytc_location_t *lc) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *vl = std::make_unique<val>(data_type(type, true)).release();
        if (lc) {
            (*vl)->loc(*lc);
        }
    });
}

tinytc_status_t tinytc_float_imm_create(tinytc_value_t *vl, double imm, tinytc_scalar_type_t type,
                                        const tinytc_location_t *loc) {
    return create_imm<float_imm>(vl, imm, type, loc);
}
tinytc_status_t tinytc_int_imm_create(tinytc_value_t *vl, int64_t imm, tinytc_scalar_type_t type,
                                      const tinytc_location_t *loc) {
    return create_imm<int_imm>(vl, imm, type, loc);
}

tinytc_status_t tinytc_value_release(tinytc_value_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_value_retain(tinytc_value_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_value_set_name(tinytc_value_t vl, char const *name) {
    if (vl == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { vl->name(std::string(name)); });
}

tinytc_status_t tinytc_value_get_name(const_tinytc_value_t vl, char const **name) {
    if (vl == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { return vl->name(); });
}
}
