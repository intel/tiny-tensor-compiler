// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SMALL_GEMM_BATCHED_20240307_HPP
#define SMALL_GEMM_BATCHED_20240307_HPP

#include "tinytc/export.h"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/gemm_generator.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/runtime.hpp"
#include "tinytc/tensor_kernel.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
class binary;
class core_info;
namespace ir {
enum class transpose;
}
} // namespace tinytc

namespace tinytc::recipe {

/**
 * @brief Returns a binary for the small batched GEMM recipe
 *
 * The binary contains a kernel for @f$\beta=0@f$ called "gemm_beta0" and a kernel for @f$\beta\neq
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
 * meaning that one has to set arguments to the kernel in the following order:
 *
 * @code
 * alpha, A_ptr, howmany, B_ptr, howmany, beta, C_ptr, howmany
 * @endcode
 *
 * @param ty Scalar types of alpha, A, B, beta, C
 * @param M Number of rows of A, C
 * @param N Number of columns of B, C
 * @param K Number columns of A, number of rows of B
 * @param tA Transpose A
 * @param tB Transpose B
 * @param ldA Leading dimension of A
 * @param strideA Number of elements between A-matrices
 * @param ldB Leading dimension of B
 * @param strideB Number of elements between B-matrices
 * @param ldC Leading dimension of C
 * @param strideC Number of elements between C-matrices
 * @param info Core info
 * @param err Error reporting callback
 *
 * @return Shared pointer to binary; nullptr in case of error
 */
TINYTC_EXPORT auto generate_small_gemm_batched_binary(
    gemm_scalar_type ty, transpose tA, transpose tB, std::uint32_t M, std::uint32_t N,
    std::uint32_t K, std::uint32_t ldA, std::uint32_t strideA, std::uint32_t ldB,
    std::uint32_t strideB, std::uint32_t ldC, std::uint32_t strideC,
    std::shared_ptr<core_info> info, error_reporter_function err = null_error_reporter())
    -> std::shared_ptr<binary>;

/**
 * @brief Creates a batched small GEMM functor
 *
 * @tparam T Floating point type
 * @tparam R Runtime
 */
template <typename T, runtime R> class TINYTC_EXPORT small_gemm_batched {
  public:
    using context_t = typename R::context_t;           ///< Context type
    using device_t = typename R::device_t;             ///< Device type
    using command_list_t = typename R::command_list_t; ///< Command list / queue type
    using event_t = typename R::event_t;               ///< Event type
    using native_event_t = typename R::native_event_t; ///< Native event type
    using mem_t = typename R::mem_t;                   ///< Memory object type
    using const_mem_t = typename R::const_mem_t;       ///< Const memory object type

    /**
     * @brief ctor
     *
     * @param tA Transpose A
     * @param tB Transpose B
     * @param M Number of rows of op_(A) and C
     * @param N Number of columns of op_(B) and C
     * @param K Number of columns of op_A(A), number of rows of op_B(B)
     * @param ldA Number of elements between columns of A
     * @param strideA Number of elements between A matrices
     * @param ldB Number of elements between columns of B
     * @param strideB Number of elements between B matrices
     * @param ldC Number of elements between columns of C
     * @param strideC Number of elements between C matrices
     * @param info Core info
     * @param ctx Context
     * @param dev Device
     */
    small_gemm_batched(transpose tA, transpose tB, std::uint32_t M, std::uint32_t N,
                       std::uint32_t K, std::uint32_t ldA, std::uint32_t strideA, std::uint32_t ldB,
                       std::uint32_t strideB, std::uint32_t ldC, std::uint32_t strideC,
                       std::shared_ptr<core_info> info, context_t ctx, device_t dev)
        : bundle_(make_binary(tA, tB, M, N, K, ldA, strideA, ldB, strideB, ldC, strideC,
                              std::move(info)),
                  std::move(ctx), std::move(dev)),
          gemm_(bundle_.get("gemm")), gemm_beta0_(bundle_.get("gemm_beta0")) {}

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * This submit prototype is only available if the runtime's native event
     * type supports reference counting.
     *
     * @param howmany Group size
     * @param alpha @f$\alpha@f$
     * @param A Pointer to A batch
     * @param B Pointer to B batch
     * @param beta @f$\beta@f$
     * @param C Pointer to C batch
     * @param q Queue
     * @param dep_events Vector of events that need to be waited on before execution
     */
    auto operator()(std::uint32_t howmany, T alpha, const_mem_t A, const_mem_t B, T beta, mem_t C,
                    command_list_t q, std::vector<native_event_t> const &dep_events = {}) -> event_t
    requires(R::is_event_managed)
    {
        auto &k = get_kernel(beta);
        k.set_args(alpha, A, howmany, B, howmany, beta, C, howmany);
        return k.submit(howmany, std::move(q), dep_events);
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * This submit prototype is only available if the lifetime of the runtime's native event type
     * is user-managed.
     *
     * @param howmany Group size
     * @param alpha @f$\alpha@f$
     * @param A Pointer to A batch
     * @param B Pointer to B batch
     * @param beta @f$\beta@f$
     * @param C Pointer to C batch
     * @param q Command list
     * @param signal_event Event that is signalled on kernel completion
     * @param num_wait_events Number of events that need to be waited on before execution
     * @param wait_events Pointer to num_wait_events event handles
     */
    void operator()(std::uint32_t howmany, T alpha, const_mem_t A, const_mem_t B, T beta, mem_t C,
                    command_list_t q, native_event_t signal_event = nullptr,
                    std::uint32_t num_wait_events = 0, native_event_t *wait_events = nullptr)
    requires(!R::is_event_managed)
    {
        auto &k = get_kernel(beta);
        k.set_args(alpha, A, howmany, B, howmany, beta, C, howmany);
        k.submit(howmany, q, signal_event, num_wait_events, wait_events);
    }

  private:
    auto make_binary(transpose tA, transpose tB, std::uint32_t M, std::uint32_t N, std::uint32_t K,
                     std::uint32_t ldA, std::uint32_t strideA, std::uint32_t ldB,
                     std::uint32_t strideB, std::uint32_t ldC, std::uint32_t strideC,
                     std::shared_ptr<core_info> info) -> std::shared_ptr<binary> {
        auto last_loc = location{};
        auto last_what = std::string{};
        auto bin = generate_small_gemm_batched_binary(
            to_scalar_type_v<T>, tA, tB, M, N, K, ldA, strideA, ldB, strideB, ldC, strideC,
            std::move(info), [&](location const &loc, std::string const &what) {
                last_loc = loc;
                last_what = what;
            });
        if (!bin) {
            throw compilation_error(std::move(last_loc), std::move(last_what));
        }
        return bin;
    }

    auto get_kernel(T beta) -> tensor_kernel<R> & { return beta == T(0.0) ? gemm_beta0_ : gemm_; }

    tensor_kernel_bundle<R> bundle_;
    tensor_kernel<R> gemm_, gemm_beta0_;
};

} // namespace tinytc::recipe

#endif // SMALL_GEMM_BATCHED_20240307_HPP
