// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20230614_HPP
#define PARSER_20230614_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace tinytc {
auto parse(std::uint64_t size, char const *input) -> prog;
}

/**
 * @brief Source manager
 *
 * The source manager can parse tensor programs from files, stdin, or memory.
 * Source code is stored in the manager such that error messages can be enhanced
 * with code context.
 */
struct tinytc_source_context {
  public:
    //! @brief ctor
    tinytc_source_context();

    auto parse(std::string name, std::string text) -> tinytc::prog;

    inline auto add_source(char const *name, char const *text) -> std::int32_t {
        sources_.emplace_back(source_input{std::string(name), std::string(text)});
        return static_cast<std::int32_t>(sources_.size());
    }

    //! Annotate context to error message
    void report_error(tinytc_location const &l, char const *what, bool append = false);
    //! Return error log of last parse call
    inline auto last_error_log() -> std::string const & { return last_error_log_; }

  private:
    struct source_input {
        std::string name, text;
    };
    std::vector<source_input> sources_;
    std::string last_error_log_;
};

#endif // PARSER_20230614_HPP
