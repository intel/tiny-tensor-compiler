// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "tinytc/ir/error.hpp"

#include <cctype>
#include <ostream>
#include <utility>

namespace tinytc {

compilation_error::compilation_error(location const &loc, status code,
                                     std::string const &extra_info)
    : loc_(loc), code_(code), extra_info_(std::move(extra_info)) {}

void report_error_with_context(std::ostream *oerr, char const *code, std::size_t code_len,
                               location const &l, std::string const &what) {
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

} // namespace tinytc

extern "C" char const *tinytc_error_string(tinytc_status_t status) {
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
    }
    return "Unknown status code";
}

