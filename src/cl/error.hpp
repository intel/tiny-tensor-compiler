// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_ERROR_20240423_HPP
#define CL_ERROR_20240423_HPP

#include "tinytc/tinytc.hpp"

#include <new>

namespace tinytc {

template <typename F> auto exception_to_status_code_cl(F &&f) -> tinytc_status_t {
    try {
        f();
    } catch (status const &st) {
        return static_cast<tinytc_status_t>(st);
    } catch (builder_error const &e) {
        return static_cast<tinytc_status_t>(e.code());
    } catch (std::bad_alloc const &e) {
        return tinytc_status_bad_alloc;
    } catch (...) {
        return tinytc_status_unknown;
    }
    return tinytc_status_success;
}

} // namespace tinytc

#endif // CL_ERROR_20240423_HPP
