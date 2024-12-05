// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GCD_20241203_HPP
#define GCD_20241203_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace tinytc {

class gcd_analysis_result {
  public:
    auto get(::const_tinytc_value_t a) const -> std::int64_t;
    auto get(::tinytc_value const &a) const -> std::int64_t;
    auto get_if(::const_tinytc_value_t a) const -> std::optional<std::int64_t>;
    auto get_if(::tinytc_value const &a) const -> std::optional<std::int64_t>;
    void set(::tinytc_value const &a, std::int64_t g);

  private:
    std::unordered_map<::tinytc_value const *, std::int64_t> gcd_;
};

/**
 * In the "GCD-analysis" we want to infer at compile time which integer divide an SSA value is
 * divisible. For example, for
 *
 * %0 = constant 32 : index
 * %1 = arith.mul %0, %x : index
 *
 * we know that %1 is at least divisible by 2, 4, 8, 16, 32 without knowing anything about %x.
 *
 * For the analysis, let P(%x) be the set of known prime factors of a value %x. We define
 *
 * "%x = constant C": P(%x) := set of prime factors of |C|
 *
 * For multiplication and addition formulae we define
 * "%z = arith.add %x, %y": P(%z) := intersection(P(%x),P(%y))
 * "%z = arith.sub %x, %y": P(%z) := intersection(P(%x),P(%y))
 * "%z = arith.mul %x, %y": P(%z) := union(P(%x),P(%y))
 *
 * If nothing is known about %x we let
 * P(%x) := {1}
 *
 * For efficiency, we encode the set of prime factors by its product, that is, by the integer
 *
 * p(%x) := \prod_{f\in P(%x)} f
 *
 * We can update p without having to resort to P as following:
 *
 * "%x = constant C": p(%x) := C
 * "%z = arith.add %x, %y": p(%z) := gcd(p(%x),p(%y))
 * "%z = arith.sub %x, %y": p(%z) := gcd(p(%x),p(%y))
 * "%z = arith.mul %x, %y": p(%z) := p(%x) * p(%y)
 * "%x = unknown":          p(%x) := 1
 *
 * where gcd is the greatest common divisor.
 *
 * One special case is when we have %zero = constant 0. By definition gcd(%zero) = 0.
 * We then have automatically
 * "%z = arith.add %zero, %x": p(%z) = p(%x) // Fine, 0 + x = x so we must have p(%z) = p(%x)
 * "%z = arith.mul %zero, %x": p(%z) = 0     // Fine, 0 * x = 0 so we must have p(%z) = p(%zero)
 */
class gcd_analysis {
  public:
    auto run_on_function(function_node const &fn) -> gcd_analysis_result;
};

} // namespace tinytc

#endif // GCD_20241203_HPP
