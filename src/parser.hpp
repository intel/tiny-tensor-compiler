// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20230614_HPP
#define PARSER_20230614_HPP

#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <string>
#include <vector>

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

    //! Create abstract syntax tree from file
    auto parse_file(char const *filename) -> tinytc::prog;
    //! Create abstract syntax tree from stdin
    auto parse_stdin() -> tinytc::prog;
    //! Create abstract syntax tree from string
    auto parse_string(std::uint64_t size, char const *src) -> tinytc::prog;

    //! Annotate context to error message
    void report_error(tinytc_location const &l, std::string const &what);
    //! Return error log of last parse call
    inline auto last_error_log() -> std::string const & { return last_error_log_; }

  private:
    struct source_input {
        std::string name, text;
    };

    auto parse(source_input const &input, tinytc_location const &initial_loc) -> tinytc::prog;

    auto add_source_input(source_input src) -> tinytc_location;
    int stdin_counter_ = 0, memory_counter_ = 0;
    std::vector<source_input> sources_;
    std::string last_error_log_;
};

#endif // PARSER_20230614_HPP
