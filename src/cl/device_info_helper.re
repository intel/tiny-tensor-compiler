// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info_helper.hpp"

namespace tinytc {

bool has_subgroups_extension(std::size_t str_length, const char *str) {
    const char *YYLIMIT = str + str_length;
    const char *YYCURSOR = str;
lex:
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char;
        re2c:eof = 0;

        whitespace            = [ \t\v\r]+;
        other_extension       = [^ ]+;

        "cl_intel_subgroups" { return true; }
        "cl_khr_subgroups"   { return true; }
        other_extension      { goto lex; }
        *                    { goto lex; }
        $                    { return false; }
    */
    return false;
}

} // namespace tinytc
