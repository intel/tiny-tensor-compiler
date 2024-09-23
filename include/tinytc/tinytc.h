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
 * @param addrspace [in][optional] Address space; default is tinytc_address_space_global
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_memref_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t scalar_ty,
                                                        uint32_t shape_size, const int64_t *shape,
                                                        uint32_t stride_size, const int64_t *stride,
                                                        const tinytc_address_space_t addrspace,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create group data type
 *
 * @param dt [out] pointer to the data type object created
 * @param memref_ty [in] memref data type object
 * @param offset [in][optional] offset parameter; pass 0 for default
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty, int64_t offset,
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

//! Convert address space to string
TINYTC_EXPORT char const *tinytc_address_space_to_string(tinytc_address_space_t as);
//! Convert arithmetic operation type to string
TINYTC_EXPORT char const *tinytc_arithmetic_to_string(tinytc_arithmetic_t op);
//! Convert arithmetic operation type to string (unary)
TINYTC_EXPORT char const *tinytc_arithmetic_unary_to_string(tinytc_arithmetic_unary_t op);
//! Convert cmp condition to string
TINYTC_EXPORT char const *tinytc_cmp_condition_to_string(tinytc_cmp_condition_t cond);
//! Convert transpose to string
TINYTC_EXPORT char const *tinytc_transpose_to_string(tinytc_transpose_t t);

/**
 * @brief Create arithmetic instruction (binary)
 *
 * @code %value = arith.<op> %a, %b : type(%a) ; type(%a) == type(%b) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param op [in] arithmetic operation type
 * @param a [in] left-hand operand
 * @param b [in] right-hand operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_arith_inst_create(tinytc_inst_t *instr, tinytc_arithmetic_t op,
                                                       tinytc_value_t a, tinytc_value_t b,
                                                       const tinytc_location_t *loc);

/**
 * @brief Create arithmetic instruction (unary)
 *
 * @code %value = arith.<op> %a : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param op [in] unary arithmetic operation type
 * @param a [in] operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_arith_unary_inst_create(tinytc_inst_t *instr,
                                                             tinytc_arithmetic_unary_t op,
                                                             tinytc_value_t a,
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param beta [in] @f$\beta@f$
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @f$\beta@f$
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @f$\beta@f$
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @f$\beta@f$
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param B [in] B
 * @param beta [in] @f$\beta@f$
 * @param C [in] C
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_hadamard_inst_create(
    tinytc_inst_t *instr, tinytc_bool_t atomic, tinytc_value_t alpha, tinytc_value_t A,
    tinytc_value_t B, tinytc_value_t beta, tinytc_value_t C, const tinytc_location_t *loc);

/**
 * @brief Create num_subgroups instruction
 *
 * @code %value = num_subgroups @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_num_subgroups_inst_create(tinytc_inst_t *instr,
                                                               const tinytc_location_t *loc);

/**
 * @brief Create parallel region
 *
 * Takes ownership of region.
 *
 * @code
 * parallel { %body }
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param body [in,pass_ownership] loop body
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parallel_inst_create(tinytc_inst_t *instr,
                                                          tinytc_region_t body,
                                                          const tinytc_location_t *loc);

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
 * @brief Create subgroup_id instruction
 *
 * @code %value = subgroup_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_id_inst_create(tinytc_inst_t *instr,
                                                             const tinytc_location_t *loc);

/**
 * @brief Create subgroup_local_id instruction
 *
 * @code %value = subgroup_local_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_local_id_inst_create(tinytc_inst_t *instr,
                                                                   const tinytc_location_t *loc);

/**
 * @brief Create subgroup_size instruction
 *
 * @code %value = subgroup_size @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_size_inst_create(tinytc_inst_t *instr,
                                                               const tinytc_location_t *loc);

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
 * @param loc [in][optional] Source code location; can be nullptr
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
 * @param alpha [in] @f$\alpha@f$
 * @param A [in] A
 * @param beta [in] @f$\beta@f$
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
 * Takes ownership of region.
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
 * @param body [in,pass_ownership] loop body
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
 * Takes ownership of region.
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
 * @param body [in,pass_ownership] loop body
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
 * Takes ownership of if and else region (if given).
 *
 * @code
 * if %condition { %then } else { %otherwise }
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param condition [in] condition
 * @param then [in,pass_ownership] region taken if condition is true
 * @param otherwise [in,pass_ownership][optional] region taken if condition is false; can be nullptr
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
 * @param yield_list_size [in] length of yielded values list
 * @param yield_list [in][range(0, yield_list_size)] yielded values array; can be nullptr if
 * yield_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_yield_inst_create(tinytc_inst_t *instr,
                                                       uint32_t yield_list_size,
                                                       tinytc_value_t *yield_list,
                                                       const tinytc_location_t *loc);

/**
 * @brief Delete inst object
 *
 * @param instr [inout] inst object
 */
