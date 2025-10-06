// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CORE_20240409_H
#define CORE_20240409_H

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
 * The reference count of the context remains unchanged.
 *
 * @param prg [in] program object
 * @param ctx [out] pointer to context object
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
 * The reference count of the context remains unchanged.
 *
 * @param bin [in] binary object
 * @param ctx [out] pointer to context object
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


#ifdef __cplusplus
}
#endif

#endif // CORE_20240409_H
