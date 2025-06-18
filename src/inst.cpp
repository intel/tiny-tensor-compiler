// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

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
    case tinytc_arithmetic_min:
        return "min";
    case tinytc_arithmetic_max:
        return "max";
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

char const *tinytc_builtin_to_string(tinytc_builtin_t b) {
    switch (b) {
    case tinytc_builtin_group_id_x:
        return "group_id.x";
    case tinytc_builtin_group_id_y:
        return "group_id.y";
    case tinytc_builtin_group_id_z:
        return "group_id.z";
    case tinytc_builtin_num_groups_x:
        return "num_groups.x";
    case tinytc_builtin_num_groups_y:
        return "num_groups.y";
    case tinytc_builtin_num_groups_z:
        return "num_groups.z";
    case tinytc_builtin_num_subgroups_x:
        return "num_subgroups.x";
    case tinytc_builtin_num_subgroups_y:
        return "num_subgroups.y";
    case tinytc_builtin_subgroup_size:
        return "subgroup_size";
    case tinytc_builtin_subgroup_id_x:
        return "subgroup_id.x";
    case tinytc_builtin_subgroup_id_y:
        return "subgroup_id.y";
    case tinytc_builtin_subgroup_linear_id:
        return "subgroup_linear_id";
    case tinytc_builtin_subgroup_local_id:
        return "subgroup_local_id";
    }
    return "unknown";
}

