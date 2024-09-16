// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/inst_node.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace tinytc;

extern "C" {

char const *tinytc_address_space_to_string(tinytc_address_space_t as) {
    switch (as) {
    case tinytc_address_space_global:
        return "global";
    case tinytc_address_space_local:
        return "local";
    }
    return "unknown";
}

char const *tinytc_arithmetic_to_string(tinytc_arithmetic_t op) {
    switch (op) {
    case tinytc_arithmetic_add:
        return "add";
    case tinytc_arithmetic_sub:
        return "sub";
    case tinytc_arithmetic_mul:
        return "mul";
    case tinytc_arithmetic_div:
        return "div";
    case tinytc_arithmetic_rem:
        return "rem";
    case tinytc_arithmetic_shl:
        return "shl";
    case tinytc_arithmetic_shr:
        return "shr";
    case tinytc_arithmetic_and:
        return "and";
    case tinytc_arithmetic_or:
        return "or";
    case tinytc_arithmetic_xor:
        return "xor";
    }
    return "unknown";
}

char const *tinytc_arithmetic_unary_to_string(tinytc_arithmetic_unary_t op) {
    switch (op) {
    case tinytc_arithmetic_unary_neg:
        return "neg";
    case tinytc_arithmetic_unary_not:
        return "not";
    }
    return "unknown";
}

char const *tinytc_cmp_condition_to_string(tinytc_cmp_condition_t cond) {
    switch (cond) {
    case tinytc_cmp_condition_eq:
        return "eq";
    case tinytc_cmp_condition_ne:
        return "ne";
    case tinytc_cmp_condition_gt:
        return "gt";
    case tinytc_cmp_condition_ge:
        return "ge";
    case tinytc_cmp_condition_lt:
        return "lt";
    case tinytc_cmp_condition_le:
        return "le";
    }
    return "unknown";
}

char const *tinytc_transpose_to_string(tinytc_transpose_t t) {
    switch (t) {
    case tinytc_transpose_T:
        return "t";
    case tinytc_transpose_N:
        return "n";
    }
    return "unknown";
}

tinytc_status_t tinytc_arith_inst_create(tinytc_inst_t *instr, tinytc_arithmetic_t op,
                                         tinytc_value_t a, tinytc_value_t b,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<arith_inst>(enum_cast<arithmetic>(op), value(a, true),
                                              value(b, true), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_arith_unary_inst_create(tinytc_inst_t *instr, tinytc_arithmetic_unary_t op,
                                               tinytc_value_t a, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<arith_unary_inst>(enum_cast<arithmetic_unary>(op), value(a, true),
                                                    get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        tinytc_scalar_type_t to_ty, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<cast_inst>(value(a, true), enum_cast<scalar_type>(to_ty),
                                             get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr, tinytc_cmp_condition_t cond,
                                       tinytc_value_t a, tinytc_value_t b,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<compare_inst>(enum_cast<cmp_condition>(cond), value(a, true),
                                                value(b, true), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<alloca_inst>(data_type(ty, true), get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_axpby_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                         tinytc_bool_t atomic, tinytc_value_t alpha,
                                         tinytc_value_t A, tinytc_value_t beta, tinytc_value_t B,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<axpby_inst>(enum_cast<transpose>(tA), value(alpha, true),
                                              value(A, true), value(beta, true), value(B, true),
                                              bool(atomic), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr, tinytc_value_t a, int64_t mode,
                                          uint32_t expand_shape_size, tinytc_value_t *expand_shape,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr || expand_shape == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto eshape_vec = std::vector<value>();
        eshape_vec.reserve(expand_shape_size);
        for (uint32_t i = 0; i < expand_shape_size; ++i) {
            eshape_vec.emplace_back(value(expand_shape[i], true));
        }
        *instr = std::make_unique<expand_inst>(value(a, true), mode, std::move(eshape_vec),
                                               get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, tinytc_value_t a, int64_t from,
                                        int64_t to, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<fuse_inst>(value(a, true), from, to, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        uint32_t index_list_size, tinytc_value_t *index_list,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr || (index_list_size > 0 && index_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto il_vec = std::vector<value>();
        il_vec.reserve(index_list_size);
        for (uint32_t i = 0; i < index_list_size; ++i) {
            il_vec.emplace_back(value(index_list[i], true));
        }
        *instr = std::make_unique<load_inst>(value(a, true), std::move(il_vec), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<group_id_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<group_size_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_gemm_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                        tinytc_transpose_t tB, tinytc_bool_t atomic,
                                        tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                                        tinytc_value_t beta, tinytc_value_t C,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<gemm_inst>(enum_cast<transpose>(tA), enum_cast<transpose>(tB),
                                             value(alpha, true), value(A, true), value(B, true),
                                             value(beta, true), value(C, true), bool(atomic),
                                             get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_gemv_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                        tinytc_bool_t atomic, tinytc_value_t alpha,
                                        tinytc_value_t A, tinytc_value_t B, tinytc_value_t beta,
                                        tinytc_value_t C, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<gemv_inst>(enum_cast<transpose>(tA), value(alpha, true),
                                             value(A, true), value(B, true), value(beta, true),
                                             value(C, true), bool(atomic), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_ger_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                       tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                                       tinytc_value_t beta, tinytc_value_t C,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<ger_inst>(value(alpha, true), value(A, true), value(B, true),
                                            value(beta, true), value(C, true), bool(atomic),
                                            get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_hadamard_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                            tinytc_value_t alpha, tinytc_value_t A,
                                            tinytc_value_t B, tinytc_value_t beta, tinytc_value_t C,
                                            const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<hadamard_inst>(value(alpha, true), value(A, true), value(B, true),
                                                 value(beta, true), value(C, true), bool(atomic),
                                                 get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_num_subgroups_inst_create(tinytc_inst_t *instr,
                                                 const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<num_subgroups_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_parallel_inst_create(tinytc_inst_t *instr, tinytc_region_t body,
                                            const tinytc_location_t *loc) {
    if (instr == nullptr || body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<parallel_inst>(region(body, true), get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, tinytc_value_t a, int64_t mode,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<size_inst>(value(a, true), mode, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_subgroup_id_inst_create(tinytc_inst_t *instr, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<subgroup_id_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_subgroup_local_id_inst_create(tinytc_inst_t *instr,
                                                     const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<subgroup_local_id_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_subgroup_size_inst_create(tinytc_inst_t *instr,
                                                 const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<subgroup_size_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_subview_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                           uint32_t slice_list_size, tinytc_value_t *offset_list,
                                           tinytc_value_t *size_list,
                                           const tinytc_location_t *loc) {
    if (instr == nullptr ||
        (slice_list_size > 0 && (offset_list == nullptr || size_list == nullptr))) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto offset_vec = std::vector<value>();
        auto size_vec = std::vector<value>();
        offset_vec.reserve(slice_list_size);
        size_vec.reserve(slice_list_size);
        for (uint32_t i = 0; i < slice_list_size; ++i) {
            offset_vec.emplace_back(value(offset_list[i], true));
            size_vec.emplace_back(value(size_list[i], true));
        }
        *instr =
            std::make_unique<subview_inst>(value(a, true), offset_vec, size_vec, get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_store_inst_create(tinytc_inst_t *instr, tinytc_value_t val, tinytc_value_t a,
                                         uint32_t index_list_size, tinytc_value_t *index_list,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr || (index_list_size > 0 && index_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto il_vec = std::vector<value>();
        il_vec.reserve(index_list_size);
        for (uint32_t i = 0; i < index_list_size; ++i) {
            il_vec.emplace_back(value(index_list[i], true));
        }
        *instr = std::make_unique<store_inst>(value(val, true), value(a, true), std::move(il_vec),
                                              get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_sum_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                       tinytc_bool_t atomic, tinytc_value_t alpha, tinytc_value_t A,
                                       tinytc_value_t beta, tinytc_value_t B,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<sum_inst>(enum_cast<transpose>(tA), value(alpha, true),
                                            value(A, true), value(beta, true), value(B, true),
                                            bool(atomic), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_for_inst_create(tinytc_inst_t *instr, tinytc_value_t loop_var,
                                       tinytc_value_t from, tinytc_value_t to, tinytc_value_t step,
                                       tinytc_region_t body, const tinytc_location_t *loc) {
    if (instr == nullptr || loop_var == nullptr || from == nullptr || to == nullptr ||
        body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<for_inst>(value(loop_var, true), value(from, true), value(to, true),
                                       value(step, true), region(body, true), get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_foreach_inst_create(tinytc_inst_t *instr, tinytc_value_t loop_var,
                                           tinytc_value_t from, tinytc_value_t to,
                                           tinytc_region_t body, const tinytc_location_t *loc) {
    if (instr == nullptr || loop_var == nullptr || from == nullptr || to == nullptr ||
        body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<foreach_inst>(value(loop_var, true), value(from, true),
                                           value(to, true), region(body, true), get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_if_inst_create(tinytc_inst_t *instr, tinytc_value_t condition,
                                      tinytc_region_t then, tinytc_region_t otherwise,
                                      uint32_t return_type_list_size,
                                      tinytc_scalar_type_t *return_type_list,
                                      const tinytc_location_t *loc) {
    if (instr == nullptr || condition == nullptr || then == nullptr ||
        (return_type_list_size > 0 && return_type_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto rt = std::vector<scalar_type>();
        rt.reserve(return_type_list_size);
        for (uint32_t i = 0; i < return_type_list_size; ++i) {
            rt.emplace_back(enum_cast<scalar_type>(return_type_list[i]));
        }
        *instr =
            std::make_unique<if_inst>(value(condition, true), region(then, true),
                                      region(otherwise, true), std::move(rt), get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_yield_inst_create(tinytc_inst_t *instr, uint32_t yield_list_size,
                                         tinytc_value_t *yield_list, const tinytc_location_t *loc) {
    if (instr == nullptr || yield_list_size == 0 || yield_list == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto yl = std::vector<value>();
        yl.reserve(yield_list_size);
        for (uint32_t i = 0; i < yield_list_size; ++i) {
            yl.emplace_back(value(yield_list[i], true));
        }
        *instr = std::make_unique<yield_inst>(std::move(yl), get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_inst_release(tinytc_inst_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_inst_retain(tinytc_inst_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_inst_get_value(const_tinytc_inst_t instr, tinytc_value_t *result) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *result = instr->result().release(); });
}

tinytc_status_t tinytc_inst_get_values(const_tinytc_inst_t instr, uint32_t *result_list_size,
                                       tinytc_value_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_results();
        if (num_results > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many results");
        }
        auto const num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            auto results = instr->result_begin();
            auto const limit = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < limit; ++i) {
                result_list[i] = value(results[i]).release();
            }
        }
        *result_list_size = num;
    });
}
}
