// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef KERNEL_METADATA_20240412_HPP
#define KERNEL_METADATA_20240412_HPP

#include <array>
#include <cstdint>

namespace tinytc {

//! Kernel metadata
struct kernel_metadata {
    std::int32_t subgroup_size;                  ///< Subgroup size
    std::array<std::int32_t, 2> work_group_size; ///< Work-group size
};

} // namespace tinytc

#endif // KERNEL_METADATA_20240412_HPP