char const *tinytc_checked_flag_to_string(tinytc_checked_flag_t flag) {
    switch (flag) {
    case tinytc_checked_flag_none:
        return "";
    case tinytc_checked_flag_rows:
        return "rows_checked";
    case tinytc_checked_flag_cols:
        return "cols_checked";
    case tinytc_checked_flag_both:
        return "both_checked";
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

char const *tinytc_math_unary_to_string(tinytc_math_unary_t op) {
    switch (op) {
    case tinytc_math_unary_cos:
        return "cos";
    case tinytc_math_unary_sin:
        return "sin";
    case tinytc_math_unary_exp:
        return "exp";
    case tinytc_math_unary_exp2:
        return "exp2";
    case tinytc_math_unary_native_cos:
        return "native_cos";
    case tinytc_math_unary_native_sin:
        return "native_sin";
    case tinytc_math_unary_native_exp:
        return "native_exp";
    case tinytc_math_unary_native_exp2:
        return "native_exp2";
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
    case tinytc_store_flag_atomic_max:
        return "atomic_max";
    case tinytc_store_flag_atomic_min:
        return "atomic_min";
    }
    return "unknown";
}

char const *tinytc_group_arithmetic_to_string(tinytc_group_arithmetic_t op) {
    switch (op) {
    case tinytc_group_arithmetic_add:
        return "add";
    case tinytc_group_arithmetic_max:
        return "max";
    case tinytc_group_arithmetic_min:
        return "min";
    }
    return "unknown";
}

char const *tinytc_group_operation_to_string(tinytc_group_operation_t op) {
    switch (op) {
    case tinytc_group_operation_exclusive_scan:
        return "exclusive_scan";
    case tinytc_group_operation_inclusive_scan:
        return "inclusive_scan";
    case tinytc_group_operation_reduce:
        return "reduce";
    }
    return "unknown";
}

char const *tinytc_reduce_mode_to_string(tinytc_reduce_mode_t m) {
    switch (m) {
    case tinytc_reduce_mode_row:
        return "row";
    case tinytc_reduce_mode_column:
        return "column";
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
                                         tinytc_value_t a, tinytc_value_t b, tinytc_data_type_t ty,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = arith_inst::create(enum_cast<arithmetic>(op), a, b, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_arith_unary_inst_create(tinytc_inst_t *instr, tinytc_arithmetic_unary_t op,
                                               tinytc_value_t a, tinytc_data_type_t ty,
                                               const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            arith_unary_inst::create(enum_cast<arithmetic_unary>(op), a, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_barrier_inst_create(tinytc_inst_t *instr,
                                           tinytc_address_spaces_t fence_flags,
                                           const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = barrier_inst::create(fence_flags, get_optional(loc)); });
}

tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        tinytc_data_type_t to_ty, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = cast_inst::create(a, to_ty, get_optional(loc)); });
}

tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr, tinytc_cmp_condition_t cond,
                                       tinytc_value_t a, tinytc_value_t b, tinytc_data_type_t ty,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = compare_inst::create(enum_cast<cmp_condition>(cond), a, b, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_constant_inst_create_boolean(tinytc_inst_t *instr, tinytc_bool_t value,
                                                    tinytc_data_type_t ty,
                                                    const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = constant_inst::create(value != 0, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_constant_inst_create_complex(tinytc_inst_t *instr, double value_re,
                                                    double value_im, tinytc_data_type_t ty,
                                                    const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            constant_inst::create(std::complex<double>(value_re, value_im), ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_constant_inst_create_float(tinytc_inst_t *instr, double value,
                                                  tinytc_data_type_t ty,
                                                  const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = constant_inst::create(value, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_constant_inst_create_int(tinytc_inst_t *instr, int64_t value,
                                                tinytc_data_type_t ty,
                                                const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = constant_inst::create(value, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_constant_inst_create_one(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                                const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    if (const auto *bt = dyn_cast<boolean_data_type>(ty); bt != nullptr) {
        return exception_to_status_code(
            [&] { *instr = constant_inst::create(true, ty, get_optional(loc)); });
    }

    scalar_type sty;
    if (const auto *st = dyn_cast<scalar_data_type>(ty); st != nullptr) {
        sty = st->ty();
    } else if (const auto *ct = dyn_cast<coopmatrix_data_type>(ty); ct != nullptr) {
        sty = ct->component_ty();
    } else {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            *instr = constant_inst::create(std::int64_t{1}, ty, get_optional(loc));
            break;
        case scalar_type::bf16:
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            *instr = constant_inst::create(double{1}, ty, get_optional(loc));
            break;
        case scalar_type::c32:
        case scalar_type::c64:
            *instr = constant_inst::create(std::complex<double>{1}, ty, get_optional(loc));
            break;
        }
    });
}

tinytc_status_t tinytc_constant_inst_create_zero(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                                 const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    if (const auto *bt = dyn_cast<boolean_data_type>(ty); bt != nullptr) {
        return exception_to_status_code(
            [&] { *instr = constant_inst::create(false, ty, get_optional(loc)); });
    }

    scalar_type sty;
    if (const auto *st = dyn_cast<scalar_data_type>(ty); st != nullptr) {
        sty = st->ty();
    } else if (const auto *ct = dyn_cast<coopmatrix_data_type>(ty); ct != nullptr) {
        sty = ct->component_ty();
    } else {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            *instr = constant_inst::create(std::int64_t{0}, ty, get_optional(loc));
            break;
        case scalar_type::bf16:
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            *instr = constant_inst::create(double{0}, ty, get_optional(loc));
            break;
        case scalar_type::c32:
        case scalar_type::c64:
            *instr = constant_inst::create(std::complex<double>{0}, ty, get_optional(loc));
            break;
        }
    });
}

tinytc_status_t tinytc_cooperative_matrix_apply_inst_create(tinytc_inst_t *instr,
                                                            tinytc_value_t mat,
                                                            tinytc_data_type_t ty,
                                                            const tinytc_location_t *loc) {
    if (instr == nullptr || mat == nullptr || ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = cooperative_matrix_apply_inst::create(mat, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_cooperative_matrix_extract_inst_create(tinytc_inst_t *instr, int64_t index,
                                                              tinytc_value_t mat,
                                                              tinytc_data_type_t ty,
                                                              const tinytc_location_t *loc) {
    if (instr == nullptr || mat == nullptr || ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_extract_inst::create(index, mat, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_cooperative_matrix_insert_inst_create(tinytc_inst_t *instr, int64_t index,
                                                             tinytc_value_t val, tinytc_value_t mat,
                                                             tinytc_data_type_t ty,
                                                             const tinytc_location_t *loc) {
    if (instr == nullptr || val == nullptr || mat == nullptr || ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_insert_inst::create(index, val, mat, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_cooperative_matrix_load_inst_create(
    tinytc_inst_t *instr, tinytc_transpose_t trans, tinytc_checked_flag_t flag, tinytc_value_t op,
    tinytc_value_t p0, tinytc_value_t p1, tinytc_data_type_t to_ty, const tinytc_location_t *loc) {
    if (instr == nullptr || op == nullptr || p0 == nullptr || p1 == nullptr || to_ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_load_inst::create(enum_cast<transpose>(trans),
                                                      enum_cast<checked_flag>(flag), op, p0, p1,
                                                      to_ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_cooperative_matrix_mul_add_inst_create(tinytc_inst_t *instr,
                                                              tinytc_value_t a, tinytc_value_t b,
                                                              tinytc_value_t c,
                                                              tinytc_data_type_t to_ty,
                                                              const tinytc_location_t *loc) {
    if (instr == nullptr || a == nullptr || b == nullptr || c == nullptr || to_ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_mul_add_inst::create(a, b, c, to_ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_cooperative_matrix_prefetch_inst_create(tinytc_inst_t *instr,
                                                               int32_t cache_level, int32_t rows,
                                                               int32_t cols, tinytc_value_t op,
                                                               tinytc_value_t p0, tinytc_value_t p1,
                                                               const tinytc_location_t *loc) {
    if (instr == nullptr || op == nullptr || p0 == nullptr || p1 == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_prefetch_inst::create(cache_level, rows, cols, op, p0, p1,
                                                          get_optional(loc));
    });
}

tinytc_status_t tinytc_cooperative_matrix_scale_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                            tinytc_value_t b, tinytc_data_type_t ty,
                                                            const tinytc_location_t *loc) {
    if (instr == nullptr || a == nullptr || b == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = cooperative_matrix_scale_inst::create(a, b, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_cooperative_matrix_store_inst_create(tinytc_inst_t *instr,
                                                            tinytc_checked_flag_t cflag,
                                                            tinytc_store_flag_t sflag,
                                                            tinytc_value_t val, tinytc_value_t op,
                                                            tinytc_value_t p0, tinytc_value_t p1,
                                                            const tinytc_location_t *loc) {
    if (instr == nullptr || val == nullptr || op == nullptr || p0 == nullptr || p1 == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cooperative_matrix_store_inst::create(enum_cast<checked_flag>(cflag),
                                                       enum_cast<store_flag>(sflag), val, op, p0,
                                                       p1, get_optional(loc));
    });
}

tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *instr = alloca_inst::create(ty, get_optional(loc)); });
}

tinytc_status_t tinytc_axpby_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                         tinytc_transpose_t tA, tinytc_value_t alpha,
                                         tinytc_value_t A, tinytc_value_t beta, tinytc_value_t B,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = axpby_inst::create(bool(atomic), enum_cast<transpose>(tA), alpha, A, beta, B,
                                    get_optional(loc));
    });
}

tinytc_status_t tinytc_builtin_inst_create(tinytc_inst_t *instr, tinytc_builtin_t btype,
                                           tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = builtin_inst::create(enum_cast<builtin>(btype), ty, get_optional(loc)); });
}

tinytc_status_t tinytc_cumsum_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic, int64_t mode,
                                          tinytc_value_t alpha, tinytc_value_t A,
                                          tinytc_value_t beta, tinytc_value_t B,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = cumsum_inst::create(bool(atomic), mode, alpha, A, beta, B, get_optional(loc));
    });
}

tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr, int64_t expanded_mode,
                                          uint32_t static_expand_shape_size,
                                          const int64_t *static_expand_shape, tinytc_value_t a,
                                          uint32_t expand_shape_size,
                                          const tinytc_value_t *expand_shape, tinytc_data_type_t ty,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr || static_expand_shape == nullptr ||
        (expand_shape_size > 0 && expand_shape == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = expand_inst::create(
            expanded_mode, array_view{static_expand_shape, static_expand_shape_size}, a,
            array_view{expand_shape, expand_shape_size}, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, int64_t from, int64_t to,
                                        tinytc_value_t a, tinytc_data_type_t ty,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = fuse_inst::create(from, to, a, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                        uint32_t index_list_size, const tinytc_value_t *index_list,
                                        tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (instr == nullptr || (index_list_size > 0 && index_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            load_inst::create(a, array_view{index_list, index_list_size}, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_gemm_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                        tinytc_transpose_t tA, tinytc_transpose_t tB,
                                        tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                                        tinytc_value_t beta, tinytc_value_t C,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = gemm_inst::create(bool(atomic), enum_cast<transpose>(tA), enum_cast<transpose>(tB),
                                   alpha, A, B, beta, C, get_optional(loc));
    });
}

tinytc_status_t tinytc_gemv_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                        tinytc_transpose_t tA, tinytc_value_t alpha,
                                        tinytc_value_t A, tinytc_value_t B, tinytc_value_t beta,
                                        tinytc_value_t C, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = gemv_inst::create(bool(atomic), enum_cast<transpose>(tA), alpha, A, B, beta, C,
                                   get_optional(loc));
    });
}

tinytc_status_t tinytc_ger_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                       tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                                       tinytc_value_t beta, tinytc_value_t C,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = ger_inst::create(bool(atomic), alpha, A, B, beta, C, get_optional(loc)); });
}

tinytc_status_t tinytc_hadamard_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                            tinytc_value_t alpha, tinytc_value_t A,
                                            tinytc_value_t B, tinytc_value_t beta, tinytc_value_t C,
                                            const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = hadamard_inst::create(bool(atomic), alpha, A, B, beta, C, get_optional(loc));
    });
}

tinytc_status_t tinytc_math_unary_inst_create(tinytc_inst_t *instr, tinytc_math_unary_t op,
                                              tinytc_value_t a, tinytc_data_type_t ty,
                                              const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = math_unary_inst::create(enum_cast<math_unary>(op), a, ty, get_optional(loc));
    });
}

tinytc_status_t tinytc_parallel_inst_create(tinytc_inst_t *instr, const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *instr = parallel_inst::create(get_optional(loc)); });
}

tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, int64_t mode, tinytc_value_t a,
                                        tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (instr == nullptr || a == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = size_inst::create(mode, a, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_subgroup_broadcast_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                      tinytc_value_t idx, tinytc_data_type_t ty,
                                                      const tinytc_location_t *loc) {
    if (instr == nullptr || a == nullptr || idx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *instr = subgroup_broadcast_inst::create(a, idx, ty, get_optional(loc)); });
}

tinytc_status_t tinytc_subgroup_operation_inst_create(tinytc_inst_t *instr,
                                                      tinytc_group_arithmetic_t arith,
                                                      tinytc_group_operation_t operation,
                                                      tinytc_value_t a, tinytc_data_type_t ty,
                                                      const tinytc_location_t *loc) {
    if (instr == nullptr || a == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = subgroup_operation_inst::create(enum_cast<group_arithmetic>(arith),
                                                 enum_cast<group_operation>(operation), a, ty,
                                                 get_optional(loc));
    });
}

