// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"

#include <cctype>
#include <sstream>
#include <utility>

namespace tinytc {

compilation_error::compilation_error(location const &loc, status code, std::string extra_info)
    : loc_(loc), code_(code), extra_info_(std::move(extra_info)) {}

auto report_error_with_context(char const *code, std::size_t code_len, std::string const &file_name,
                               location const &l, std::string const &what) -> std::string {
    constexpr int additional_context_lines = 2;

    int cur_line = 1;
    const char *begin = code;
    const char *limit = begin + code_len;
    while (cur_line + additional_context_lines < l.begin.line && *begin != '\0' && begin <= limit) {
        if (*begin == '\n') {
            ++cur_line;
        }
        ++begin;
    }
    auto oerr = std::ostringstream{};
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
            oerr << std::string(begin, end) << std::endl;
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
                oerr << std::string(col_begin, ' ') << std::string(num_col, '~') << std::endl;
            }
            ++cur_line;
            start_col = -1;
            begin = end + 1;
        }
        ++end;
    }
    oerr << file_name << ":";
    print_range(oerr, l.begin, l.end);
    oerr << ": " << what;
    return oerr.str();
}

} // namespace tinytc

extern "C" {
char const *tinytc_error_string(tinytc_status_t status) {
    switch (status) {
    case tinytc_status_success:
        return "Success";
    case tinytc_status_bad_alloc:
        return "Bad allocation";
    case tinytc_status_invalid_arguments:
        return "Invalid arguments passed to function";
    case tinytc_status_out_of_range:
        return "Out of range";
    case tinytc_status_runtime_error:
        return "General runtime error";
    case tinytc_status_internal_compiler_error:
        return "Internal compiler error";
    case tinytc_status_unsupported_subgroup_size:
        return "Unsupported subgroup size";
    case tinytc_status_unsupported_work_group_size:
        return "Work group size is larger than maximum work group size supported by device";
    case tinytc_status_compilation_error:
        return "Compilation error";
    case tinytc_status_file_io_error:
        return "I/O error occured in file operation";
    case tinytc_status_parse_error:
        return "Parse error";
    case tinytc_status_ir_out_of_bounds:
        return "Argument is out of bounds";
    case tinytc_status_ir_invalid_shape:
        return "Mode size must be non-negative";
    case tinytc_status_ir_incompatible_shapes:
        return "Incompatible tensor shapes";
    case tinytc_status_ir_shape_stride_mismatch:
        return "Dimension of shape and stride must match";
    case tinytc_status_ir_scalar_mismatch:
        return "Scalar type mismatch";
    case tinytc_status_ir_invalid_number_of_indices:
        return "Number of indices must match memref order or must be 1 for group types";
    case tinytc_status_ir_expected_scalar:
        return "Expected scalar type";
    case tinytc_status_ir_expected_memref:
        return "Expected memref type";
    case tinytc_status_ir_expected_memref_or_scalar:
        return "Expected memref type or scalar type";
    case tinytc_status_ir_expected_memref_or_group:
        return "Expected memref or group operand";
    case tinytc_status_ir_expected_vector_or_matrix:
        return "Expected vector or matrix input";
    case tinytc_status_ir_unexpected_yield:
        return "Yield encountered in non-yielding region";
    case tinytc_status_ir_yield_mismatch:
        return "Number of yielded values does not match number of values yielded by region";
    case tinytc_status_ir_multiple_dynamic_modes:
        return "At most one mode must be dynamic ('?')";
    case tinytc_status_ir_invalid_slice:
        return "Offset must be non-negative and must not be '?'; size must be positive or '?'";
    case tinytc_status_ir_expand_shape_order_too_small:
        return "Expand shape must have at least 2 entries";
    case tinytc_status_ir_expand_shape_mismatch:
        return "Product of expand shape must equal mode size";
    case tinytc_status_ir_collective_called_from_spmd:
        return "Collective instruction must not be called from SPMD region";
    }
    return "Unknown status code";
}
}
