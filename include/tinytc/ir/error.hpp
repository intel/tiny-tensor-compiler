// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ERROR_20240221_HPP
#define ERROR_20240221_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/location.hpp"

#include <cstddef>
#include <exception>
#include <functional>
#include <iosfwd>
#include <string>

namespace tinytc::ir {

constexpr static int additional_context_lines = 2;

//! Type of error reporting functor required in some functions
using error_reporter_function = std::function<void(ir::location const &, std::string const &)>;
inline auto null_error_reporter() -> error_reporter_function {
    return [](ir::location const &, std::string const &) {};
}

//! Compilation error
class TINYTC_EXPORT compilation_error : public std::exception {
  public:
    //! ctor; taking location and expanatory string
    compilation_error(location const &loc, std::string const &what);
    //! Get location
    inline location loc() const noexcept { return loc_; }
    //! Get explanatory string
    inline char const *what() const noexcept override { return what_.c_str(); }

  private:
    location loc_;
    std::string what_;
};

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
                                             std::size_t code_len, ir::location const &l,
                                             std::string const &what);

} // namespace tinytc::ir

#endif // ERROR_20240221_HPP