tinytc_status_t tinytc_subview_inst_create(tinytc_inst_t *instr, uint32_t static_list_size,
                                           const int64_t *static_offset_list,
                                           const int64_t *static_size_list, tinytc_value_t a,
                                           uint32_t offset_list_size,
                                           const tinytc_value_t *offset_list,
                                           uint32_t size_list_size, const tinytc_value_t *size_list,
                                           tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (instr == nullptr ||
        (static_list_size > 0 && (static_offset_list == nullptr || static_size_list == nullptr)) ||
        (offset_list_size > 0 && offset_list == nullptr) ||
        (size_list_size > 0 && size_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = subview_inst::create(array_view{static_offset_list, static_list_size},
                                      array_view{static_size_list, static_list_size}, a,
                                      array_view{offset_list, offset_list_size},
                                      array_view{size_list, size_list_size}, ty, get_optional(loc));
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
        *instr = store_inst::create(enum_cast<store_flag>(flag), val, a,
                                    array_view{index_list, index_list_size}, get_optional(loc));
    });
}

tinytc_status_t tinytc_sum_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                       tinytc_transpose_t tA, tinytc_value_t alpha,
                                       tinytc_value_t A, tinytc_value_t beta, tinytc_value_t B,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = sum_inst::create(bool(atomic), enum_cast<transpose>(tA), alpha, A, beta, B,
                                  get_optional(loc));
    });
}

