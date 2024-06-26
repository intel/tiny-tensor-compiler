// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ERROR_20240410_HPP
#define ERROR_20240410_HPP

#include "parser.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <exception>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>

namespace tinytc {

auto report_error_with_context(char const *code, std::size_t code_len, std::string const &file_name,
                               location const &l, std::string const &what) -> std::string;

//! Compilation error
class compilation_error : public std::exception {
  public:
    //! ctor; taking location, status code, and expanatory string
    compilation_error(location const &loc, status code, std::string extra_info = {});
    //! Get status code
    inline auto code() const noexcept { return code_; }
    //! Get location
    inline auto loc() const noexcept -> location const & { return loc_; }
    //! Get explanatory string
    inline char const *what() const noexcept override { return error_string(code_); }
    //! Get additional information
    inline auto extra_info() const -> std::string const & { return extra_info_; }

  private:
    location loc_;
    status code_;
    std::string extra_info_;
};

class internal_compiler_error : public std::exception {
  public:
    inline char const *what() const noexcept override { return "Internal compiler error"; }
};

template <typename F>
auto exception_to_status_code(F &&f, tinytc_source_context_t context = nullptr) -> tinytc_status_t {
    try {
        f();
    } catch (internal_compiler_error const &e) {
        return tinytc_status_internal_compiler_error;
    } catch (status const &e) {
        return static_cast<tinytc_status_t>(e);
    } catch (builder_error const &e) {
        if (context) {
            context->report_error(e.loc(), e.what());
        }
        return static_cast<tinytc_status_t>(e.code());
    } catch (compilation_error const &e) {
        if (context) {
            if (e.extra_info().size() > 0) {
                auto what =
                    (std::ostringstream{} << e.what() << " (" << e.extra_info() << ')').str();
            } else {
                context->report_error(e.loc(), e.what());
            }
        }
        return static_cast<tinytc_status_t>(e.code());
    } catch (std::bad_alloc const &e) {
        return tinytc_status_bad_alloc;
    } catch (std::out_of_range const &e) {
        return tinytc_status_out_of_range;
    } catch (std::exception const &e) {
        return tinytc_status_runtime_error;
    } catch (...) {
        return tinytc_status_unknown;
    }
    return tinytc_status_success;
}

} // namespace tinytc

#endif // ERROR_20240410_HPP
