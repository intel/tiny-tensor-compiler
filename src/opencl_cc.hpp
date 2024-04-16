// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// Code COPIED from Double-Batched FFT Library
// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OPENCL_CC_20240307_HPP
#define OPENCL_CC_20240307_HPP

#include <cstdint>
#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {

enum class bundle_format;

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
 * @param source OpenCL-C source code
 * @param format Target binary format
 * @param ip_version Device ip version; you may pass 0 for format==spirv
 * @param options List of compiler options
 * @param extensions List of OpenCL-C extensions
 *
 * @return binary
 */
std::vector<std::uint8_t> compile_opencl_c(std::string const &source, bundle_format format,
                                           std::uint32_t ip_version,
                                           std::vector<char const *> const &options = {},
                                           std::vector<char const *> const &extensions = {});

} // namespace tinytc

#endif // OPENCL_CC_20240307_HPP
