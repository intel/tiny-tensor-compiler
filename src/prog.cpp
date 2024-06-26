// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/program_node.hpp"
#include "passes.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

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

tinytc_status_t tinytc_prog_release(tinytc_prog_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_retain(tinytc_prog_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_dump(const_tinytc_prog_t prg) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { dump_ir(std::cerr, *prg); });
}

tinytc_status_t tinytc_prog_print_to_file(const_tinytc_prog_t prg, char const *filename) {
    if (prg == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto stream = std::ofstream(filename);
        if (!stream.good()) {
            throw status::file_io_error;
        }
        dump_ir(stream, *prg);
    });
}

tinytc_status_t tinytc_prog_print_to_string(const_tinytc_prog_t prg, char **str) {
    if (prg == nullptr || str == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const text = [&] {
            auto oss = std::ostringstream{};
            dump_ir(oss, *prg);
            return std::move(oss).str();
        }();
        auto const length = text.size() + 1; // Need to include terminating null character
        *str = (char *)malloc(length * sizeof(char));
        if (!str) {
            throw status::bad_alloc;
        }
        std::strncpy(*str, text.c_str(), length);
    });
}

void tinytc_string_destroy(char *str) { free(str); }
}
