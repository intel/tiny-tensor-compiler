// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240409_H
#define TINYTC_20240409_H

#include "tinytc/export.h"
#include "tinytc/types.h"
#include "tinytc/version.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
/////////// Error //////////
////////////////////////////

#define TINYTC_CHECK_STATUS(X)                                                                     \
    do {                                                                                           \
        tinytc_status_t status = X;                                                                \
        if (status != tinytc_status_success) {                                                     \
            return status;                                                                         \
        }                                                                                          \
    } while (0)

/**
 * @brief Translate status code to textual description
 *
 * @param status [in] status code
 *
 * @return String
 */
TINYTC_EXPORT char const *tinytc_error_string(tinytc_status_t status);

////////////////////////////
//////// Scalar type ///////
////////////////////////////

//! Convert scalar type to string
TINYTC_EXPORT char const *tinytc_scalar_type_to_string(tinytc_scalar_type_t ty);
//! Size of scalar type in bytes
TINYTC_EXPORT size_t tinytc_scalar_type_size(tinytc_scalar_type_t ty);

////////////////////////////
///////// Data type ////////
////////////////////////////

/**
 * @brief Create scalar data type
 *
 * @param dt [out] pointer to the data type object created
 * @param type [in] scalar type
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_scalar_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t type,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create memref data type
 *
 * @param dt [out] pointer to the data type object created
 * @param scalar_ty [in] element type
 * @param shape_size [in] tensor order; number of elements in shape array, must be 0 if shape ==
 * nullptr
 * @param shape [in][range(0, shape_size)] array of mode sizes
 * @param stride_size [in][optional] number of elements in stride array; must be either 0 for
 * automatic stride calculation or must match shape_size; must be 0 if stride == nullptr
 * @param stride [in][optional][range(0, stride_size)] stride array
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_memref_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t scalar_ty,
                                                        uint32_t shape_size, const int64_t *shape,
                                                        uint32_t stride_size, const int64_t *stride,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create group data type
 *
 * @param dt [out] pointer to the data type object created
 * @param memref_ty [in] memref data type object
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty,
                                                       const tinytc_location_t *loc);

/**
 * @brief Release data type object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt);

/**
 * @brief Increase reference count of data type object by 1
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_retain(tinytc_data_type_t dt);

////////////////////////////
/////////// Value //////////
////////////////////////////

/**
 * @brief Create value
 *
 * @param vl [out] pointer to the value object created
 * @param type [in] data type object
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_create(tinytc_value_t *vl, tinytc_data_type_t type,
                                                  const tinytc_location_t *loc);

/**
 * @brief Create floating point immediate value
 *
 * @param vl [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_float_imm_create(tinytc_value_t *vl, double imm,
                                                      tinytc_scalar_type_t type,
                                                      const tinytc_location_t *loc);
/**
 * @brief Create integer immediate value
 *
 * @param vl [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_int_imm_create(tinytc_value_t *vl, int64_t imm,
                                                    tinytc_scalar_type_t type,
                                                    const tinytc_location_t *loc);

/**
 * @brief Release value object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param vl [inout] value object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_release(tinytc_value_t vl);

/**
 * @brief Increase reference count of value object by 1
 *
 * @param vl [inout] value object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_retain(tinytc_value_t vl);

/**
 * @brief Set name of value
 *
 * @param vl [inout] value object
 * @param name [in] name; null-terminated string
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_set_name(tinytc_value_t vl, char const *name);

/**
 * @brief Get name of value
 *
 * The returned pointer may be invalidated if the value or any node in the abstract syntax
 * tree referencing the value is modified.
 *
 * @param vl [in] value object
 * @param name [out] pointer to C string
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_get_name(const_tinytc_value_t vl, char const **name);

////////////////////////////
/////// Instructions ///////
////////////////////////////

//! Convert binary op to string
TINYTC_EXPORT char const *tinytc_binary_op_to_string(tinytc_binary_op_t op);
//! Convert cmp condition to string
TINYTC_EXPORT char const *tinytc_cmp_condition_to_string(tinytc_cmp_condition_t cond);
//! Convert transpose to string
TINYTC_EXPORT char const *tinytc_transpose_to_string(tinytc_transpose_t t);

/**
 * @brief Create binary op instruction
 *
 * @code %value = <binary_op> %a, %b : type(%a) ; type(%a) == type(%b) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param op [in] binary op type
 * @param a [in] left-hand operand
 * @param b [in] right-hand operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_op_inst_create(tinytc_inst_t *instr,
                                                           tinytc_binary_op_t op, tinytc_value_t a,
                                                           tinytc_value_t b,
                                                           const tinytc_location_t *loc);

/**
 * @brief Create cast instruction
 *
 * @code %value = cast %a, %b : type(%a) -> %to_ty @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param to_ty [in] target type
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                      tinytc_scalar_type_t to_ty,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create binary op instruction
 *
 * @code %value = cmp.<cond> %a, %b : type(%a) ; type(%a) == type(%b) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param cond [in] compare type
 * @param a [in] left-hand operand
 * @param b [in] right-hand operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr,
                                                     tinytc_cmp_condition_t cond, tinytc_value_t a,
                                                     tinytc_value_t b,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create neg instruction
 *
 * @code %value = neg %a : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_neg_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create alloca instruction
 *
 * @code %value = alloca -> %ty @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param ty [in] type that is allocated
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr, tinytc_data_type_t ty,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create axpby instruction
 *
 * @code
 * axpby.<tA>.<atomic> %alpha, %A, %beta, %B : type(%alpha), type(%A), type(%beta), type(%B)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param tA [in] operation applied on A
 * @param atomic [in] true for atomic updates of B
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param beta [in] @$\beta@$
 * @param B [in] B
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_axpby_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                                       tinytc_bool_t atomic, tinytc_value_t alpha,
                                                       tinytc_value_t A, tinytc_value_t beta,
                                                       tinytc_value_t B,
                                                       const tinytc_location_t *loc);

/**
 * @brief Create expand instruction
 *
 * @code %value = expand %a[%mode -> %expand_shape] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param mode [in] expanded mode
 * @param expand_shape_size [in] dimension of expand shape; must be at least 2
 * @param expand_shape [in][range(2, expand_shape_size)] expand shape array
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                        int64_t mode, uint32_t expand_shape_size,
                                                        tinytc_value_t *expand_shape,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create fuse instruction
 *
 * @code %value = fuse %a[%from, %to] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param from [in] first mode to fuse
 * @param to [in] last mode to fuse
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                      int64_t from, int64_t to,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create load instruction
 *
 * @code %value = load %a[%index_list] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param index_list_size [in] number of indices
 * @param index_list [in][range(0, index_list_size)] indices array; may be nullptr if
 * index_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                      uint32_t index_list_size,
                                                      tinytc_value_t *index_list,
                                                      const tinytc_location_t *loc);
/**
 * @brief Create group_id instruction
 *
 * @code %value = group_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr,
                                                          const tinytc_location_t *loc);

/**
 * @brief Create group_size instruction
 *
 * @code %value = group_size @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr,
                                                            const tinytc_location_t *loc);

/**
 * @brief Create GEMM instruction
 *
 * @code
 * gemm.<tA>.<tB>.<atomic> %alpha, %A, %B, %beta, %C
 *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param tA [in] operation applied on A
 * @param tB [in] operation applied on B
 * @param atomic [in] true for atomic updates of C
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @$\beta@$
 * @param C [in] C
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_gemm_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                                      tinytc_transpose_t tB, tinytc_bool_t atomic,
                                                      tinytc_value_t alpha, tinytc_value_t A,
                                                      tinytc_value_t B, tinytc_value_t beta,
                                                      tinytc_value_t C,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create GEMV instruction
 *
 * @code
 * gemv.<tA>.<atomic> %alpha, %A, %B, %beta, %C
 *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param tA [in] operation applied on A
 * @param atomic [in] true for atomic updates of C
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @$\beta@$
 * @param C [in] C
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_gemv_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                                      tinytc_bool_t atomic, tinytc_value_t alpha,
                                                      tinytc_value_t A, tinytc_value_t B,
                                                      tinytc_value_t beta, tinytc_value_t C,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create GER instruction
 *
 * @code
 * ger.<atomic> %alpha, %A, %B, %beta, %C
 *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param atomic [in] true for atomic updates of C
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @$\beta@$
 * @param C [in] C
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ger_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                                     tinytc_value_t alpha, tinytc_value_t A,
                                                     tinytc_value_t B, tinytc_value_t beta,
                                                     tinytc_value_t C,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create Hadamard instruction
 *
 * @code
 * hadamard.<atomic> %alpha, %A, %B, %beta, %C
 *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param atomic [in] true for atomic updates of C
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @$\beta@$
 * @param C [in] C
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_hadamard_inst_create(
    tinytc_inst_t *instr, tinytc_bool_t atomic, tinytc_value_t alpha, tinytc_value_t A,
    tinytc_value_t B, tinytc_value_t beta, tinytc_value_t C, const tinytc_location_t *loc);

/**
 * @brief Create size instruction
 *
 * @code %value = size %a[%mode] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param mode [in] mode for that the size is queried
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                      int64_t mode, const tinytc_location_t *loc);

/**
 * @brief Create subview instruction
 *
 * @code %value = subview %a[%offset1:%size1,...,%offsetN:%sizeN] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param slice_list_size [in] number of slices
 * @param offset_list [in][range(0, slice_list_size)] offset array; may be nullptr if
 * slice_list_size is 0
 * @param size_list [in][range(0, slice_list_size)] size array; may be nullptr if slice_list_size
 * is 0; size_list[i] may be nullptr if a single offset shall be passed instead of a range for the
 * i-th mode
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subview_inst_create(tinytc_inst_t *instr, tinytc_value_t a,
                                                         uint32_t slice_list_size,
                                                         tinytc_value_t *offset_list,
                                                         tinytc_value_t *size_list,
                                                         const tinytc_location_t *loc);

/**
 * @brief Create store instruction
 *
 * @code store %val, %a[%index_list] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param val [in] value to store
 * @param a [in] operand
 * @param index_list_size [in] number of indices
 * @param index_list [in][range(0, index_list_size)] indices array; may be nullptr if
 * index_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_store_inst_create(tinytc_inst_t *instr, tinytc_value_t val,
                                                       tinytc_value_t a, uint32_t index_list_size,
                                                       tinytc_value_t *index_list,
                                                       const tinytc_location_t *loc);

/**
 * @brief Create sum instruction
 *
 * @code
 * sum.<tA>.<atomic> %alpha, %A, %beta, %B : type(%alpha), type(%A), type(%beta), type(%B)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param tA [in] operation applied on A
 * @param atomic [in] true for atomic updates of C
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param beta [in] @$\beta@$
 * @param B [in] B
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_sum_inst_create(tinytc_inst_t *instr, tinytc_transpose_t tA,
                                                     tinytc_bool_t atomic, tinytc_value_t alpha,
                                                     tinytc_value_t A, tinytc_value_t beta,
                                                     tinytc_value_t B,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create for loop
 *
 * @code
 * for %loop_var = %from, %to, %step : type(%loop_var) { %body }
 * ; type(%loop_var) == type(%from)
 * ; type(%loop_var) == type(%to)
 * ; type(%loop_var) == type(%step)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loop_var [in] loop variable
 * @param from [in] loop begion
 * @param to [in] loop bound
 * @param step [in][optional] loop step; can be nullptr
 * @param body [in] loop body
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_for_inst_create(tinytc_inst_t *instr, tinytc_value_t loop_var,
                                                     tinytc_value_t from, tinytc_value_t to,
                                                     tinytc_value_t step, tinytc_region_t body,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create foreach loop
 *
 * @code
 * foreach %loop_var = %from, %to : type(%loop_var) { %body }
 * ; type(%loop_var) == type(%from)
 * ; type(%loop_var) == type(%to)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loop_var [in] loop variable
 * @param from [in] loop begion
 * @param to [in] loop bound
 * @param body [in] loop body
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_foreach_inst_create(tinytc_inst_t *instr,
                                                         tinytc_value_t loop_var,
                                                         tinytc_value_t from, tinytc_value_t to,
                                                         tinytc_region_t body,
                                                         const tinytc_location_t *loc);

/**
 * @brief Create if condition
 *
 * @code
 * if %condition { %then } else { %otherwise }
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param condition [in] condition
 * @param then [in] region taken if condition is true
 * @param otherwise [in][optional] region taken if condition is false; can be nullptr
 * @param return_type_list_size [in] length of return type array
 * @param return_type_list [in][range(0, return_type_list_size)] return type array; can be nullptr
 * if return_type_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_if_inst_create(tinytc_inst_t *instr, tinytc_value_t condition,
                                                    tinytc_region_t then, tinytc_region_t otherwise,
                                                    uint32_t return_type_list_size,
                                                    tinytc_scalar_type_t *return_type_list,
                                                    const tinytc_location_t *loc);

/**
 * @brief Create yield instruction
 *
 * @code
 * yield %identifier1, ..., %identifierN : type(%identifier1), ..., type(%identifierN)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param yield_list_size [in] length of yielded values list; must be at least 1
 * @param yield_list [in][range(1, yield_list_size)] yielded values array
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_yield_inst_create(tinytc_inst_t *instr,
                                                       uint32_t yield_list_size,
                                                       tinytc_value_t *yield_list,
                                                       const tinytc_location_t *loc);

/**
 * @brief Release inst object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param instr [inout] inst object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_release(tinytc_inst_t instr);

/**
 * @brief Increase reference count of inst object by 1
 *
 * @param instr [inout] inst object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_retain(tinytc_inst_t instr);

/**
 * @brief Get value produced by instruction
 *
 * @param instr [in] inst object
 * @param result [out] result value; may be set to nullptr if instruction does not return a value
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_get_value(const_tinytc_inst_t instr,
                                                    tinytc_value_t *result);

/**
 * @brief Get values produced by instruction
 *
 * Function can be called with result_list_size = 0 and result_list = nullptr in order to obtain
 * the number of results
 *
 * @param instr [in] inst object
 * @param result_list_size [inout] number of results to fetch; is updated with the actual value
 * @param result_list [out][range(0, result_list_size)] user-provided memory for storing result
 * handles; at most result_list_size values are written; can be nullptr if result_list_size is 0
 * @param result [out] result value; may be set to nullptr if instruction does not return a value
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_get_values(const_tinytc_inst_t instr,
                                                     uint32_t *result_list_size,
                                                     tinytc_value_t *result_list);

////////////////////////////
////////// Region //////////
////////////////////////////

/**
 * @brief Create region
 *
 * @param reg [out] pointer to the region object created
 * @param instruction_list_size [in] length of instruction array
 * @param instruction_list [in][range(0, instruction_list_size)] instruction array; can be nullptr
 * if instruction_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_create(tinytc_region_t *reg,
                                                   uint32_t instruction_list_size,
                                                   tinytc_inst_t *instruction_list,
                                                   const tinytc_location_t *loc);
/**
 * @brief Release region object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param reg [inout] region object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_release(tinytc_region_t reg);

/**
 * @brief Increase reference count of region object by 1
 *
 * @param reg [inout] region object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_retain(tinytc_region_t reg);

////////////////////////////
/////////// Func ///////////
////////////////////////////

/**
 * @brief Create function prototype
 *
 * @param fun [out] pointer to the func object created
 * @param arg_list_size [in] length of argument array
 * @param arg_list [in][range(0, arg_list_size)] argument array; can be nullptr if arg_list_size is
 * 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_prototype_create(tinytc_func_t *fun, char const *name,
                                                               uint32_t arg_list_size,
                                                               tinytc_value_t *arg_list,
                                                               const tinytc_location_t *loc);

/**
 * @brief Create function
 *
 * @param fun [out] pointer to the func object created
 * @param prototype [in] function prototype
 * @param body [in] function body
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_create(tinytc_func_t *fun, tinytc_func_t prototype,
                                                     tinytc_region_t body,
                                                     const tinytc_location_t *loc);

/**
 * @brief Set work-group size
 *
 * @param fun [out] func object (must be the function definition, not the function prototype)
 * @param x [in] number of rows in parallel grid; must be a multiple of the subgroup size
 * @param y [in] number of columns in parallel grid
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_set_work_group_size(tinytc_func_t fun, uint32_t x,
                                                                  uint32_t y);
/**
 * @brief Set subgroup size
 *
 * @param fun [out] func object (must be the function definition, not the function prototype)
 * @param sgs [in] subgroup size; the supported values need to be queried from the compute device
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_set_subgroup_size(tinytc_func_t fun, uint32_t sgs);

/**
 * @brief Release function object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param fun [inout] function object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_release(tinytc_func_t fun);

/**
 * @brief Increase reference count of function object by 1
 *
 * @param fun [inout] function object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_retain(tinytc_func_t fun);

////////////////////////////
/////////// Prog ///////////
////////////////////////////

/**
 * @brief Create program
 *
 * @param prg [out] pointer to the prog object created
 * @param fun_list_size [in] length of func array
 * @param fun_list [in][range(0, fun_list_size)] func array; can be nullptr if fun_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_program_create(tinytc_prog_t *prg, uint32_t fun_list_size,
                                                    tinytc_func_t *fun_list,
                                                    const tinytc_location_t *loc);

/**
 * @brief Release program object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param prg [inout] program object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_release(tinytc_prog_t prg);

/**
 * @brief Increase reference count of program object by 1
 *
 * @param prg [inout] program object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_retain(tinytc_prog_t prg);

////////////////////////////
// Visitors and transforms /
////////////////////////////

/**
 * @brief Dump program to stderr
 *
 * @param prg [in] program object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_dump(const_tinytc_prog_t prg);

/**
 * @brief Print program to file
 *
 * @param prg [in] program object
 * @param filename [in] filename
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_print_to_file(const_tinytc_prog_t prg,
                                                        char const *filename);

/**
 * @brief Print program to string
 *
 * The user is responsible to dispose the string with tinytc_string_destroy.
 *
 * @param prg [in] program object
 * @param str [out] pointer to string
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_print_to_string(const_tinytc_prog_t prg, char **str);

/**
 * @brief Delete a string returned from tinytc API
 *
 * @param str [in] string
 */
