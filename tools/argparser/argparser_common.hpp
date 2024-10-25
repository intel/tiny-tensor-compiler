// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGPARSER_COMMON_20241010_HPP
#define ARGPARSER_COMMON_20241010_HPP

#include "tinytc/types.hpp"

#include <cstdint>
#include <iosfwd>
#include <utility>
#include <vector>

namespace tinytc {
class compiler_context;
}

namespace tinytc::cmd {
class arg_parser;

using optflag_states = std::vector<std::pair<optflag, std::int32_t>>;

void add_optflag_states(arg_parser &parser, optflag_states &flags);
void set_optflags(compiler_context &ctx, optflag_states const &flags);
void list_optimization_flags(std::ostream &os);

void add_core_feature_flags(arg_parser &parser, tinytc_core_feature_flags_t &flags);
void list_core_feature_flags(std::ostream &os);

} // namespace tinytc::cmd

#endif // ARGPARSER_COMMON_20241010_HPP