TINYTC_EXPORT void tinytc_inst_destroy(tinytc_inst_t instr);

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
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_create(tinytc_region_t *reg,
                                                   const tinytc_location_t *loc);

/**
 * @brief Append instruction to region
 *
 * The region takes ownership of the instruction.
 * An instruction must not be added to multiple regions.
 *
 * @param reg [inout] region object
 * @param instruction [in,pass_ownership] instruction
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_add_instruction(tinytc_region_t reg,
                                                            tinytc_inst_t instruction);

/**
 * @brief Delete region object
 *
 * @param reg [inout] region object
 */
TINYTC_EXPORT void tinytc_region_destroy(tinytc_region_t reg);

////////////////////////////
/////////// Func ///////////
////////////////////////////

/**
 * @brief Create function
 *
 * Function takes ownership of region.
 *
 * @param fun [out] pointer to the func object created
 * @param name [in] function name
 * @param arg_list_size [in] length of argument array
 * @param arg_list [in][range(0,arg_list_size)] argument array; can be nullptr if arg_list_size is 0
 * @param body [in,pass_ownership] function body
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_create(tinytc_func_t *fun, char const *name,
                                                     uint32_t arg_list_size,
                                                     tinytc_value_t *arg_list, tinytc_region_t body,
                                                     const tinytc_location_t *loc);
/**
 * @brief Set work-group size
 *
 * @param fun [out] func object (must be the function definition, not the function prototype)
 * @param x [in] number of rows in parallel grid; must be a multiple of the subgroup size
 * @param y [in] number of columns in parallel grid
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_set_work_group_size(tinytc_func_t fun, int32_t x,
                                                                  int32_t y);
/**
 * @brief Set subgroup size
 *
 * @param fun [out] func object (must be the function definition, not the function prototype)
 * @param sgs [in] subgroup size; the supported values need to be queried from the compute device
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_function_set_subgroup_size(tinytc_func_t fun, int32_t sgs);

/**
 * @brief Delete function object
 *
 * @param fun [inout] function object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT void tinytc_func_destroy(tinytc_func_t fun);

////////////////////////////
/////////// Prog ///////////
////////////////////////////

/**
 * @brief Create program
 *
 * @param prg [out] pointer to the prog object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_program_create(tinytc_prog_t *prg,
                                                    const tinytc_location_t *loc);

/**
 * @brief Append function to program
 *
 * The program takes ownership of the function.
 * A function must not be added to multiple programs.
 *
 * @param prg [inout] program object
 * @param fun [in,pass_ownership] function object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_add_function(tinytc_prog_t prg, tinytc_func_t fun);

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
 * @brief Delete a (non-const) string returned from tinytc API
 *
 * @param str [in] string
 */
TINYTC_EXPORT void tinytc_string_destroy(char *str);

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Create core_info for a generic GPUs
 *
 * @param info [out] pointer to the core_info object created
 * @param register_space [in] Size of register file per subgroup in bytes
 * @param max_work_group_size [in] Maximum size of local work group
 * @param sgs_size [in] Length of sgs array
 * @param sgs [in] Allowed subgroup sizes
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_generic_create(tinytc_core_info_t *info,
                                                              int32_t register_space,
                                                              int32_t max_work_group_size,
                                                              uint32_t sgs_size,
                                                              int32_t const *sgs);

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
 * @param sgs_size [in] Length of sgs array
 * @param sgs [in] Allowed subgroup sizes
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_intel_create(tinytc_core_info_t *info,
                                                            uint32_t ip_version,
                                                            int32_t num_eus_per_subslice,
                                                            int32_t num_threads_per_eu,
                                                            uint32_t sgs_size, int32_t const *sgs);

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
                                                                  int32_t const **sgs);