TINYTC_EXPORT void tinytc_string_destroy(char *str);

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Look up core info for Intel GPU architecture
 *
 * @param info [out] pointer to the core_info object created
 * @param arch [in] IP version
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_intel_create_from_arch(
    tinytc_core_info_t *info, tinytc_intel_gpu_architecture_t arch);

/**
 * @brief Create core_info for Intel GPUs
 *
 * @param info [out] pointer to the core_info object created
 * @param ip_version [in] IP version of architecture
 * @param num_eus_per_subslice [in] Number of Execution Units (Xe Vector Engines) per subslice (Xe
 * Core)
 * @param num_threads_per_eu [in] Number of threads per Execution Unit (Xe Vector Engine)
 * @param local_memory_size [in] Size of shared local memory
 * @param sgs_size [in] Length of sgs array
 * @param sgs [in] Allowed subgroup sizes
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_intel_create(tinytc_core_info_t *info,
                                                            uint32_t ip_version,
                                                            uint32_t num_eus_per_subslice,
                                                            uint32_t num_threads_per_eu,
                                                            uint32_t local_memory_size,
                                                            uint32_t sgs_size, uint32_t const *sgs);

/**
 * @brief Returns IP version
 *
 * @param info [in] core info object
 * @param ip_version [out] pointer to IP version
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_ip_version(const_tinytc_core_info_t info,
                                                              uint32_t *ip_version);

/**
 * @brief Returns available subgroup sizes
 *
 * @param info [in] core info object
 * @param sgs_size [out] pointer to number of subgroup sizes
 * @param sgs [out] pointer to subgroup size array; pointer is invalidated when core info is deleted
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_subgroup_sizes(const_tinytc_core_info_t info,
                                                                  uint32_t *sgs_size,
                                                                  uint32_t const **sgs);

/**
 * @brief Returns size of one register size in bytes
 *
 * @param info [in] core info object
 * @param size [out] pointer to register size
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_register_size(const_tinytc_core_info_t info,
                                                                 uint32_t *size);

/**
 * @brief Returns available number of registers per subgroup
 *
 * @param info [in] core info object
 * @param size [out] pointer to number of registers
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t
tinytc_core_info_get_num_registers_per_thread(const_tinytc_core_info_t info, uint32_t *num);

/**
 * @brief Request core feature
 *
 * @param info [in] core info object
 * @param flag [in] feature flag
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_set_core_feature(tinytc_core_info_t info,
                                                                tinytc_core_feature_flag_t flag);

/**
 * @brief Clear core feature request
 *
 * @param info [in] core info object
 * @param flag [in] feature flag
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_clear_core_feature(tinytc_core_info_t info,
                                                                  tinytc_core_feature_flag_t flag);

/**
 * @brief Delete core_info object
 *
 * @param info [in] core info object
 */
