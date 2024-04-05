// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TALL_AND_SKINNY_20240313_HPP
#define TALL_AND_SKINNY_20240313_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/gemm_generator.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/runtime.hpp"
#include "tinytc/tensor_kernel.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
class binary;
class core_info;
} // namespace tinytc

namespace tinytc::recipe {

/**
 * @brief Returns a binary for the tall and skinny recipe
 *
 * The binary contains a kernel for beta = 0 called "gemm_beta0" and a kernel for beta != 0 called
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
 * meaning that one has to set arguments to the kernel in the following order:
 *
 * @code
 * alpha, A_ptr, M, ldA, B_ptr, ldB, beta, C_ptr, M, ldC
 * @endcode
 *
 * where ldA, ldB, ldC is the size of stride[1] of A, B, C, respectively.
 *
 * @param ty Scalar types of alpha, A, B, beta, C
 * @param M_block_size Size of M block that each work group gets
 * @param N Number of columns of B, C
 * @param K Number columns of A, number of rows of B
 * @param info Core info
 * @param err Error reporting callback
 *
 * @return Shared pointer to binary; nullptr in case of error
 */
TINYTC_EXPORT auto
generate_tall_and_skinny_binary(ir::gemm_scalar_type ty, std::uint32_t M_block_size,
                                std::uint32_t N, std::uint32_t K, std::shared_ptr<core_info> info,
                                ir::error_reporter_function err = ir::null_error_reporter())
    -> std::shared_ptr<binary>;

/**
 * @brief Creates a tall and skinny GEMM functor
 *
 * @tparam T Floating point type
 * @tparam R Runtime
 */
template <typename T, runtime R> class TINYTC_EXPORT tall_and_skinny {
  public:
    using context_t = typename R::context_t;           ///< Context type
    using device_t = typename R::device_t;             ///< Device type
    using command_list_t = typename R::command_list_t; ///< Command list / queue type
    using event_t = typename R::event_t;               ///< Event type
    using native_event_t = typename R::native_event_t; ///< Native event type
    using mem_t = typename R::mem_t;                   ///< Memory object type
    using const_mem_t = typename R::const_mem_t;       ///< Const memory object type

    /**
     * @brief Compute group size
     *
     * @param M Number of rows of A and C
     *
     * @return Group size
     */
    inline auto howmany(std::uint32_t M) const -> std::size_t {
        return 1 + (M - 1) / M_block_size_;
    }

    /**
     * @brief ctor
     *
     * @param N Number of columns of B and C
     * @param K Number of columns of A, number of rows B
     * @param info Core info
     * @param ctx Context
     * @param dev Device
     */
    tall_and_skinny(std::uint32_t N, std::uint32_t K, std::shared_ptr<core_info> info,
                    context_t ctx, device_t dev)
        : M_block_size_(128),
          bundle_(make_binary(N, K, std::move(info)), std::move(ctx), std::move(dev)),
          gemm_(bundle_.get("gemm")), gemm_beta0_(bundle_.get("gemm_beta0")) {}

    /**
     * @brief Submits a kernel to the runtime for execution on the device
     *
     * This submit prototype is only available if the runtime's native event
     * type supports reference counting.
     *
     * @param M Number of rows of A and C
     * @param alpha @f$\alpha@f$
     * @param A Pointer to A
     * @param ldA Number of elements between columns of A
     * @param B Pointer to B
     * @param ldB Number of elements between columns of B
     * @param beta @f$\beta@f$
     * @param C Pointer to C
     * @param ldC Number of elements between columns of C
     * @param q Queue
     * @param dep_events Vector of events that need to be waited on before execution
     */
    auto operator()(std::uint32_t M, T alpha, const_mem_t A, std::uint32_t ldA, const_mem_t B,
                    std::uint32_t ldB, T beta, mem_t C, std::uint32_t ldC, command_list_t q,
                    std::vector<native_event_t> const &dep_events = {}) -> event_t
    requires(R::is_event_managed)
    {
        auto &k = get_kernel(beta);
        k.set_args(alpha, A, M, ldA, B, ldB, beta, C, M, ldC);
        return k.submit(howmany(M), std::move(q), dep_events);
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device
     *
     * This submit prototype is only available if the lifetime of the runtime's native event type
     * is user-managed.
     *
     * @param M Number of rows of A and C
     * @param alpha @f$\alpha@f$
     * @param A Pointer to A
     * @param ldA Number of elements between columns of A
     * @param B Pointer to B
     * @param ldB Number of elements between columns of B
     * @param beta @f$\beta@f$
     * @param C Pointer to C
     * @param ldC Number of elements between columns of C
     * @param q Command list
     * @param signal_event Event that is signalled on kernel completion
     * @param num_wait_events Number of events that need to be waited on before exuection
     * @param wait_events Pointer to num_wait_events event handles
     */
    void operator()(std::uint32_t M, T alpha, const_mem_t A, std::uint32_t ldA, const_mem_t B,
                    std::uint32_t ldB, T beta, mem_t C, std::uint32_t ldC, command_list_t q,
                    native_event_t signal_event = nullptr, std::uint32_t num_wait_events = 0,
                    native_event_t *wait_events = nullptr)
    requires(!R::is_event_managed)
    {
        auto &k = get_kernel(beta);
        k.set_args(alpha, A, M, ldA, B, ldB, beta, C, M, ldC);
        k.submit(howmany(M), q, signal_event, num_wait_events, wait_events);
    }

  private:
    auto make_binary(std::uint32_t N, std::uint32_t K, std::shared_ptr<core_info> info)
        -> std::shared_ptr<binary> {
        auto last_loc = ir::location{};
        auto last_what = std::string{};
        auto bin = generate_tall_and_skinny_binary(
            ir::to_scalar_type_v<T>, M_block_size_, N, K, std::move(info),
            [&](ir::location const &loc, std::string const &what) {
                last_loc = loc;
                last_what = what;
            });
        if (!bin) {
            throw ir::compilation_error(std::move(last_loc), std::move(last_what));
        }
        return bin;
    }

    auto get_kernel(T beta) -> tensor_kernel<R> & { return beta == T(0.0) ? gemm_beta0_ : gemm_; }

    std::uint32_t M_block_size_;
    tensor_kernel_bundle<R> bundle_;
    tensor_kernel<R> gemm_, gemm_beta0_;
};

} // namespace tinytc::recipe

#endif // TALL_AND_SKINNY_20240313_HPP
