// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/assemble.hpp"
#include "spv/enums.hpp"
#include "spv/inst_assembler.hpp"
#include "spv/module.hpp"
#include "spv/visit.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <cstdint>
#include <vector>

namespace tinytc::spv {

auto assembler::run_on_module(tinytc_spv_mod const &mod) -> binary {
    auto data = std::vector<std::uint8_t>{};
    auto stream = word_stream<std::int32_t>{data};

    const std::int32_t bound = mod.bound();
    // Guess instruction stream by using 5 words per instruction that produces a result
    // Not really important, but could be improved
    data.reserve(5 * sizeof(std::int32_t) * bound);

    // Make header
    const std::int32_t version = (mod.major_version() << 16) | (mod.minor_version() << 8);
    const std::int32_t generator_number = 0;
    stream << magic_number << version << generator_number << bound << std::int32_t{0};

    // Assemble instructions
    auto ia = inst_assembler{stream};
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto const &i : mod.insts(enum_cast<section>(s))) {
            visit(ia, i);
        }
    }

    // Create binary
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_binary_create(&bin, mod.context(), tinytc_bundle_format_spirv, data.size(),
                                      data.data(), mod.core_features()));
    return binary{bin};
}

} // namespace tinytc::spv

