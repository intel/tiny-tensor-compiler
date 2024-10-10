// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGPARSER_COMMON_20241010_HPP
#define ARGPARSER_COMMON_20241010_HPP

#include "argparser.hpp"
#include "tinytc/tinytc.hpp"

namespace tinytc::cmd {

struct optflag_states {
    std::int32_t unsafe_fp_math = -1;
};

inline void add_optflag_states(arg_parser &parser, optflag_states &flags) {
    auto const validator = [](std::int32_t value) { return -1 <= value && value <= 1; };
    parser
        .set_long_opt("unsafe-fp-math", &flags.unsafe_fp_math,
                      "Enable unsafe floating point math (e.g. 0.0 * x = 0.0)", 1)
        .validator(validator);
}

inline void set_optflags(compiler_context &ctx, optflag_states const &flags) {
    ctx.set_optimization_flag(optflag::unsafe_fp_math, flags.unsafe_fp_math);
}

} // namespace tinytc::cmd

#endif // ARGPARSER_COMMON_20241010_HPP
