// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BUNDLE_FORMAT_20240308_HPP
#define BUNDLE_FORMAT_20240308_HPP

namespace tinytc {

//! Target binary format
enum class bundle_format {
    spirv, ///< SPIR-V
    native ///< Native device binary
};

} // namespace tinytc

#endif // BUNDLE_FORMAT_20240308_HPP
