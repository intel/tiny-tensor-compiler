// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// Code COPIED from Double-Batched FFT Library
// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OPENCL_CC_20240307_HPP
#define OPENCL_CC_20240307_HPP

#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {

class opencl_c_compilation_error : public std::exception {
  public:
    opencl_c_compilation_error(std::string build_log) : build_log_(std::move(build_log)) {}
    inline char const *what() const noexcept override { return build_log_.c_str(); }

  private:
    std::string build_log_;
};

/**
 * @brief Takes OpenCL-C code and outputs a SPIR-V or native device binary
 *
 * @param source_length Source text length (excluding zero terminator)
 * @param source_text OpenCL-C source code (zero-terminated)
 * @param format Target binary format
 * @param ip_version Device ip version; you may pass 0 for format==spirv
 * @param options List of compiler options
 * @param extensions List of OpenCL-C extensions
 *
 * @return binary
 */
std::vector<std::uint8_t> compile_opencl_c(std::size_t source_length, char const *source_text,
                                           bundle_format format, std::uint32_t ip_version,
                                           std::uint32_t options_size = 0,
                                           char const *const *options = nullptr,
                                           std::uint32_t extensions_size = 0,
                                           char const *const *extensions = nullptr);

} // namespace tinytc

#endif // OPENCL_CC_20240307_HPP