TINYTC_EXPORT void tinytc_core_info_destroy(tinytc_core_info_t info);

////////////////////////////
////////// Parser //////////
////////////////////////////

/**
 * @brief Parser tensor language source file and create prog
 *
 * @param prg [out] pointer to prog object created
 * @param filename [in] path to source file
 * @param ctx [inout][optional] source context object; stores error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_file(tinytc_prog_t *prg, char const *filename,
                                                tinytc_source_context_t ctx);

/**
 * @brief Parser tensor language source from stdin and create prog
 *
 * @param prg [out] pointer to prog object created
 * @param ctx [inout][optional] source context object; stores error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_stdin(tinytc_prog_t *prg, tinytc_source_context_t ctx);

/**
 * @brief Parser tensor language source from string and create prog
 *
 * @param prg [out] pointer to prog object created
 * @param source_size [in] length of source string
 * @param source [in] source string
 * @param ctx [inout][optional] source context object; stores error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_string(tinytc_prog_t *prg, size_t source_size,
                                                  char const *source, tinytc_source_context_t ctx);
/**
 * @brief Create source context
 *
 * The source context stores the tensor language source and enhaces error messages with
 * source code context.
 *
 * @param ctx [out] pointer to the source context object created
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_create(tinytc_source_context_t *ctx);

/**
 * @brief Add source context
 *
 * Manually add a source file to the source context that can be referenced in a tinytc_location.
 * Useful to enhance error messages when using the builder methods and classes.
 *
 * @param ctx [in] source context object
 * @param name [in] source name
 * @param text [in] source text
 * @param source_id [out] pointer to source id
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_add_source(tinytc_source_context_t ctx,
                                                               char const *name, char const *text,
                                                               int32_t *source_id);

/**
 * @brief Get error log
 *
 * The string's memory is owned by source context.
 * Note that the pointer may invalidated by any function call involving the source context object,
 * so the string should be copied or printed right after a call to this function.
 *
 * @param ctx [in] source context object
 * @param log [out] pointer to string
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_get_error_log(tinytc_source_context_t ctx,
                                                                  char const **log);

/**
 * @brief Report an error and augment the error with source context
 *
 * @param ctx [in] source context object
 * @param location [in] source location
 * @param what [in] error description
 * @param append [in] true: append to error log, false: clear error log
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_report_error(tinytc_source_context_t ctx,
                                                                 const tinytc_location_t *location,
                                                                 char const *what,
                                                                 tinytc_bool_t append);

/**
 * @brief Delete source context object
 *
 * @param ctx [in] source context object
 */
