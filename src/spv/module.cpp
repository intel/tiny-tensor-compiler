// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/types.h"

#include "spv/defs.hpp"
#include "spv/module.hpp"
#include "spv/pass/dump_asm.hpp"
#include "support/ilist_base.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

namespace tinytc {
void ilist_callbacks<spv::spv_inst>::node_added(spv::spv_inst *) {}
void ilist_callbacks<spv::spv_inst>::node_removed(spv::spv_inst *node) { delete node; }
} // namespace tinytc

using namespace tinytc;

tinytc_spv_mod::tinytc_spv_mod(compiler_context ctx, tinytc_core_feature_flags_t core_features,
                               std::int32_t major_version, std::int32_t minor_version)
    : ctx_{std::move(ctx)}, core_features_(core_features), major_version_{major_version},
      minor_version_{minor_version} {}
tinytc_spv_mod::~tinytc_spv_mod() {}

auto tinytc_spv_mod::bound() const -> std::uint32_t {
    std::uint32_t bnd = 0;
    for (auto const &sec : insts_) {
        for (auto const &i : sec) {
            if (i.has_result_id()) {
                bnd = std::max(bnd, i.id());
            }
        }
    }
    return bnd + 1;
}

extern "C" {

tinytc_status_t tinytc_spv_mod_dump(const_tinytc_spv_mod_t mod) {
    if (mod == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { spv::dump_asm_pass{std::cerr}.run_on_module(*mod); });
}

tinytc_status_t tinytc_spv_mod_print_to_file(const_tinytc_spv_mod_t mod, char const *filename) {
    if (mod == nullptr || filename == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto stream = std::ofstream(filename);
        if (!stream.good()) {
            throw status::file_io_error;
        }
        spv::dump_asm_pass{stream}.run_on_module(*mod);
    });
}

tinytc_status_t tinytc_spv_mod_print_to_string(const_tinytc_spv_mod_t mod, char **str) {
    if (mod == nullptr || str == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const text = [&] {
            auto oss = std::ostringstream{};
            spv::dump_asm_pass{oss}.run_on_module(*mod);
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
tinytc_status_t tinytc_spv_mod_release(tinytc_spv_mod_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_spv_mod_retain(tinytc_spv_mod_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
