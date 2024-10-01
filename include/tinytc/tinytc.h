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
 * @brief Get scalar data type
 *
 * @param dt [out] pointer to the data type object created
 * @param ctx [inout] compiler context
 * @param type [in] scalar type
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_scalar_type_get(tinytc_data_type_t *dt,
                                                     tinytc_compiler_context_t ctx,
                                                     tinytc_scalar_type_t type);

/**
 * @brief Get memref data type
 *
 * Note: modifies compiler context
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
TINYTC_EXPORT tinytc_status_t tinytc_memref_type_get(tinytc_data_type_t *dt,
                                                     tinytc_data_type_t scalar_ty,
                                                     uint32_t shape_size, const int64_t *shape,
                                                     uint32_t stride_size, const int64_t *stride,
                                                     tinytc_address_space_t addrspace,
                                                     const tinytc_location_t *loc);

/**
 * @brief Get group data type
 *
 * Note: modifies compiler context
 *
 * @param dt [out] pointer to the data type object created
 * @param memref_ty [in] memref data type object
 * @param offset [in][optional] offset parameter; pass 0 for default
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_get(tinytc_data_type_t *dt,
                                                    tinytc_data_type_t memref_ty, int64_t offset,
                                                    const tinytc_location_t *loc);

////////////////////////////
/////////// Value //////////
////////////////////////////

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
 * @brief Set name of value with explicit number of characters
 *
 * @param vl [inout] value object
 * @param name_length [in] number of characters
 * @param name [in] name; not necessarily null-terminated
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_set_name_n(tinytc_value_t vl, uint32_t name_length,
                                                      char const *name);

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
                                                      tinytc_data_type_t to_ty,
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
 * @brief Create complex constant instruction
 *
 * @param instr [out] pointer to the inst object created
 * @param value_re [in] constant value (real part)
 * @param value_im [in] constant value (imaginary part)
 * @param ty [in] type of constant
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_constant_inst_create_complex(tinytc_inst_t *instr,
                                                                  double value_re, double value_im,
                                                                  tinytc_data_type_t ty,
                                                                  const tinytc_location_t *loc);

/**
 * @brief Create floating constant instruction
 *
 * @param instr [out] pointer to the inst object created
 * @param value [in] constant value
 * @param ty [in] type of constant
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_constant_inst_create_float(tinytc_inst_t *instr, double value,
                                                                tinytc_data_type_t ty,
                                                                const tinytc_location_t *loc);

/**
 * @brief Create integer constant instruction
 *
 * @param instr [out] pointer to the inst object created
 * @param value [in] constant value
 * @param ty [in] type of constant
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_constant_inst_create_int(tinytc_inst_t *instr, int64_t value,
                                                              tinytc_data_type_t ty,
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
 * @param expanded_mode [in] expanded mode
 * @param static_expand_shape_size [in] dimension of static expand shape; must be at least 2
 * @param static_expand_shape [in][range(2, static expand_shape_size)] static expand shape array
 * @param expand_shape_size [in][optional] dimension of expand shape; must match number of entries
 * equal to TINYTC_DYNAMIC in static_expand_shape array; can be 0
 * @param expand_shape [in][optional][range(0, expand_shape_size)] expand shape array
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_expand_inst_create(
    tinytc_inst_t *instr, tinytc_value_t a, int64_t expanded_mode,
    uint32_t static_expand_shape_size, const int64_t *static_expand_shape,
    uint32_t expand_shape_size, const tinytc_value_t *expand_shape, const tinytc_location_t *loc);

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
                                                      const tinytc_value_t *index_list,
                                                      const tinytc_location_t *loc);
/**
 * @brief Create group_id instruction
 *
 * @code %value = group_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr,
                                                          tinytc_compiler_context_t ctx,
                                                          const tinytc_location_t *loc);

/**
 * @brief Create group_size instruction
 *
 * @code %value = group_size @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr,
                                                            tinytc_compiler_context_t ctx,
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
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_num_subgroups_inst_create(tinytc_inst_t *instr,
                                                               tinytc_compiler_context_t ctx,
                                                               const tinytc_location_t *loc);

/**
 * @brief Create parallel region
 *
 * @code
 * parallel { }
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parallel_inst_create(tinytc_inst_t *instr,
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
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_id_inst_create(tinytc_inst_t *instr,
                                                             tinytc_compiler_context_t ctx,
                                                             const tinytc_location_t *loc);

/**
 * @brief Create subgroup_local_id instruction
 *
 * @code %value = subgroup_local_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_local_id_inst_create(tinytc_inst_t *instr,
                                                                   tinytc_compiler_context_t ctx,
                                                                   const tinytc_location_t *loc);

/**
 * @brief Create subgroup_size instruction
 *
 * @code %value = subgroup_size @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param ctx [in] compiler context
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subgroup_size_inst_create(tinytc_inst_t *instr,
                                                               tinytc_compiler_context_t ctx,
                                                               const tinytc_location_t *loc);

/**
 * @brief Create subview instruction
 *
 * @code %value = subview %a[%offset1:%size1,...,%offsetN:%sizeN] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param a [in] operand
 * @param static_list_size [in] number of slices
 * @param static_offset_list [in][range(0, static_list_size)] offsets (need to add value to
 * offset_list if static_offset_list[i] == TINYTC_DYNAMIC); may be nullptr if static_offset_list = 0
 * @param static_size_list [in][range(0, static_list_size)] sizes (need to add value to size_list
 * if static_size_list[i] == TINYTC_DYNAMIC); may be nullptr if static_offset_list = 0
 * @param offset_list_size [in] number of dynamic offsets
 * @param offset_list [in][range(0, offset_list_size)] offset array; may be nullptr if
 * offset_list_size is 0
 * @param size_list_size [in] number of dynamic sizes
 * @param size_list [in][range(0, size_list_size)] size array; may be nullptr if size_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subview_inst_create(
    tinytc_inst_t *instr, tinytc_value_t a, uint32_t static_list_size,
    const int64_t *static_offset_list, const int64_t *static_size_list, uint32_t offset_list_size,
    const tinytc_value_t *offset_list, uint32_t size_list_size, const tinytc_value_t *size_list,
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
                                                       const tinytc_value_t *index_list,
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
 * @code
 * for %loop_var = %from, %to, %step : loop_var_type { }
 * ; loop_var_type == type(%from)
 * ; loop_var_type == type(%to)
 * ; loop_var_type == type(%step)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param from [in] loop begion
 * @param to [in] loop bound
 * @param step [in][optional] loop step; can be nullptr
 * @param loop_var_type [in] type of loop variable
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_for_inst_create(tinytc_inst_t *instr, tinytc_value_t from,
                                                     tinytc_value_t to, tinytc_value_t step,
                                                     tinytc_data_type_t loop_var_type,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create foreach loop
 *
 * @code
 * foreach %loop_var = %from, %to : loop_var_type { }
 * ; loop_var_type == type(%from)
 * ; loop_var_type == type(%to)
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param from [in] loop begion
 * @param to [in] loop bound
 * @param loop_var_type [in] type of loop variable
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_foreach_inst_create(tinytc_inst_t *instr, tinytc_value_t from,
                                                         tinytc_value_t to,
                                                         tinytc_data_type_t loop_var_type,
                                                         const tinytc_location_t *loc);

/**
 * @brief Create if condition
 *
 * @code
 * if %condition -> (return_type_list, ...) { } else { }
 * @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param condition [in] condition
 * @param return_type_list_size [in] length of return type array
 * @param return_type_list [in][range(0, return_type_list_size)] return type array; can be nullptr
 * if return_type_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_if_inst_create(tinytc_inst_t *instr, tinytc_value_t condition,
                                                    uint32_t return_type_list_size,
                                                    const tinytc_data_type_t *return_type_list,
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
                                                       const tinytc_value_t *yield_list,
                                                       const tinytc_location_t *loc);

/**
 * @brief Delete inst object
 *
 * @param instr [inout] inst object
 */