TINYTC_EXPORT void tinytc_source_context_destroy(tinytc_source_context_t ctx);

////////////////////////////
///////// Compiler /////////
////////////////////////////

/**
 * @brief Compile tensor language to OpenCL-C
 *
 * @param src [out] pointer to the source object created
 * @param prg [inout] tensor program; modified as compiler passes are run
 * @param info [in] core info object
 * @param ctx [inout][optional] source context object to save extended error messages that are
 * enhanced with source code context; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                                            const_tinytc_core_info_t info,
                                                            tinytc_source_context_t ctx);
/**
 * @brief Compile OpenCL-C source to device binary
 *
 * @param bin [out] pointer to the binary object created
 * @param src [in] source text
 * @param info [in] core info object
 * @param format [in] binary format (SPIR-V or native)
 * @param ctx [inout][optional] source context object to save extended error messages that are
 * enhanced with source code context; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_compile_to_binary(tinytc_binary_t *bin,
                                                              const_tinytc_source_t src,
                                                              const_tinytc_core_info_t info,
                                                              tinytc_bundle_format_t format,
                                                              tinytc_source_context_t ctx);

/**
 * @brief Compile tensor language to device binary
 *
 * @param bin [out] pointer to the binary object created
 * @param prg [inout] tensor program; modified as compiler passes are run
 * @param info [in] core info object
 * @param format [in] binary format (SPIR-V or native)
 * @param ctx [inout][optional] source context object to save extended error messages that are
 * enhanced with source code context; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_compile_to_binary(tinytc_binary_t *bin, tinytc_prog_t prg,
                                                            const_tinytc_core_info_t info,
                                                            tinytc_bundle_format_t format,
                                                            tinytc_source_context_t ctx);

/**
 * @brief Get source text
 *
 * @param src [in] source object
 * @param code [out] code contains a pointer to the source text; the pointer is only valid as long
 * as the source object is alive
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_get_code(const_tinytc_source_t src, char const **code);

/**
 * @brief Get raw binary data
 *
 * @param bin [in] binary object
 * @param format [out] binary format
 * @param data_size [out] size of data
 * @param data [out] data array; returned pointer is invalidated if the binary object is deleted
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_get_raw(const_tinytc_binary_t bin,
                                                    tinytc_bundle_format_t *format,
                                                    uint64_t *data_size, uint8_t const **data);
/**
 * @brief Get requested core features
 *
 * @param bin [in] binary object
 * @param core_features [out] core features
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_get_core_features(const_tinytc_binary_t bin,
                                                              uint32_t *core_features);

/**
 * @brief Delete source object
 *
 * @param src [in] source object
 */
