// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_OPTIONS_20230621_HPP
#define COMPILER_OPTIONS_20230621_HPP

#include "tinytc/export.h"

#include <string>
#include <vector>

namespace clir {
class func;
}
namespace clir {
class prog;
}

namespace tinytc::internal {

TINYTC_EXPORT extern const std::vector<std::string> default_compiler_options;
TINYTC_EXPORT extern const char *large_register_file_compiler_option_ze;
TINYTC_EXPORT extern const char *large_register_file_compiler_option_cl;

TINYTC_EXPORT std::vector<std::string> required_extensions(clir::func f);
TINYTC_EXPORT std::vector<std::string> required_extensions(clir::prog p);

} // namespace tinytc::internal

#endif // COMPILER_OPTIONS_20230621_HPP
