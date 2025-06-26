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

////////////////////////////
////////// FP math /////////
////////////////////////////

/**
 * @brief Convert f32 number to bf16 number (represented as ushort)
 *
 * @param x f32 number
 *
 * @return bf16 number
 */
TINYTC_EXPORT uint16_t tinytc_f32_to_bf16_as_ui16(float x);

/**
 * @brief Convert bf16 number (represented as ushort) to f32 number
 *
 * @param x bf16 number
 *
 * @return f32 number
 */
TINYTC_EXPORT float tinytc_bf16_as_ui16_to_f32(uint16_t x);

/**
 * @brief Convert f32 number to f16 number (represented as ushort)
 *
 * @param x f32 number
 *
 * @return f16 number
 */
TINYTC_EXPORT uint16_t tinytc_f32_to_f16_as_ui16(float x);

/**
 * @brief Convert f16 number (represented as ushort) to f32 number
 *
 * @param x f16 number
 *
 * @return f32 number
 */
TINYTC_EXPORT float tinytc_f16_as_ui16_to_f32(uint16_t x);

////////////////////////////
/////////// Prog ///////////
////////////////////////////

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
TINYTC_EXPORT tinytc_status_t tinytc_prog_dump(tinytc_prog_t prg);

/**
 * @brief Print program to file
 *
 * @param prg [in] program object
 * @param filename [in] filename
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_print_to_file(tinytc_prog_t prg, char const *filename);

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
TINYTC_EXPORT tinytc_status_t tinytc_prog_print_to_string(tinytc_prog_t prg, char **str);

/**
 * @brief Dump SPIR-V module to stderr
 *
 * @param mod [in] module
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_spv_mod_dump(const_tinytc_spv_mod_t mod);

/**
 * @brief Print SPIR-V module to file
 *
 * @param mod [in] module
 * @param filename [in] filename
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_spv_mod_print_to_file(const_tinytc_spv_mod_t mod,
                                                           char const *filename);

/**
 * @brief Print SPIR-V module to string
 *
 * The user is responsible to dispose the string with tinytc_string_destroy.
 *
 * @param mod [in] module
 * @param str [out] pointer to string
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_spv_mod_print_to_string(const_tinytc_spv_mod_t mod,
                                                             char **str);

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
                                                              size_t sgs_size, int32_t const *sgs);

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
 * @brief Look up core info for Intel GPU architecture
 *
 * @param info [out] pointer to the core_info object created
 * @param name [in] architecture name
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_intel_create_from_name(tinytc_core_info_t *info,
                                                                      char const *name);

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
                                                            size_t sgs_size, int32_t const *sgs);

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
                                                                  size_t *sgs_size,
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
 * @param info [inout] core info object
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
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_core_features(
    const_tinytc_core_info_t info, tinytc_core_feature_flags_t *flags);

/**
 * @brief Set SPIR-V feature
 *
 * @param info [inout] core info object
 * @param feature [in] SPIR-V feature
 * @param available [in] Set to true if feature is available and false otherwise
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_set_spirv_feature(tinytc_core_info_t info,
                                                                 tinytc_spirv_feature_t feature,
                                                                 tinytc_bool_t available);

/**
 * @brief Get SPIR-V feature
 *
 * @param info [in] core info object
 * @param feature [in] SPIR-V feature
 * @param available [out] Writes true to available if feature is available and false otherwise
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_have_spirv_feature(const_tinytc_core_info_t info,
                                                                  tinytc_spirv_feature_t feature,
                                                                  tinytc_bool_t *available);

/**
 * @brief Get default memref alignment
 *
 * @param info [in] Core info
 * @param alignment [out] pointer to alignment in bytes
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_get_default_alignment(const_tinytc_core_info_t info,
                                                                     int32_t *alignment);

/**
 * @brief Set default memref alignment
 *
 * @param info [inout] Core info
 * @param alignment [in] alignment in bytes
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_core_info_set_default_alignment(tinytc_core_info_t info,
                                                                     int32_t alignment);

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
 * @param ctx [inout] context object
 * @param reporter [in] error reporting callback; set to nullptr to disable reporting
 * @param user_data [in][optional] pointer to user data that is passed to the callback; can be
 * nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_set_error_reporter(
    tinytc_compiler_context_t ctx, tinytc_error_reporter_t reporter, void *user_data);

/**
 * @brief Sets an optimization flag
 *
 * The state can be 0 (disabled), 1 (enabled), or -1 (use default according to optimization level).
 *
 * @param ctx [inout] context object
 * @param flag [in] optimization flag
 * @param state [in] flag state
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_compiler_context_set_optimization_flag(
    tinytc_compiler_context_t ctx, tinytc_optflag_t flag, int32_t state);

/**
 * @brief Set optimization level (from 0 to 2)
 *
 * @param ctx [inout] context object
 * @param level [in] optimization level
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t
tinytc_compiler_context_set_optimization_level(tinytc_compiler_context_t ctx, int32_t level);

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
TINYTC_EXPORT tinytc_status_t tinytc_list_function_passes(size_t *names_size,
                                                          char const *const **names);

/**
 * @brief Compile tensor language to SPIR-V
 *
 * @param mod [out] pointer to the SPIR-V module created
 * @param prg [inout] tensor program; modified as compiler passes are run
 * @param info [in] core info object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_compile_to_spirv(tinytc_spv_mod_t *mod, tinytc_prog_t prg,
                                                           const_tinytc_core_info_t info);

/**
 * @brief Compiler tensor language to SPIR-V and assemble
 *
 * @param bin [out] pointer to the binary object created
 * @param prg [inout] tensor program; modified as compiler passes are run
 * @param info [in] core info object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_prog_compile_to_spirv_and_assemble(
    tinytc_binary_t *bin, tinytc_prog_t prg, const_tinytc_core_info_t info);

/**
 * @brief Assemble SPIR-V module
 *
 * @param bin [out] pointer to the binary object created
 * @param mod [in] SPIR-V module
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_spirv_assemble(tinytc_binary_t *bin,
                                                    const_tinytc_spv_mod_t mod);

/**
 * @brief Create binary
 *
 * @param bin [out] pointer to binary object
 * @param ctx [in] compiler context
 * @param format [in] Bundle format (SPIR-V or Native)
 * @param data_size [in] Size of data in bytes
 * @param data [in][range(0, data_size)] Binary data; data is copied
 * @param core_features [in][optional] requested core features; must be 0 (default) or a
 * combination of tinytc_core_feature_flag_t
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_create(tinytc_binary_t *bin,
                                                   tinytc_compiler_context_t ctx,
                                                   tinytc_bundle_format_t format, size_t data_size,
                                                   uint8_t const *data,
                                                   tinytc_core_feature_flags_t core_features);

/**
 * @brief Get context object from binary object
 *
 * @param bin [in] binary object
 * @param ctx [out] pointer to context object; reference count is increased so the user needs to
 * call tinytc_compiler_context_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_binary_get_compiler_context(const_tinytc_binary_t bin,
                                                                 tinytc_compiler_context_t *ctx);

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

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Returns a small batched GEMM recipe
 *
 * The program contains a kernel for @f$\beta=0@f$ called "gemm_beta0" and a kernel for
 * @f$\beta\neq 0@f$ called "gemm". All matrix shapes and strides are known at compile-time.
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
 * The program contains a kernel for beta = 0 called "gemm_beta0" and a kernel for beta != 0
 * called "gemm". M (= number of rows of A, C) and strides are dynamic.
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
 * @param M_block_size [in][optional] Size of M block that each work group gets; pass 0 to have
 * the parameter auto-selected
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
 * The specialization constants may be either set to a fixed value or to TINYTC_DYNAMIC.
 * Note that if a specialization constant is set to a fixed value then the parameter with the
 * same name in tinytc_recipe_tall_and_skinny_set_args is ignored.
 *
 * Furthermore, the memory alignment may be passed with alignA, alignB, and alignC or set to 0 to
 * use the default memory alignment (= size of scalar type).
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
 * @param alignA [in] Memory alignment of A; can be 0
 * @param alignB [in] Memory alignment of B; can be 0
 * @param alignC [in] Memory alignment of C; can be 0
 * @param M_block_size [in][optional] Size of M block that each work group gets; pass 0 to have
 * the parameter auto-selected
 * @param ctx [inout][optional] context object; a new context is created if ctx is nullptr
 *
 * @return
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_tall_and_skinny_create_specialized(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t M,
    int64_t N, int64_t K, int64_t ldA, int64_t ldB, int64_t ldC, int32_t alignA, int32_t alignB,
    int32_t alignC, int32_t M_block_size, tinytc_compiler_context_t ctx);

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
 * @param prg [out] pointer to prog object; reference count is increased so the user needs to
 * call tinytc_prog_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_get_prog(const_tinytc_recipe_t recipe,
                                                     tinytc_prog_t *prg);

/**
 * @brief Get binary
 *
 * @param recipe [in] recipe object
 * @param bin [out] pointer to binary; reference count is increased so the user needs to
 * call tinytc_binary_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_recipe_get_binary(const_tinytc_recipe_t recipe,
                                                       tinytc_binary_t *bin);

/**
 * @brief Get recipe object
 *
 * @param handler [in] recipe handler object
 * @param recipe [out] pointer to recipe object; reference count is increased so the user needs
 * to call tinytc_recipe_release to clean up
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t
tinytc_recipe_handler_get_recipe(const_tinytc_recipe_handler_t handler, tinytc_recipe_t *recipe);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_20240409_H
