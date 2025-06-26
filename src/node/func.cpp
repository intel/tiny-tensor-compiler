// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/func.hpp"
#include "error.hpp"
#include "location.hpp"
#include "node/attr.hpp"
#include "tinytc/builder.h"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
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

extern "C" {

tinytc_status_t tinytc_func_create(tinytc_func_t *fun, size_t name_length, char const *name,
                                   size_t num_params, const tinytc_data_type_t *param_type_list,
                                   tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (fun == nullptr || (num_params > 0 && param_type_list == nullptr) || ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *fun = std::make_unique<tinytc_func>(std::string(name, name_length),
                                             array_view(param_type_list, num_params), ty,
                                             get_optional(loc))
                   .release();
    });
}

tinytc_status_t tinytc_func_set_parameter_attr(tinytc_func_t fun, int32_t arg_no, attr a) {
    if (fun == nullptr || arg_no < 0 || arg_no >= fun->num_params()) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { fun->param_attr(arg_no, a); });
}

tinytc_status_t tinytc_func_set_attr(tinytc_func_t fun, tinytc_attr_t a) {
    if (fun == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { fun->attr(a); });
}

tinytc_status_t tinytc_func_get_body(tinytc_func_t fun, tinytc_region_t *body) {
    if (fun == nullptr || body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *body = &fun->body(); });
}

void tinytc_func_destroy(tinytc_func_t obj) { delete obj; }
}
