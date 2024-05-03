// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info_helper.hpp"

namespace tinytc {

bool has_subgroup_extension(std::size_t str_length, const char *str) {
    const char *YYLIMIT = str + str_length;
    const char *YYCURSOR = str;
    const char *YYMARKER;
    for (;;) {
        /*!re2c
            re2c:yyfill:enable = 0;
            re2c:define:YYCTYPE = char;
            re2c:eof = 0;

            whitespace            = [ \t\v\r]+;

            "cl_intel_subgroups" { return true; }
            "cl_khr_subgroups"   { return true; }
            whitespace           { continue; }
            *                    { continue; }
            $                    { break; }
        */
    }
    return false;
}

bool has_additional_subgroup_extensions(std::size_t str_length, const char *str) {
    const char *YYLIMIT = str + str_length;
    const char *YYCURSOR = str;
    const char *YYMARKER;
    bool has_reqd_subgroup_size = false, has_subgroups_long = false, has_subgroups_short = false;
    for (;;) {
        /*!re2c
            re2c:yyfill:enable = 0;
            re2c:define:YYCTYPE = char;
            re2c:eof = 0;

            "cl_intel_required_subgroup_size"   { has_reqd_subgroup_size = true; continue; }
            "cl_intel_subgroups_long"           { has_subgroups_long = true; continue; }
            "cl_intel_subgroups_short"          { has_subgroups_short = true; continue; }
            whitespace           { continue; }
            *                    { continue; }
            $                    { break; }
        */
    }
    return has_reqd_subgroup_size && has_subgroups_long && has_subgroups_short;
}

auto get_opencl_version(std::size_t str_length, const char *str) -> opencl_version {
    const char *t1, *t2, *t3;
    const char *YYLIMIT = str + str_length;
    const char *YYCURSOR = str;
    const char *YYMARKER;
    int major = 0, minor = 0;

    /*!stags:re2c format = 'const char *@@;\n'; */
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char;
        re2c:eof = 0;

        number = [0-9]+;

        "OpenCL " @t1 number @t2 "." @t3 number {
            major = 0, minor = 0;
            for (; t1 < t2; ++t1) {
                major = 10 * major + (*t1 - '0');
            }
            for (; t3 < YYCURSOR; ++t3) {
                minor = 10 * minor + (*t3 - '0');
            }
            goto ret;
        }
        * { goto ret; }
        $ { goto ret; }
    */
ret:
    return {major, minor};
}

} // namespace tinytc
