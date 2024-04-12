// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "source.hpp"
#include "tinytc/tinytc.h"

#include <memory>
#include <utility>

using namespace tinytc;

extern "C" {

/*tinytc_status_t tinytc_program_create(tinytc_prog_t *prg, uint32_t fun_list_size,
                                      tinytc_func_t *fun_list, const tinytc_location_t *loc) {
    if (prg == nullptr || (fun_list_size > 0 && fun_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto fun_vec = std::vector<func>();
        fun_vec.reserve(fun_list_size);
        for (uint32_t i = 0; i < fun_list_size; ++i) {
            fun_vec.emplace_back(func(fun_list[i], true));
        }
        *prg = std::make_unique<program>(std::move(fun_vec), get_optional(loc)).release();
    });
}*/

tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                              tinytc_core_info_t info) {

}

tinytc_status_t tinytc_prog_compile_to_binary(tinytc_binary_t *bin, tinytc_prog_t prg,
                                              tinytc_core_info_t info,
                                              tinytc_bundle_format_t format);
tinytc_status_t tinytc_source_compile_to_binary(tinytc_binary_t *bin, tinytc_source_t src,
                                                tinytc_core_info_t info,
                                                tinytc_bundle_format_t format);

tinytc_status_t tinytc_source_get_code(tinytc_source_t src, char const **code) {
    if (src == nullptr || code == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *code = src->code(); });
}

tinytc_status_t tinytc_source_destroy(tinytc_source_t src) {
    return exception_to_status_code([&] { delete src; });
}

tinytc_status_t tinytc_binary_destroy(tinytc_binary_t bin) {
    return exception_to_status_code([&] { delete bin; });
}
}
