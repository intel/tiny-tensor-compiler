// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_GENERATOR_20240314_HPP
#define GEMM_GENERATOR_20240314_HPP

#include "tinytc/export.h"

#include <clir/builtin_type.hpp>
#include <clir/func.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace tinytc {

class core_config;
class local_tiling;
enum class scalar_type;
enum class transpose;

//! Struct to handle mixed precision GEMMs
struct TINYTC_EXPORT gemm_scalar_type {
    //! alpha, A, B, beta, C all have the same type
    gemm_scalar_type(scalar_type ty);
    //! alpha's, A's, and B's type is different from beta's and C's type
    gemm_scalar_type(scalar_type alphaAB, scalar_type betaC);
    //! All operands potentially have a different type
    gemm_scalar_type(scalar_type alpha, scalar_type A, scalar_type B, scalar_type beta,
                     scalar_type C);
    scalar_type alpha, ///< @f$\alpha@f$ type
        A,             ///< A element type
        B,             ///< B element type
        beta,          ///< @f$\beta@f$ type
        C;             ///< C element type
};

/**
 * @brief GEMM configuration struct
 *
 * The interface supports the operation
 *
 * C = alpha * opA(A) * opB(B) + beta * C,
 *
 * where
 *
 * opA/B(X) = transA/B == T ? X^T : X
 *
 * C is an MxN matrix, A is a MxK matrix, and B is a KxN matrix.
 *
 * The address of a matrix is calculated as following. Let X be element of {A,B,C}, then
 *
 * X(i,j) = X[i * X_stride[0] + j * X_stride[1]]
 *
 * If the atomic flag is set, C is updated atomically, either using
 *
 * * beta = 0:     atomic store
 * * beta = 1:     atomic fetch add
 * * general beta: atomic compare exchange
 */
struct TINYTC_EXPORT gemm_configuration {
    gemm_scalar_type ty;                  ///< scalar types of alpha, A, B, beta, C
    transpose transA;                     ///< Transposition of A
    transpose transB;                     ///< Transposition of B
    std::int64_t M;                       ///< M, can be set to dynamic
    std::int64_t N;                       ///< N, can be set to dynamic
    std::int64_t K;                       ///< K, can be set to dynamic
    std::array<std::int64_t, 2> A_stride; ///< stride of A, entries can be set to dynamic
    std::array<std::int64_t, 2> B_stride; ///< stride of B, entries can be set to dynamic
    std::array<std::int64_t, 2> C_stride; ///< stride of C, entries can be set to dynamic
    std::optional<double> alpha;          ///< fixed alpha if set; dynamic alpha if std::nullopt
    std::optional<double> beta;           ///< fixed beta if set; dynamic beta if std::nullopt
    bool atomic = false;                  ///< update C atomically

    std::string identifier(
        std::string_view prefix = "gemm") const; ///< convert configuration to identification string
};

/**
 * @brief Generate GEMM
 *
 * @param gemm_cfg configuration
 * @param tiling Size of 2D subgroup grid
 * @param core_cfg Core configuration
 * @param name Routine prefix
 * @param As Memory space of A (global or local)
 * @param Bs Memory space of B (global or local)
 * @param Cs Memory space of C (global or local)
 *
 * @return OpenCL-C AST
 */
TINYTC_EXPORT clir::func generate_gemm(gemm_configuration const &gemm_cfg,
                                       local_tiling const &tiling, core_config const &core_cfg,
                                       std::string_view name,
                                       clir::address_space As = clir::address_space::global_t,
                                       clir::address_space Bs = clir::address_space::global_t,
                                       clir::address_space Cs = clir::address_space::global_t);

/**
 * @brief Calculate maximum register blocking size of GEMM
 *
 * @param C_scalar_type_size_in_bytes Size of scalar type of result matrix in bytes
 * @param sgs Subgroup size
 * @param register_space Size of register file per core in bytes
 * @param max_fill_fraction Fraction of register file that shall be blocked at most
 *
 * @return {number of row-blocks (block size = subgroup size), number of columns}
 */
TINYTC_EXPORT auto
max_register_block_gemm(std::uint32_t C_scalar_type_size_in_bytes, std::uint32_t sgs,
                        std::uint32_t register_space,
                        std::pair<std::uint32_t, std::uint32_t> max_fill_fraction = {1, 2})
    -> std::pair<std::uint32_t, std::uint32_t>;

} // namespace tinytc

#endif // GEMM_GENERATOR_20240314_HPP
