// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_ERROR_20240419_HPP
#define ZE_ERROR_20240419_HPP

#include "tinytc/core.h"
#include "tinytc/core.hpp"

#include <new>

namespace tinytc {

template <typename F> auto exception_to_status_code_ze(F &&f) -> tinytc_status_t {
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

#endif // ERROR_20240419_HPP
