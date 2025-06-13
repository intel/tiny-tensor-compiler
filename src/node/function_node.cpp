// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/function_node.hpp"
#include "error.hpp"
#include "node/attr_node.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <cstddef>
#include <utility>

using namespace tinytc;

tinytc_func::tinytc_func(std::string name, tinytc::array_view<tinytc_data_type_t> params,
                         tinytc_data_type_t ty, tinytc_location const &lc)
    : name_(std::move(name)), ty_{ty}, loc_{lc} {
    body_.kind(tinytc::region_kind::collective);
    body_.loc(loc_);
    body_.set_params(std::move(params));
}

void tinytc_func::param_attr(std::int32_t param_no, tinytc_attr_t a) {
    if (param_no < 0 || param_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (static_cast<std::int64_t>(param_attr_.size()) != num_params()) {
        param_attr_.resize(num_params(), nullptr);
    }
    param_attr_[param_no] = a;
}
auto tinytc_func::param_attr(std::int32_t param_no) const -> tinytc_attr_t {
    if (param_no < 0 || param_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (param_attr_.empty()) {
        return nullptr;
    }
    return param_attr_[param_no];
}

auto tinytc_func::subgroup_size() const -> std::int32_t {
    if (auto sgs_attr = get_attr(attr_, "subgroup_size"); sgs_attr) {
        auto sgs = dyn_cast_or_throw<integer_attr>(sgs_attr, [&] {
            return compilation_error(loc_, status::ir_expected_integer_attribute);
        });
        return sgs->value();
    }
    throw compilation_error(loc_, status::internal_compiler_error, "Subgroup size is missing");
}

auto tinytc_func::work_group_size() const -> std::array<std::int32_t, 2u> {
    if (auto wgs_attr = get_attr(attr_, "work_group_size"); wgs_attr) {
        auto wgs_array = dyn_cast_or_throw<array_attr>(
            wgs_attr, [&] { return compilation_error(loc_, status::ir_expected_array_attribute); });
        if (wgs_array->size() != 2) {
            throw compilation_error(loc_, status::ir_unexpected_array_attribute_size,
                                    "Work group size attribute must have 2 entries");
        }
        auto wgs = std::array<std::int32_t, 2u>{};
        for (std::size_t i = 0; i < 2; ++i) {
            wgs[i] = dyn_cast_or_throw<integer_attr>(wgs_array->value(i), [&] {
                         return compilation_error(loc_, status::ir_expected_integer_attribute);
                     })->value();
        }
        return wgs;
    }
    throw compilation_error(loc_, status::internal_compiler_error, "Work group size is missing");
}
