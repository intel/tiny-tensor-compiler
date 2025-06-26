// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser_common.hpp"
#include "argparser.hpp"
#include "tinytc/tinytc.hpp"
#include "util/fnv1a.hpp"

#include <cstring>
#include <ostream>

namespace tinytc::cmd {

void add_optflag_states(arg_parser &parser, optflag_states &flags) {
    auto const converter = [](char const *str, std::pair<optflag, std::int32_t> &val) {
        optflag flag = {};
        std::int32_t state = 1;
        constexpr char const disable_prefix[] = "no-";
        constexpr std::size_t disable_prefix_len = sizeof(disable_prefix) - 1;
        if (std::strncmp(str, disable_prefix, disable_prefix_len) == 0) {
            state = 0;
            str = str + disable_prefix_len;
        }
        switch (fnv1a(str, std::strlen(str))) {
        case "unsafe-fp-math"_fnv1a:
            flag = optflag::unsafe_fp_math;
            break;
        default:
            return parser_status::invalid_argument;
        };
        val = std::make_pair(flag, state);
        return parser_status::success;
    };
    parser
        .set_short_opt('f', &flags,
                       "Enable optimization flag; use \"no-\" prefix to disable optimization flag")
        .converter(converter);
}

void set_optflags(compiler_context &ctx, optflag_states const &flags) {
    for (auto const &flag : flags) {
        set_optimization_flag(ctx, flag.first, flag.second);
    }
}

void list_optimization_flags(std::ostream &os) {
    os << "Optimization flags:" << std::endl;
    for (int i = 0; i < arg_parser::optindent; ++i) {
        os << ' ';
    }
    os << "unsafe-fp-math" << std::endl;
}

void add_core_feature_flags(arg_parser &parser, tinytc_core_feature_flags_t &flags) {
    auto const converter = [](char const *str, tinytc_core_feature_flags_t &val) {
        bool clear = false;
        constexpr char const disable_prefix[] = "no-";
        constexpr std::size_t disable_prefix_len = sizeof(disable_prefix) - 1;
        if (std::strncmp(str, disable_prefix, disable_prefix_len) == 0) {
            clear = true;
            str = str + disable_prefix_len;
        }
        tinytc_core_feature_flags_t flag = 0;
        switch (fnv1a(str, std::strlen(str))) {
        case "large-register-file"_fnv1a:
            flag = tinytc_core_feature_flag_large_register_file;
            break;
        default:
            return parser_status::invalid_argument;
        };
        if (clear) {
            val &= ~flag;
        } else {
            val |= flag;
        }
        return parser_status::success;
    };
    parser
        .set_short_opt('F', &flags,
                       "Enable core feature flag; use \"no-\" prefix to clear feature flag")
        .converter(converter);
}

void list_core_feature_flags(std::ostream &os) {
    os << "Core feature flags:" << std::endl;
    for (int i = 0; i < arg_parser::optindent; ++i) {
        os << ' ';
    }
    os << "large-register-file" << std::endl;
}

} // namespace tinytc::cmd
