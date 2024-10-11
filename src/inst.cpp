// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "error.hpp"
#include "location.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>

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
    case tinytc_arithmetic_unary_abs:
        return "abs";
    case tinytc_arithmetic_unary_not:
        return "not";
    case tinytc_arithmetic_unary_neg:
        return "neg";
    case tinytc_arithmetic_unary_conj:
        return "conj";
    case tinytc_arithmetic_unary_im:
        return "im";
    case tinytc_arithmetic_unary_re:
        return "re";
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

char const *tinytc_store_flag_to_string(tinytc_store_flag_t flag) {
    switch (flag) {
    case tinytc_store_flag_regular:
        return "";
    case tinytc_store_flag_atomic:
        return "atomic";
    case tinytc_store_flag_atomic_add:
        return "atomic_add";
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
        *instr = std::make_unique<arith_inst>(enum_cast<arithmetic>(op), a, b, get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_arith_unary_inst_create(tinytc_inst_t *instr, tinytc_arithmetic_unary_t op,
                                               tinytc_value_t a, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<arith_unary_inst>(enum_cast<arithmetic_unary>(op), a,
                                                    get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        tinytc_data_type_t to_ty, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<cast_inst>(a, to_ty, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr, tinytc_cmp_condition_t cond,
                                       tinytc_value_t a, tinytc_value_t b,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<compare_inst>(enum_cast<cmp_condition>(cond), a, b, get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_constant_inst_create_complex(tinytc_inst_t *instr, double value_re,
                                                    double value_im, tinytc_data_type_t ty,
                                                    const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<constant_inst>(std::complex<double>(value_re, value_im), ty,
                                                 get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_constant_inst_create_float(tinytc_inst_t *instr, double value,
                                                  tinytc_data_type_t ty,
                                                  const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<constant_inst>(value, ty, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_constant_inst_create_int(tinytc_inst_t *instr, int64_t value,
                                                tinytc_data_type_t ty,
                                                const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<constant_inst>(value, ty, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<alloca_inst>(ty, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_axpby_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                         tinytc_bool_t atomic, tinytc_value_t alpha,
                                         tinytc_value_t A, tinytc_value_t beta, tinytc_value_t B,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<axpby_inst>(enum_cast<transpose>(tA), alpha, A, beta, B,
                                              bool(atomic), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                          int64_t expanded_mode, uint32_t static_expand_shape_size,
                                          const int64_t *static_expand_shape,
                                          uint32_t expand_shape_size,
                                          const tinytc_value_t *expand_shape,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr || static_expand_shape == nullptr ||
        (expand_shape_size > 0 && expand_shape == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<expand_inst>(
                     a, expanded_mode, array_view{static_expand_shape, static_expand_shape_size},
                     array_view{expand_shape, expand_shape_size}, get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, tinytc_value_t a, int64_t from,
                                        int64_t to, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<fuse_inst>(a, from, to, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        uint32_t index_list_size, const tinytc_value_t *index_list,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr || (index_list_size > 0 && index_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<load_inst>(a, array_view{index_list, index_list_size},
                                             get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr, tinytc_compiler_context_t ctx,
                                            const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<group_id_inst>(ctx, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr, tinytc_compiler_context_t ctx,
                                              const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<group_size_inst>(ctx, get_optional(loc)).release(); });
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
                                             alpha, A, B, beta, C, bool(atomic), get_optional(loc))
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
        *instr = std::make_unique<gemv_inst>(enum_cast<transpose>(tA), alpha, A, B, beta, C,
                                             bool(atomic), get_optional(loc))
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
        *instr = std::make_unique<ger_inst>(alpha, A, B, beta, C, bool(atomic), get_optional(loc))
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
        *instr =
            std::make_unique<hadamard_inst>(alpha, A, B, beta, C, bool(atomic), get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_num_subgroups_inst_create(tinytc_inst_t *instr,
                                                 tinytc_compiler_context_t ctx,
                                                 const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<num_subgroups_inst>(ctx, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_parallel_inst_create(tinytc_inst_t *instr, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<parallel_inst>(get_optional(loc)).release(); });
}

tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, tinytc_value_t a, int64_t mode,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<size_inst>(a, mode, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_subgroup_id_inst_create(tinytc_inst_t *instr, tinytc_compiler_context_t ctx,
                                               const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<subgroup_id_inst>(ctx, get_optional(loc)).release(); });
}

tinytc_status_t tinytc_subgroup_local_id_inst_create(tinytc_inst_t *instr,
                                                     tinytc_compiler_context_t ctx,
                                                     const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<subgroup_local_id_inst>(ctx, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_subgroup_size_inst_create(tinytc_inst_t *instr,
                                                 tinytc_compiler_context_t ctx,
                                                 const tinytc_location_t *loc) {
    if (instr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = std::make_unique<subgroup_size_inst>(ctx, get_optional(loc)).release(); });
}

tinytc_status_t
tinytc_subview_inst_create(tinytc_inst_t *instr, tinytc_value_t a, uint32_t static_list_size,
                           const int64_t *static_offset_list, const int64_t *static_size_list,
                           uint32_t offset_list_size, const tinytc_value_t *offset_list,
                           uint32_t size_list_size, const tinytc_value_t *size_list,
                           const tinytc_location_t *loc) {
    if (instr == nullptr ||
        (static_list_size > 0 && (static_offset_list == nullptr || static_size_list == nullptr)) ||
        (offset_list_size > 0 && offset_list == nullptr) ||
        (size_list_size > 0 && size_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<subview_inst>(a, array_view{static_offset_list, static_list_size},
                                           array_view{static_size_list, static_list_size},
                                           array_view{offset_list, offset_list_size},
                                           array_view{size_list, size_list_size}, get_optional(loc))
                .release();
    });
}

tinytc_status_t tinytc_store_inst_create(tinytc_inst_t *instr, tinytc_store_flag_t flag,
                                         tinytc_value_t val, tinytc_value_t a,
                                         uint32_t index_list_size, const tinytc_value_t *index_list,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr || (index_list_size > 0 && index_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<store_inst>(enum_cast<store_flag>(flag), val, a,
                                         array_view{index_list, index_list_size}, get_optional(loc))
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
        *instr = std::make_unique<sum_inst>(enum_cast<transpose>(tA), alpha, A, beta, B,
                                            bool(atomic), get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_for_inst_create(tinytc_inst_t *instr, tinytc_value_t from, tinytc_value_t to,
                                       tinytc_value_t step, tinytc_data_type_t loop_var_type,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr || loop_var_type == nullptr || from == nullptr || to == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<for_inst>(from, to, step, loop_var_type, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_foreach_inst_create(tinytc_inst_t *instr, tinytc_value_t from,
                                           tinytc_value_t to, tinytc_data_type_t loop_var_type,
                                           const tinytc_location_t *loc) {
    if (instr == nullptr || loop_var_type == nullptr || from == nullptr || to == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<foreach_inst>(from, to, loop_var_type, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_if_inst_create(tinytc_inst_t *instr, tinytc_value_t condition,
                                      uint32_t return_type_list_size,
                                      const tinytc_data_type_t *return_type_list,
                                      const tinytc_location_t *loc) {
    if (instr == nullptr || condition == nullptr ||
        (return_type_list_size > 0 && return_type_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<if_inst>(condition,
                                           array_view{return_type_list, return_type_list_size},
                                           get_optional(loc))
                     .release();
    });
}

tinytc_status_t tinytc_yield_inst_create(tinytc_inst_t *instr, uint32_t yield_list_size,
                                         const tinytc_value_t *yield_list,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr || (yield_list_size != 0 && yield_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            std::make_unique<yield_inst>(array_view{yield_list, yield_list_size}, get_optional(loc))
                .release();
    });
}

void tinytc_inst_destroy(tinytc_inst_t obj) { delete obj; }

tinytc_status_t tinytc_inst_get_values(tinytc_inst_t instr, uint32_t *result_list_size,
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
        auto num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            num = std::min(num, *result_list_size);
            auto results = instr->result_begin();
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}

tinytc_status_t tinytc_inst_get_regions(tinytc_inst_t instr, uint32_t *result_list_size,
                                        tinytc_region_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_child_regions();
        if (num_results > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many results");
        }
        auto num = static_cast<std::uint32_t>(num_results);
        if (*result_list_size > 0) {
            auto results = instr->child_regions_begin();
            num = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}
}