tinytc_status_t tinytc_for_inst_create(tinytc_inst_t *instr, tinytc_scalar_type_t loop_var_type,
                                       tinytc_value_t from, tinytc_value_t to, tinytc_value_t step,
                                       uint32_t init_return_list_size,
                                       const tinytc_value_t *initial_value_list,
                                       const tinytc_data_type_t *return_type_list,
                                       const tinytc_location_t *loc) {
    if (instr == nullptr || from == nullptr || to == nullptr ||
        (init_return_list_size != 0 &&
         (initial_value_list == nullptr || return_type_list == nullptr))) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = for_inst::create(enum_cast<scalar_type>(loop_var_type), from, to, step,
                                  array_view{initial_value_list, init_return_list_size},
                                  array_view{return_type_list, init_return_list_size},
                                  get_optional(loc));
    });
}

tinytc_status_t tinytc_foreach_inst_create(tinytc_inst_t *instr, tinytc_scalar_type_t loop_var_type,
                                           uint32_t dim, const tinytc_value_t *from_list,
                                           const tinytc_value_t *to_list,
                                           const tinytc_location_t *loc) {
    if (instr == nullptr || from_list == nullptr || to_list == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr =
            foreach_inst::create(enum_cast<scalar_type>(loop_var_type), array_view{from_list, dim},
                                 array_view{to_list, dim}, get_optional(loc));
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
        *instr = if_inst::create(condition, array_view{return_type_list, return_type_list_size},
                                 get_optional(loc));
    });
}

tinytc_status_t tinytc_yield_inst_create(tinytc_inst_t *instr, uint32_t yield_list_size,
                                         const tinytc_value_t *yield_list,
                                         const tinytc_location_t *loc) {
    if (instr == nullptr || (yield_list_size != 0 && yield_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = yield_inst::create(array_view{yield_list, yield_list_size}, get_optional(loc));
    });
}

void tinytc_inst_destroy(tinytc_inst_t obj) { tinytc_inst::destroy(obj); }

tinytc_status_t tinytc_inst_get_parent_region(tinytc_inst_t instr, tinytc_region_t *parent) {
    if (instr == nullptr || parent == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *parent = instr->parent(); });
}

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

tinytc_status_t tinytc_inst_set_attr(tinytc_inst_t instr, tinytc_attr_t a) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { instr->attr(a); });
}
}
