// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace tinytc {

compilation_error::compilation_error(location const &loc, status code, std::string extra_info)
    : loc_(loc), ref_values_{}, num_ref_values_{0}, code_(code),
      extra_info_(std::move(extra_info)) {}

compilation_error::compilation_error(location const &loc,
                                     array_view<const_tinytc_value_t> ref_values, status code,
                                     std::string extra_info)
    : loc_(loc), code_(code), extra_info_(std::move(extra_info)) {
    num_ref_values_ = std::min(error_max_ref, ref_values.size());
    for (std::size_t i = 0; i < num_ref_values_; ++i) {
        ref_values_[i] = ref_values[i];
    }
}

auto report_error_with_context(char const *code, std::size_t code_len, std::string const &file_name,
                               location const &l, std::string const &what) -> std::string {
    constexpr int additional_context_lines = 2;

    auto oerr = std::ostringstream{};
    oerr << file_name << ":";
    print_range(oerr, l.begin, l.end);
    oerr << ": " << what;

    int cur_line = 1;
    const char *begin = code;
    const char *limit = begin + code_len;
    while (cur_line + additional_context_lines < l.begin.line && *begin != '\0' && begin <= limit) {
        if (*begin == '\n') {
            ++cur_line;
        }
        ++begin;
    }

    char const *end = begin;
    int start_col = -1;
    while (cur_line <= l.end.line && *end != '\0' && end <= limit) {
        if (!std::isspace(*end) && start_col < 0) {
            start_col = static_cast<int>(end - begin);
        }
        if (*end == '\n') {
            // start_col < 0 => only white-space
            if (start_col < 0) {
                start_col = static_cast<int>(end - begin);
            }
            oerr << std::endl << std::string(begin, end) << std::endl;
            if (cur_line >= l.begin.line) {
                int col_begin = 0;
                int num_col = 0;
                if (l.begin.line != l.end.line) {
                    if (cur_line == l.begin.line) {
                        col_begin = l.begin.column > 1 ? l.begin.column - 1 : 0;
                        num_col = static_cast<int>(end - begin) - col_begin;
                    } else if (cur_line == l.end.line) {
                        num_col = l.end.column > 1 ? l.end.column - 1 : 0;
                        if (num_col >= start_col) {
                            num_col -= start_col;
                            col_begin = start_col;
                        } else {
                            col_begin = 0;
                        }
                    } else {
                        col_begin = start_col;
                        num_col = static_cast<int>(end - begin) - col_begin;
                    }
                } else {
                    col_begin = l.begin.column > 1 ? l.begin.column - 1 : 0;
                    num_col = l.end.column > l.begin.column ? l.end.column - l.begin.column : 1;
                }
                oerr << std::string(col_begin, ' ') << std::string(num_col, '~');
            }
            ++cur_line;
            start_col = -1;
            begin = end + 1;
        }
        ++end;
    }
    return std::move(oerr).str();
}

} // namespace tinytc
