// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "ir/node/inst_node.hpp"
#include "location.hpp"
#include "tinytc/tinytc.h"
#include "util.hpp"

#include <memory>
#include <type_traits>
#include <utility>

using namespace tinytc;

extern "C" {
char const *tinytc_binary_op_to_string(tinytc_binary_op_t op) {
    switch (op) {
    case tinytc_binary_op_add:
        return "add";
    case tinytc_binary_op_sub:
        return "sub";
    case tinytc_binary_op_mul:
        return "mul";
    case tinytc_binary_op_div:
        return "div";
    case tinytc_binary_op_rem:
        return "rem";
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

tinytc_status_t tinytc_binary_op_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                             tinytc_binary_op_t op, tinytc_value_t a,
                                             tinytc_value_t b, const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<binary_op_inst>(enum_cast<binary_op>(op), value(a, true),
                                                  value(b, true), get_optional(loc))
                     .release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                        tinytc_value_t a, tinytc_scalar_type_t to_ty,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<cast_inst>(value(a, true), enum_cast<scalar_type>(to_ty),
                                             get_optional(loc))
                     .release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                       tinytc_cmp_condition_t cond, tinytc_value_t a,
                                       tinytc_value_t b, const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<compare_inst>(enum_cast<cmp_condition>(cond), value(a, true),
                                                value(b, true), get_optional(loc))
                     .release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_neg_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                       tinytc_value_t a, const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<neg_inst>(value(a, true), get_optional(loc)).release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                          tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<alloca_inst>(data_type(ty, true), get_optional(loc)).release();
        *result = (*instr)->result().release();
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

tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                          tinytc_value_t a, int64_t mode,
                                          uint32_t expand_shape_size, tinytc_value_t *expand_shape,
                                          const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr || expand_shape == nullptr) {
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
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                        tinytc_value_t a, int64_t from, int64_t to,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<fuse_inst>(value(a, true), from, to, get_optional(loc)).release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                        tinytc_value_t a, uint32_t index_list_size,
                                        tinytc_value_t *index_list, const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr || (index_list_size > 0 && index_list == nullptr)) {
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
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                            const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<group_id_inst>(get_optional(loc)).release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                              const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<group_size_inst>(get_optional(loc)).release();
        *result = (*instr)->result().release();
    });
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

tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                        tinytc_value_t a, int64_t mode,
                                        const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *instr = std::make_unique<size_inst>(value(a, true), mode, get_optional(loc)).release();
        *result = (*instr)->result().release();
    });
}

tinytc_status_t tinytc_subview_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                           tinytc_value_t a, uint32_t slice_list_size,
                                           tinytc_slice_t *slice_list,
                                           const tinytc_location_t *loc) {
    if (instr == nullptr || result == nullptr || (slice_list_size > 0 && slice_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto slice_vec = std::vector<slice>();
        slice_vec.reserve(slice_list_size);
        for (uint32_t i = 0; i < slice_list_size; ++i) {
            slice_vec.emplace_back(value(slice_list[i].offset, true),
                                   value(slice_list[i].size, true));
        }
        *instr =
            std::make_unique<subview_inst>(value(a, true), std::move(slice_vec), get_optional(loc))
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

tinytc_status_t tinytc_sum_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                       tinytc_transpose_t tA, tinytc_value_t alpha,
                                       tinytc_value_t A, tinytc_value_t beta, tinytc_value_t B,
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

tinytc_status_t tinytc_inst_release(tinytc_inst_t instr) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ref_count = instr->dec_ref();
        if (ref_count == 0) {
            delete instr;
        }
    });
}

tinytc_status_t tinytc_inst_retain(tinytc_inst_t instr) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { instr->inc_ref(); });
}
}
