// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ERROR_20240410_HPP
#define ERROR_20240410_HPP

#include "location.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <exception>
#include <new>
#include <stdexcept>
#include <string>

namespace tinytc {

//! Compilation error
class compilation_error : public std::exception {
  public:
    //! ctor; taking location, status code, and expanatory string
    compilation_error(location const &loc, status code, std::string const &extra_info = {});
    //! Get status code
    inline auto code() const noexcept { return code_; }
    //! Get location
    inline location loc() const noexcept { return loc_; }
    //! Get explanatory string
    inline char const *what() const noexcept override { return error_string(code_); }
    //! Get additional information
    inline char const *extra_info() const { return extra_info_.c_str(); }

  private:
    location loc_;
    status code_;
    std::string extra_info_;
};

class internal_compiler_error : public std::exception {
  public:
    inline char const *what() const noexcept override { return "Internal compiler error"; }
};

template <typename F> auto exception_to_status_code(F &&f) -> tinytc_status_t {
    try {
        f();
    } catch (internal_compiler_error const &e) {
        return tinytc_status_internal_compiler_error;
    } catch (compilation_error const &e) {
        return static_cast<tinytc_status_t>(e.code());
    } catch (std::bad_alloc const &) {
        return tinytc_status_bad_alloc;
    } catch (std::out_of_range const &) {
        return tinytc_status_out_of_range;
    } catch (...) {
        return tinytc_status_runtime_error;
    }
    return tinytc_status_success;
}

} // namespace tinytc

#endif // ERROR_20240410_HPP
