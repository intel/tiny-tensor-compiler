// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "ir/node/program_node.hpp"
#include "location.hpp"
#include "tinytc/tinytc.h"

#include <memory>
#include <utility>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_program_create(tinytc_prog_t *prg, uint32_t fun_list_size,
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
}

tinytc_status_t tinytc_prog_release(tinytc_prog_t prg) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ref_count = prg->dec_ref();
        if (ref_count == 0) {
            delete prg;
        }
    });
}

tinytc_status_t tinytc_prog_retain(tinytc_prog_t prg) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { prg->inc_ref(); });
}
}
