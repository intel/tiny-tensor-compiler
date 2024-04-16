// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_ERROR_20240416_HPP
#define ZE_ERROR_20240416_HPP

#include <level_zero/ze_api.h>

#include <new>

namespace tinytc {

template <typename F> auto exception_to_ze_result(F &&f) -> ze_result_t {
    try {
        return f();
    } catch (std::bad_alloc const &e) {
        return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    } catch (...) {
        return ZE_RESULT_ERROR_UNKNOWN;
    }
    return ZE_RESULT_SUCCESS;
}

} // namespace tinytc

#endif // ZE_ERROR_20240416_HPP