/**
 * @brief Returns register space per subgroup in bytes
 *
 * @param info [in] core info object
 * @param space [out] pointer to register space
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_register_space(const_tinytc_core_info_t info,
                                                                  int32_t *space);

/**
 * @brief Set core features
 *
 * @param info [in] core info object
 * @param flags [in] set core features; must be 0 or a combination of tinytc_core_feature_flag_t
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_set_core_features(tinytc_core_info_t info,
                                                                 tinytc_core_feature_flags_t flags);

/**
 * @brief Get core features
 *
 * @param info [in] core info object
 * @param flags [out] pointer to core feature flags
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t
tinytc_core_info_get_core_features(tinytc_core_info_t info, tinytc_core_feature_flags_t *flags);

/**
 * @brief Release core info object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] core info object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_release(tinytc_core_info_t obj);

/**
 * @brief Increase reference count of core info object by 1
 *
 * @param obj [inout] core info object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_retain(tinytc_core_info_t obj);

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
TINYTC_EXPORT tinytc_status_t tinytc_source_context_get_error_log(const_tinytc_source_context_t ctx,
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
 * @brief Release source context object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] source context object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_release(tinytc_source_context_t obj);

/**
 * @brief Increase reference count of source context object by 1
 *
 * @param obj [inout] source context object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_context_retain(tinytc_source_context_t obj);

////////////////////////////
///////// Compiler /////////
////////////////////////////

/**
 * @brief Run a function pass on every function of a program
 *
 * @param pass_name [in] name of function pass; cf. tinytc_list_function_passes
 * @param prg [inout] tensor program; modified as compiler pass is run
 * @param info [in][optional] core info object; might be nullptr if core info is not required for
 * pass
 * @param ctx [inout][optional] source context object to save extended error messages that are
 * enhanced with source code context; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_run_function_pass(char const *pass_name, tinytc_prog_t prg,
                                                       const_tinytc_core_info_t info,
                                                       tinytc_source_context_t ctx);

/**
 * @brief List function passes
 *
 * @param names_size [out] pointer to number of function pass names
 * @param names [out][range(0,names_size)] pointer to array of C-strings; array owned by tinytc
 *
 * @return
 */
TINYTC_EXPORT tinytc_status_t tinytc_list_function_passes(uint32_t *names_size,
                                                          char const *const **names);

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
 * @brief Get source text
 *
 * @param src [in] source object
 * @param length [out] pointer to code length
 * @param code [out] code contains a pointer to the source text; the pointer is only valid as long
 * as the source object is alive
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_get_code(const_tinytc_source_t src, size_t *length,
                                                     char const **code);