TINYTC_EXPORT void tinytc_source_destroy(tinytc_source_t src);

/**
 * @brief Release binary object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param bin [inout] binary object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_release(tinytc_binary_t bin);

/**
 * @brief Increase reference count of binary object by 1
 *
 * @param bin [inout] binary object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_retain(tinytc_binary_t bin);

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Returns a small batched GEMM recipe
 *
 * The program contains a kernel for @f$\beta=0@f$ called "gemm_beta0" and a kernel for @f$\beta\neq
 * 0@f$ called "gemm". All matrix shapes and strides are known at compile-time.
 *
 * The signature of the generated kernels gemm and gemm_beta0 is (if A and B are not transposed)
 *
 * @code
 * func @{name}(%alpha: {ty.alpha},
 *              %A: memref<{ty.A}x{M}x{K}x?,strided<1,{ldA},{strideA}>>,
 *              %B: memref<{ty.B}x{K}x{N}x?,strided<1,{ldB},{strideB}>>,
 *              %beta: {ty.beta},
 *              %C: memref<{ty.C}x{M}x{N}x?,strided<1,{ldC},{strideC}>>)
 * @endcode
 *
 * meaning that its kernels need arguments in the following order:
 *
 * @code
 * alpha, A_ptr, howmany, B_ptr, howmany, beta, C_ptr, howmany
 * @endcode
 *
 * @param recipe [out] pointer to the recipe object created
 * @param info [in] core info object
 * @param ty [in] Scalar types of alpha, A, B, beta, C
 * @param tA [in] Transpose A
 * @param tB [in] Transpose B
 * @param M [in] Number of rows of A, C
 * @param N [in] Number of columns of B, C
 * @param K [in] Number columns of A, number of rows of B
 * @param ldA [in] Leading dimension of A
 * @param strideA [in] Number of elements between A-matrices
 * @param ldB [in] Leading dimension of B
 * @param strideB [in] Number of elements between B-matrices
 * @param ldC [in] Leading dimension of C
 * @param strideC [in] Number of elements between C-matrices
 * @param ctx [inout][optional] source context object; saves error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_small_gemm_batched_create(
    tinytc_recipe_t *recipe, tinytc_core_info_t info, tinytc_scalar_type_t ty,
    tinytc_transpose_t tA, tinytc_transpose_t tB, uint32_t M, uint32_t N, uint32_t K, uint32_t ldA,
    uint32_t strideA, uint32_t ldB, uint32_t strideB, uint32_t ldC, uint32_t strideC,
    tinytc_source_context_t ctx);

/**
 * @brief Set kernel arguments for small GEMM batched recipe
 *
 * @param handler [inout] Recipe handler object
 * @param howmany [in] Group size
 * @param alpha_size [in] Size of alpha argument
 * @param alpha_value [in] Pointer to data used for alpha; data is copied
 * @param A [in] Memory object used for A-matrix
 * @param B [in] Memory object used for B-matrix
 * @param beta_size [in] Size of beta argument
 * @param beta_value [in] Pointer to data used for beta; data is copied
 * @param C [in] Memory object used for C-matrix
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_small_gemm_batched_set_args(
    tinytc_recipe_handler_t handler, uint32_t howmany, size_t alpha_size, const void *alpha_value,
    tinytc_mem_t A, tinytc_mem_t B, size_t beta_size, const void *beta_value, tinytc_mem_t C);

/**
 * @brief Returns a tall and skinny recipe
 *
 * The program contains a kernel for beta = 0 called "gemm_beta0" and a kernel for beta != 0 called
 * "gemm". M (= number of rows of A, C) and strides are dynamic.
 *
 * The signature of the generated kernels gemm and gemm_beta0 is
 *
 * @code
 * func @{name}(%alpha: {ty.alpha},
 *              %A: memref<{ty.A}x?x{K},strided<1,?>>,
 *              %B: memref<{ty.B}x{K}x{N},strided<1,?>>,
 *              %beta: {ty.beta},
 *              %C: memref<{ty.C}x?x{N},strided<1,?>>)
 * @endcode
 *
 * meaning that its kernels need arguments in the following order:
 *
 * @code
 * alpha, A_ptr, M, ldA, B_ptr, ldB, beta, C_ptr, M, ldC
 * @endcode
 *
 * where ldA, ldB, ldC is the size of stride[1] of A, B, C, respectively.
 *
 * @param recipe [out] pointer to the recipe object created
 * @param info [in] core info object
 * @param ty [in] Scalar type of alpha, A, B, beta, C
 * @param [in] M_block_size Size of M block that each work group gets
 * @param [in] N Number of columns of B, C
 * @param [in] K Number columns of A, number of rows of B
 * @param ctx [inout][optional] source context object; saves error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create(
    tinytc_recipe_t *recipe, tinytc_core_info_t info, tinytc_scalar_type_t ty,
    uint32_t M_block_size, uint32_t N, uint32_t K, tinytc_source_context_t ctx);

/**
 * @brief Set kernel arguments for tall and skinny GEMM recipe
 *
 * @param handler [inout] Recipe handler object
 * @param M [in] Size of M-mode
 * @param alpha_size [in] Size of alpha argument
 * @param alpha_value [in] Pointer to data used for alpha; data is copied
 * @param A [in] Memory object used for A-matrix
 * @param ldA [in] Leading dimension of A
 * @param B [in] Memory object used for B-matrix
 * @param ldB [in] Leading dimension of B
 * @param beta_size [in] Size of beta argument
 * @param beta_value [in] Pointer to data used for beta; data is copied
 * @param C [in] Memory object used for C-matrix
 * @param ldC [in] Leading dimension of C
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_set_args(
    tinytc_recipe_handler_t handler, uint32_t M, size_t alpha_size, const void *alpha_value,
    tinytc_mem_t A, uint32_t ldA, tinytc_mem_t B, uint32_t ldB, size_t beta_size,
    const void *beta_value, tinytc_mem_t C, uint32_t ldC);

/**
 * @brief Get prog object
 *
 * @param recipe [in] recipe object
 * @param prg [out] pointer to prog object; reference count is increased so the user needs to call
 * tinytc_prog_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_get_prog(const_tinytc_recipe_t recipe,
                                                     tinytc_prog_t *prg);

/**
 * @brief Get binary object
 *
 * @param recipe [in] recipe object
 * @param bin [out] pointer to binary object; reference count is increased so the user needs to call
 * tinytc_binary_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_get_binary(const_tinytc_recipe_t recipe,
                                                       tinytc_binary_t *bin);

/**
 * @brief Release recipe object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] recipe object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_release(tinytc_recipe_t obj);

/**
 * @brief Increase reference count of recipe object by 1
 *
 * @param obj [inout] recipe object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_retain(tinytc_recipe_t obj);

/**
 * @brief Release recipe handler object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] recipe handler object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_handler_release(tinytc_recipe_handler_t obj);

/**
 * @brief Increase reference count of recipe handler object by 1
 *
 * @param obj [inout] recipe handler object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_handler_retain(tinytc_recipe_handler_t obj);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_20240409_H
