// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ERROR_20240221_HPP
#define ERROR_20240221_HPP

#include "tinytc/export.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <exception>
#include <functional>
#include <iosfwd>
#include <string>

namespace tinytc {

constexpr static int additional_context_lines = 2;

//! Type of error reporting functor required in some functions
using error_reporter_function = std::function<void(location const &, std::string const &)>;
inline auto null_error_reporter() -> error_reporter_function {
    return [](location const &, std::string const &) {};
}

/**
 * @brief Report error and show code context
 *
 * @param oerr Pointer to output stream; can be nullptr to skip error reporting
 * @param code Code string
 * @param code_len Length of code string
 * @param l Code location
 * @param what Explanatory string
 */
TINYTC_EXPORT void report_error_with_context(std::ostream *oerr, char const *code,
                                             std::size_t code_len, location const &l,
                                             std::string const &what);

} // namespace tinytc

#endif // ERROR_20240221_HPP

