// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info_helper.hpp"

#include <cctype>

namespace tinytc {

auto get_opencl_extensions(std::size_t str_length, const char *str) -> opencl_exts_t {
    const char *YYLIMIT = str + str_length;
    const char *YYCURSOR = str;
    const char *YYMARKER;
    opencl_exts_t result = 0;
    for (;;) {
        /*!re2c
            re2c:yyfill:enable = 0;
            re2c:define:YYCTYPE = char;
            re2c:eof = 0;

            whitespace            = [ \t\v\r]+;


            "cl_khr_fp16"                     { result |= opencl_ext_cl_khr_fp16; continue; }
            "cl_khr_fp64"                     { result |= opencl_ext_cl_khr_fp64; continue; }
            "cl_khr_subgroups"                { result |= opencl_ext_cl_khr_subgroups; continue; }
            "cl_intel_subgroups"              { result |= opencl_ext_cl_intel_subgroups; continue; }
            "cl_intel_required_subgroup_size" {
                result |= opencl_ext_cl_intel_required_subgroup_size; continue;
            }
            "cl_intel_subgroups_long"         {
                result |= opencl_ext_cl_intel_subgroups_long; continue;
            }
            "cl_intel_subgroups_short"        {
                result |= opencl_ext_cl_intel_subgroups_short; continue;
            }
            "cl_intel_spirv_subgroups"        {
                result |= opencl_ext_cl_intel_spirv_subgroups; continue;
            }
            "cl_khr_int64_base_atomics"       {
                result |= opencl_ext_cl_khr_int64_base_atomics; continue;
            }
            "cl_khr_int64_extended_atomics"   {
                result |= opencl_ext_cl_khr_int64_extended_atomics; continue;
            }
            "cl_ext_float_atomics"            {
                result |= opencl_ext_cl_ext_float_atomics; continue;
            }
            whitespace                        { continue; }
            *                                 {
                // skip remaining characters until we find whitespace
                while (!std::isspace(*YYCURSOR) && YYCURSOR < YYLIMIT) {
                    ++YYCURSOR;
                }
                continue;
            }
            $                                 { break; }
        */
    }
    return result;
}

auto get_opencl_extensions(cl_device_id device) -> opencl_exts_t {
    std::string extensions = device_info<std::string>(device, CL_DEVICE_EXTENSIONS);
    return get_opencl_extensions(extensions.size(), extensions.c_str());
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

auto get_opencl_version(cl_device_id device) -> opencl_version {
    std::string version = device_info<std::string>(device, CL_DEVICE_VERSION);
    return get_opencl_version(version.size(), version.c_str());
}

} // namespace tinytc