/**
 * @brief Get source location
 *
 * @param src [in] source object
 * @param loc [out] pointer to location
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_get_location(const_tinytc_source_t src,
                                                         tinytc_location_t *loc);

/**
 * @brief Get core features
 *
 * @param src [in] source object
 * @param core_features [out] pointer to core features
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_get_core_features(
    const_tinytc_source_t src, tinytc_core_feature_flags_t *core_features);

/**
 * @brief Get required OpenCL extensions
 *
 * @param src [in] source object
 * @param extensions_size [out] pointer to number of extensions
 * @param extensions [out][range(0,extensions_size)] pointer to array of C-strings; array owned by
 * source object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_get_extensions(const_tinytc_source_t src,
                                                           uint32_t *extensions_size,
                                                           char const *const **extensions);

/**
 * @brief Create binary
 *
 * @param bin [out] pointer to binary object
 * @param format [in] Bundle format (SPIR-V or Native)
 * @param data_size [in] Size of data in bytes
 * @param data [in][range(0, data_size)] Binary data; data is copied
 * @param core_features [in][optional] requested core features; must be 0 (default) or a combination
 * of tinytc_core_feature_flag_t
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_create(tinytc_binary_t *bin,
                                                   tinytc_bundle_format_t format, size_t data_size,
                                                   uint8_t const *data,
                                                   tinytc_core_feature_flags_t core_features);

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
                                                    size_t *data_size, uint8_t const **data);
/**
 * @brief Get requested core features
 *
 * @param bin [in] binary object
 * @param core_features [out] core features
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_get_core_features(
    const_tinytc_binary_t bin, tinytc_core_feature_flags_t *core_features);

/**
 * @brief Release source object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] source object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_release(tinytc_source_t obj);

/**
 * @brief Increase reference count of source object by 1
 *
 * @param obj [inout] source object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_source_retain(tinytc_source_t obj);

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
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty,
    tinytc_transpose_t tA, tinytc_transpose_t tB, int64_t M, int64_t N, int64_t K, int64_t ldA,
    int64_t strideA, int64_t ldB, int64_t strideB, int64_t ldC, int64_t strideC,
    tinytc_source_context_t ctx);

/**
 * @brief Set kernel arguments for small GEMM batched recipe
 *
 * @param handler [inout] Recipe handler object
 * @param howmany [in] Group size
 * @param alpha_size [in] Size of alpha argument
 * @param alpha_value [in] Pointer to data used for alpha; data is copied
 * @param A_type [in] Type of memory object used for A-matrix
 * @param A_value [in] Memory object used for A-matrix
 * @param B_type [in] Type of memory object used for B-matrix
 * @param B_value [in] Memory object used for B-matrix
 * @param beta_size [in] Size of beta argument
 * @param beta_value [in] Pointer to data used for beta; data is copied
 * @param C_type [in] Type of memory object used for C-matrix
 * @param C_value [in] Memory object used for C-matrix
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_small_gemm_batched_set_args(
    tinytc_recipe_handler_t handler, int64_t howmany, size_t alpha_size, const void *alpha_value,
    tinytc_mem_type_t A_type, const void *A_value, tinytc_mem_type_t B_type, const void *B_value,
    size_t beta_size, const void *beta_value, tinytc_mem_type_t C_type, const void *C_value);

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
 * @param N [in] Number of columns of B, C
 * @param K [in] Number columns of A, number of rows of B
 * @param M_block_size [in][optional] Size of M block that each work group gets; pass 0 to have the
 * parameter auto-selected
 * @param ctx [inout][optional] source context object; saves error log; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t N,
    int64_t K, int32_t M_block_size, tinytc_source_context_t ctx);

/**
 * @brief Returns a tall and skinny recipe with additional specialization constants
 *
 * Similar to tinytc_recipe_tall_and_skinny_create but with the additional specialization
 * constants M, ldA, ldB, and ldC.
 * The specializtion constants may be either set to a fixed value or to TINYTC_DYNAMIC.
 * Note that if a specialization constant is set to a fixed value then the parameter with the same
 * name in tinytc_recipe_tall_and_skinny_set_args is ignored.
 *
 * The generated kernels have the following signature:
 *
 * @code
 * func @{name}(%alpha: {ty.alpha},
 *              %A: memref<{ty.A}x{M}x{K},strided<1,{ldA}>>,
 *              %B: memref<{ty.B}x{K}x{N},strided<1,{ldB}>>,
 *              %beta: {ty.beta},
 *              %C: memref<{ty.C}x{M}x{N},strided<1,{ldC}>>)
 * @endcode
 *
 * @param recipe [out] pointer to the recipe object created
 * @param info [in] core info object
 * @param ty [in] Scalar type of alpha, A, B, beta, C
 * @param M [in] Number of rows of A, C; can be TINYTC_DYNAMIC
 * @param N [in] Number of columns of B, C
 * @param K [in] Number columns of A, number of rows of B
 * @param ldA [in] Leading dimension of A; can be TINYTC_DYNAMIC
 * @param ldB [in] Leading dimension of B; can be TINYTC_DYNAMIC
 * @param ldC [in] Leading dimension of C; can be TINYTC_DYNAMIC
 * @param M_block_size [in][optional] Size of M block that each work group gets; pass 0 to have the
 * parameter auto-selected
 * @param ctx [inout][optional] source context object; saves error log; can be nullptr
 *
 * @return
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create_specialized(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t M,
    int64_t N, int64_t K, int64_t ldA, int64_t ldB, int64_t ldC, int32_t M_block_size,
    tinytc_source_context_t ctx);

/**
 * @brief Suggest an M block size for tall and skinny recipe
 *
 * @param info [in] core info object
 * @param M_block_size [out] pointer to block size
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_suggest_block_size(
    const_tinytc_core_info_t info, int32_t *M_block_size);

/**
 * @brief Set kernel arguments for tall and skinny GEMM recipe
 *
 * @param handler [inout] Recipe handler object
 * @param M [in] Size of M-mode
 * @param alpha_size [in] Size of alpha argument
 * @param alpha_value [in] Pointer to data used for alpha; data is copied
 * @param A_type [in] Type of memory object used for A-matrix
 * @param A_value [in] Memory object used for A-matrix
 * @param ldA [in] Leading dimension of A
 * @param B_type [in] Type of memory object used for B-matrix
 * @param B_value [in] Memory object used for B-matrix
 * @param ldB [in] Leading dimension of B
 * @param beta_size [in] Size of beta argument
 * @param beta_value [in] Pointer to data used for beta; data is copied
 * @param C_type [in] Type of memory object used for C-matrix
 * @param C_value [in] Memory object used for C-matrix
 * @param ldC [in] Leading dimension of C
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_set_args(
    tinytc_recipe_handler_t handler, int64_t M, size_t alpha_size, const void *alpha_value,
    tinytc_mem_type_t A_type, const void *A_value, int64_t ldA, tinytc_mem_type_t B_type,
    const void *B_value, int64_t ldB, size_t beta_size, const void *beta_value,
    tinytc_mem_type_t C_type, const void *C_value, int64_t ldC);

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
 * @brief Get source object
 *
 * @param recipe [in] recipe object
 * @param src [out] pointer to source object; reference count is increased so the user needs to call
 * tinytc_source_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_get_source(const_tinytc_recipe_t recipe,
                                                       tinytc_source_t *src);

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
 * @brief Get recipe object
 *
 * @param handler [in] recipe handler object
 * @param recipe [out] pointer to recipe object; reference count is increased so the user needs to
 * call tinytc_recipe_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t
tinytc_recipe_handler_get_recipe(const_tinytc_recipe_handler_t handler, tinytc_recipe_t *recipe);

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
