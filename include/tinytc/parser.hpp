// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20230614_HPP
#define PARSER_20230614_HPP

#include "tinytc/export.h"
#include "tinytc/ir/error.hpp"
#include "tinytc/tinytc.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace tinytc {

/**
 * @brief Source manager
 *
 * The source manager can parse tensor programs from files, stdin, or memory.
 * Source code is stored in the manager such that error messages can be enhanced
 * with code context.
 */
class TINYTC_EXPORT source_manager {
  public:
    /**
     * @brief ctor
     *
     * @param oerr ostream for error printing; set to nullptr to omit output
     */
    source_manager(std::ostream *oerr = nullptr);

    //! Create abstract syntax tree from file
    auto parse_file(std::string const &filename) -> prog;
    //! Create abstract syntax tree from stdin
    auto parse_stdin() -> prog;
    //! Create abstract syntax tree from string
    auto parse_string(std::string input) -> prog;

    //! Report error with code context
    void report_error(location const &l, std::string const &what);

    //! Get error reporter
    auto error_reporter() -> error_reporter_function;

  private:
    struct source {
        std::string name, text;
    };
    auto add_source(source src) -> location;
    std::ostream *oerr_;
    int stdin_counter_ = 0, memory_counter_ = 0;
    std::vector<source> sources_;
};

/**
 * @brief Parses a program written in the tensor language
 *
 * @param input Code input
 * @param oerr ostream for error printing; set to nullptr to omit output
 * @param filename Optional filename that is added to source code locations
 *
 * @return Abstract syntax tree on success; nullptr on error
 */
TINYTC_EXPORT auto parse(std::string const &input, location const &initial_loc,
                         std::ostream *oerr = nullptr) -> prog;

} // namespace tinytc

#endif // PARSER_20230614_HPP
