// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_ERROR_20240423_HPP
#define SYCL_ERROR_20240423_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_sycl.h"

#include <new>
#include <sycl/sycl.hpp>

namespace tinytc {

template <typename F> auto exception_to_status_code_sycl(F &&f) -> tinytc_status_t {
    try {
        f();
    } catch (status const &st) {
        return static_cast<tinytc_status_t>(st);
    } catch (builder_error const &e) {
        return static_cast<tinytc_status_t>(e.code());
    } catch (sycl::exception const &e) {
        return tinytc_sycl_convert_status(e.code().value());
    } catch (std::bad_alloc const &e) {
        return tinytc_status_bad_alloc;
    } catch (...) {
        return tinytc_status_unknown;
    }
    return tinytc_status_success;
}

} // namespace tinytc

#endif // SYCL_ERROR_20240423_HPP
