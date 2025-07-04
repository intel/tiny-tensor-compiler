// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/convert_to_spirv.hpp"
#include "spv/converter.hpp"
#include "tinytc/types.hpp"

#include <utility>

namespace tinytc {

convert_to_spirv_pass::convert_to_spirv_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw status::invalid_arguments;
    }
}

auto convert_to_spirv_pass::run_on_program(tinytc_prog &p) -> shared_handle<tinytc_spv_mod_t> {
    return spv::convert_prog_to_spirv(p, *info_);
}

} // namespace tinytc
