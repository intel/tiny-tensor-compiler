// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240409_H
#define TINYTC_20240409_H

#include "tinytc/export.h"
#include "tinytc/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_scalar_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t type);

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
 * @return tinytc_success on success and error otherwise
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
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty);

/**
 * @brief Release data type object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt);

/**
 * @brief Increase reference count of data type object by 1
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_success on success and error otherwise
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
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_create(tinytc_value_t *vl, tinytc_data_type_t type);

/**
 * @brief Create floating point immediate value
 *
 * @param vl [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_float_imm_create(tinytc_value_t *vl, double imm,
                                                      tinytc_scalar_type_t type);
/**
 * @brief Create integer immediate value
 *
 * @param vl [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_int_imm_create(tinytc_value_t *vl, int64_t imm,
                                                    tinytc_scalar_type_t type);

/**
 * @brief Release value object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param vl [inout] value object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_release(tinytc_value_t vl);

/**
 * @brief Increase reference count of value object by 1
 *
 * @param vl [inout] value object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_retain(tinytc_value_t vl);

/**
 * @brief Set name of value
 *
 * @param vl [inout] value object
 * @param name [in] name; null-terminated string
 *
 * @return tinytc_success on success and error otherwise
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
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_get_name(tinytc_value_t vl, char const **name);

////////////////////////////
//////// Instruction ///////
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
 * @param result [out] pointer to the resulting value object created
 * @param op [in] binary op type
 * @param a [in] left-hand operand
 * @param b [in] right-hand operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_op_inst_create(tinytc_inst_t *instr,
                                                           tinytc_value_t *result,
                                                           tinytc_binary_op_t op, tinytc_value_t a,
                                                           tinytc_value_t b,
                                                           const tinytc_location_t *loc);

/**
 * @brief Create cast instruction
 *
 * @code %value = cast %a, %b : type(%a) -> %to_ty @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param to_ty [in] target type
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cast_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                      tinytc_value_t a, tinytc_scalar_type_t to_ty,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create binary op instruction
 *
 * @code %value = cmp.<cond> %a, %b : type(%a) ; type(%a) == type(%b) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param cond [in] compare type
 * @param a [in] left-hand operand
 * @param b [in] right-hand operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cmp_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                     tinytc_cmp_condition_t cond, tinytc_value_t a,
                                                     tinytc_value_t b,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create neg instruction
 *
 * @code %value = neg %a : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_neg_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                     tinytc_value_t a,
                                                     const tinytc_location_t *loc);

/**
 * @brief Create alloca instruction
 *
 * @code %value = alloca -> %ty @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param ty [in] type that is allocated
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_alloca_inst_create(tinytc_inst_t *instr,
                                                        tinytc_value_t *result,
                                                        tinytc_data_type_t ty,
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
 * @return tinytc_success on success and error otherwise
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
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param mode [in] expanded mode
 * @param expand_shape_size [in] dimension of expand shape; must be at least 2
 * @param expand_shape [in][range(0, expand_shape_size)] expand shape array
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_expand_inst_create(tinytc_inst_t *instr,
                                                        tinytc_value_t *result, tinytc_value_t a,
                                                        int64_t mode, uint32_t expand_shape_size,
                                                        tinytc_value_t *expand_shape,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create fuse instruction
 *
 * @code %value = fuse %a[%from, %to] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param from [in] first mode to fuse
 * @param to [in] last mode to fuse
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_fuse_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                      tinytc_value_t a, int64_t from, int64_t to,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create load instruction
 *
 * @code %value = load %a[%index_list] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param index_list_size [in] number of indices
 * @param index_list [in][range(0, index_list_size)] indices array; may be nullptr if
 * index_list_size is 0
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_load_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                      tinytc_value_t a, uint32_t index_list_size,
                                                      tinytc_value_t *index_list,
                                                      const tinytc_location_t *loc);
/**
 * @brief Create group_id instruction
 *
 * @code %value = group_id @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_id_inst_create(tinytc_inst_t *instr,
                                                          tinytc_value_t *result,
                                                          const tinytc_location_t *loc);

/**
 * @brief Create group_size instruction
 *
 * @code %value = group_size @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_size_inst_create(tinytc_inst_t *instr,
                                                            tinytc_value_t *result,
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
 * @return tinytc_success on success and error otherwise
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
 * @return tinytc_success on success and error otherwise
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
 * @return tinytc_success on success and error otherwise
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
 * @return tinytc_success on success and error otherwise
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
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param mode [in] mode for that the size is queried
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_size_inst_create(tinytc_inst_t *instr, tinytc_value_t *result,
                                                      tinytc_value_t a, int64_t mode,
                                                      const tinytc_location_t *loc);

/**
 * @brief Create subview instruction
 *
 * @code %value = subview %a[%slices] : type(%a) @endcode
 *
 * @param instr [out] pointer to the inst object created
 * @param result [out] pointer to the resulting value object created
 * @param a [in] operand
 * @param slice_list_size [in] number of slices
 * @param slice_list [in][range(0, slice_list_size)] slice array; may be nullptr if slice_list_size
 * is 0
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_subview_inst_create(tinytc_inst_t *instr,
                                                         tinytc_value_t *result, tinytc_value_t a,
                                                         uint32_t slice_list_size,
                                                         tinytc_slice_t *slice_list,
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
 * @return tinytc_success on success and error otherwise
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
 * @param atomic [in] true for atomic updates of C
 * @param tA [in] operation applied on A
 * @param alpha [in] @$\alpha@$
 * @param A [in] A
 * @param beta [in] @$\beta@$
 * @param B [in] B
 * @param loc [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_sum_inst_create(tinytc_inst_t *instr, tinytc_bool_t atomic,
                                                     tinytc_transpose_t tA, tinytc_value_t alpha,
                                                     tinytc_value_t A, tinytc_value_t beta,
                                                     tinytc_value_t B,
                                                     const tinytc_location_t *loc);

///**
// * @code
// * for %loop_var = %from, %to : type(%loop_var) { %body }
// * ; type(%loop_var) == type(%from)
// * ; type(%loop_var) == type(%to)
// * @endcode
// */
// void create_for(value loop_var, value from, value to, region body,
//                std::source_location const loc = std::source_location::current());
///**
// * @code
// * for %loop_var = %from, %to, %step : type(%loop_var) { %body }
// * ; type(%loop_var) == type(%from)
// * ; type(%loop_var) == type(%to)
// * ; type(%loop_var) == type(%step)
// * @endcode
// */
// void create_for(value loop_var, value from, value to, value step, region body,
//                std::source_location const loc = std::source_location::current());
////! Build for-loop with functor f(region_builder&) -> void
// template <typename F>
// void create_for(data_type loop_var_ty, value from, value to, F &&f, std::string const &prefix =
// "",
//                 std::source_location const loc = std::source_location::current()) {
//     create_for<F>(std::move(loop_var_ty), std::move(from), std::move(to), nullptr,
//                   std::forward<F>(f), prefix, std::move(loc));
// }
////! Build for-loop with functor f(region_builder&) -> void
// template <typename F>
// void create_for(data_type loop_var_ty, value from, value to, value step, F &&f,
//                 std::string const &prefix = "",
//                 std::source_location const loc = std::source_location::current()) {
//     auto loop_var = value(std::move(loop_var_ty), this->name(prefix));
//     auto bb = region_builder{};
//     bb.name_counters(this->name_counters());
//     f(bb);
//     create_for(std::move(loop_var), std::move(from), std::move(to), std::move(step),
//                bb.get_product(), std::move(loc));
// }
///**
// * @code
// * foreach %loop_var = %from, %to : type(%loop_var) { %body }
// * ; type(%loop_var) == type(%from)
// * ; type(%loop_var) == type(%to)
// * @endcode
// */
// void create_foreach(value loop_var, value from, value to, region body,
//                    std::source_location const loc = std::source_location::current());
////! Build foreach-loop with functor f(region_builder&) -> void
// template <typename F>
// void create_foreach(data_type loop_var_ty, value from, value to, F &&f,
//                     std::string const &prefix = "",
//                     std::source_location const loc = std::source_location::current()) {
//     auto loop_var = value(std::move(loop_var_ty), this->name(prefix));
//     auto bb = region_builder{};
//     bb.name_counters(this->name_counters());
//     f(bb);
//     create_foreach(std::move(loop_var), std::move(from), std::move(to), bb.get_product(),
//                    std::move(loc));
// }
//
///**
// * @code
// * if %condition { %then } else { %otherwise }
// * @endcode
// *
// * Set otherwise = nullptr to omit the else region
// */
// void create_if(value condition, region then, region otherwise = nullptr,
//               std::source_location const loc = std::source_location::current());
////! Build if with functor then(region_builder&) -> void
// template <typename F>
// void create_if(value condition, F &&then,
//                std::source_location const loc = std::source_location::current()) {
//     auto bb = region_builder{};
//     bb.name_counters(this->name_counters());
//     then(bb);
//     create_if(std::move(condition), bb.get_product(), std::move(loc));
// }
////! Build if/else with functors then(region_builder&) -> void and otherwise(region_builder&) ->
////! void
// template <typename F, typename G>
// void create_ifelse(value condition, F &&then, G &&otherwise,
//                    std::source_location const loc = std::source_location::current()) {
//     auto bb1 = region_builder{};
//     bb1.name_counters(this->name_counters());
//     then(bb1);
//     auto bb2 = region_builder{};
//     bb2.name_counters(this->name_counters());
//     otherwise(bb2);
//     create_if(std::move(condition), bb1.get_product(), bb2.get_product(), std::move(loc));
// }

/**
 * @brief Release inst object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param instr [inout] inst object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_release(tinytc_inst_t instr);

/**
 * @brief Increase reference count of inst object by 1
 *
 * @param instr [inout] inst object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_inst_retain(tinytc_inst_t instr);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_20240409_H
