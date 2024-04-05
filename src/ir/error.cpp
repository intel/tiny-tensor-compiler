// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/error.hpp"

#include <cctype>
#include <ostream>
#include <utility>

namespace tinytc::ir {

compilation_error::compilation_error(location const &loc, std::string const &what)
    : loc_(loc), what_(std::move(what)) {}

void report_error_with_context(std::ostream *oerr, char const *code, std::size_t code_len,
                               ir::location const &l, std::string const &what) {
    if (!oerr) {
        return;
    }
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
    int start_col = 0;
    while (cur_line <= l.end.line && *end != '\0' && end <= limit) {
        if (!std::isspace(*end) && start_col == 0) {
            start_col = static_cast<int>(end - begin);
        }
        if (*end == '\n') {
            *oerr << std::string(begin, end) << std::endl;
            if (cur_line >= l.begin.line) {
                int col_begin = l.begin.column > 1 ? l.begin.column - 1 : 0;
                int num_col = l.end.column > l.begin.column ? l.end.column - l.begin.column : 1;
                if (l.begin.line != l.end.line) {
                    if (cur_line == l.begin.line) {
                        num_col = static_cast<int>(end - begin) - col_begin;
                    } else if (cur_line == l.end.line) {
                        int const delta = start_col - col_begin;
                        col_begin = start_col;
                        num_col = num_col >= delta ? num_col - delta : 0;
                    } else {
                        col_begin = start_col;
                        num_col = static_cast<int>(end - begin) - col_begin;
                    }
                }
                *oerr << std::string(col_begin, ' ') << std::string(num_col, '~') << std::endl;
            }
            ++cur_line;
            start_col = 0;
            begin = end + 1;
        }
        ++end;
    }
    *oerr << l << ": " << what << std::endl;
}

} // namespace tinytc::ir