TINYTC_EXPORT void tinytc_inst_destroy(tinytc_inst_t instr);

/**
 * @brief Get values produced by instruction
 *
 * Function can be called with result_list_size = 0 and result_list = nullptr in order to obtain
 * the number of results
 *
 * @param instr [in] inst object
 * @param result_list_size [inout] pointer to the number of results; if result_list_size is 0, then
 * it is updated with the number of results; if result_list_size is greater than the number of
 * results, the value is updated with the correct number of results
 * @param result_list [out][range(0, result_list_size)] user-provided memory for storing result
 * handles; at most result_list_size values are written; can be nullptr if result_list_size is 0
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_get_values(tinytc_inst_t instr,
                                                     uint32_t *result_list_size,
                                                     tinytc_value_t *result_list);

/**
 * @brief Get child regions of instruction
 *
 * Function can be called with result_list_size = 0 and result_list = nullptr in order to obtain
 * the number of results
 *
 * @param instr [in] inst object
 * @param result_list_size [inout] pointer to the number of results; if result_list_size is 0, then
 * it is updated with the number of results; if result_list_size is greater than the number of
 * results, the value is updated with the correct number of results
 * @param result_list [out][range(0, result_list_size)] user-provided memory for storing result
 * handles; at most result_list_size values are written; can be nullptr if result_list_size is 0
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_get_regions(tinytc_inst_t instr,
                                                      uint32_t *result_list_size,
                                                      tinytc_region_t *result_list);

////////////////////////////
////////// Region //////////
////////////////////////////

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
 * @brief Get region parameters
 *
 * Function can be called with result_list_size = 0 and result_list = nullptr in order to obtain
 * the number of results
 *
 * @param reg [in] region object
 * @param result_list_size [inout] pointer to the number of results; if result_list_size is 0, then
 * it is updated with the number of results; if result_list_size is greather than the number of
 * results, the value is updated with the correct number of results
 * @param result_list [out][range(0, result_list_size)] user-provided memory for storing result
 * handles; at most result_list_size values are written; can be nullptr if result_list_size is 0
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_region_get_parameters(tinytc_region_t reg,
                                                           uint32_t *result_list_size,
                                                           tinytc_value_t *result_list);

////////////////////////////
/////////// Func ///////////
////////////////////////////

/**
 * @brief Create function
 *
 * Function takes ownership of region.
 *
 * @param fun [out] pointer to the func object created
 * @param name_length [in] length of function_name
 * @param name [in] function name
 * @param num_params [in] number of parameters
 * @param param_type_list [in][range(0,num_params)] parameter data types; can be nullptr if
 * num_params is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_create(tinytc_func_t *fun, uint32_t name_length,
                                                 char const *name, uint32_t num_params,
                                                 const tinytc_data_type_t *param_type_list,
                                                 const tinytc_location_t *loc);

/**
 * @brief Set work-group size
 *
 * @param fun [inout] function object
 * @param x [in] number of rows in parallel grid; must be a multiple of the subgroup size
 * @param y [in] number of columns in parallel grid
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_set_work_group_size(tinytc_func_t fun, int32_t x,
                                                              int32_t y);
/**
 * @brief Set subgroup size
 *
 * @param fun [inout] function object
 * @param sgs [in] subgroup size; the supported values need to be queried from the compute device
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_set_subgroup_size(tinytc_func_t fun, int32_t sgs);

/**
 * @brief Get function body
 *
 * @param fun [in] function object
 * @param body [out] pointer to body region
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_func_get_body(tinytc_func_t fun, tinytc_region_t *body);

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
 * @param ctx [in] compiler context object
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_create(tinytc_prog_t *prg, tinytc_compiler_context_t ctx,
                                                 const tinytc_location_t *loc);

/**
 * @brief Append function to program
 *
 * The program takes ownership of the function.
 * A function must not be added to multiple programs nor must the user destroy the function after
 * adding it to the program.
 *
 * @param prg [inout] program object
 * @param fun [in,pass_ownership] function object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_add_function(tinytc_prog_t prg, tinytc_func_t fun);

/**
 * @brief Get context object from program object
 *
 * @param prg [in] program object
 * @param ctx [out] pointer to context object; reference count is increased so the user needs to
 * call tinytc_compiler_context_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_get_compiler_context(const_tinytc_prog_t prg,
                                                               tinytc_compiler_context_t *ctx);

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
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_file(tinytc_prog_t *prg, char const *filename,
                                                tinytc_compiler_context_t ctx);

/**
 * @brief Parser tensor language source from stdin and create prog
 *
 * @param prg [out] pointer to prog object created
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_stdin(tinytc_prog_t *prg, tinytc_compiler_context_t ctx);

/**
 * @brief Parser tensor language source from string and create prog
 *
 * @param prg [out] pointer to prog object created
 * @param source_size [in] length of source string
 * @param source [in] source string
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_parse_string(tinytc_prog_t *prg, size_t source_size,
                                                  char const *source,
                                                  tinytc_compiler_context_t ctx);
/**
 * @brief Create context
 *
 * The context stores the tensor language source and reports enhaces error messages with
 * source code context. Moreover, the context caches data such as types and constants.
 *
 * @param ctx [out] pointer to the context object created
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_create(tinytc_compiler_context_t *ctx);

/**
 * @brief Add source context
 *
 * Manually add a source file to the context that can be referenced in a tinytc_location.
 * Useful to enhance error messages when using the builder methods and classes.
 *
 * @param ctx [in] context object
 * @param name [in] source name
 * @param text [in] source text
 * @param source_id [out] pointer to source id
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_add_source(tinytc_compiler_context_t ctx,
                                                                 char const *name, char const *text,
                                                                 int32_t *source_id);

/**
 * @brief Set error reporter
 *
 * Error reporting function that is called whenever an error occurs in the parser or the builder.
 *
 * @param ctx [in] context object
 * @param reporter [in] error reporting callback; set to nullptr to disable reporting
 * @param user_data [in][optional] pointer to user data that is passed to the callback; can be
 * nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_set_error_reporter(
    tinytc_compiler_context_t ctx, tinytc_error_reporter_t reporter, void *user_data);

/**
 * @brief Report an error and augment the error with source context
 *
 * @param ctx [in] context object
 * @param location [in] source location
 * @param what [in] error description
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_report_error(
    tinytc_compiler_context_t ctx, const tinytc_location_t *location, char const *what);

/**
 * @brief Release context object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param obj [inout] context object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_release(tinytc_compiler_context_t obj);

/**
 * @brief Increase reference count of context object by 1
 *
 * @param obj [inout] context object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_retain(tinytc_compiler_context_t obj);

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
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_run_function_pass(char const *pass_name, tinytc_prog_t prg,
                                                       const_tinytc_core_info_t info);

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
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                                            const_tinytc_core_info_t info);

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
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_small_gemm_batched_create(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty,
    tinytc_transpose_t tA, tinytc_transpose_t tB, int64_t M, int64_t N, int64_t K, int64_t ldA,
    int64_t strideA, int64_t ldB, int64_t strideB, int64_t ldC, int64_t strideC,
    tinytc_compiler_context_t ctx);

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
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t N,
    int64_t K, int32_t M_block_size, tinytc_compiler_context_t ctx);

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
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create_specialized(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t M,
    int64_t N, int64_t K, int64_t ldA, int64_t ldB, int64_t ldC, int32_t M_block_size,
    tinytc_compiler_context_t ctx);

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
