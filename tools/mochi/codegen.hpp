// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_20250611_HPP
#define CODEGEN_20250611_HPP

#include "object.hpp"

#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace mochi {

class objects;

template <typename F, typename... T> auto to_string(F generator, T &&...args) -> std::string {
    std::ostringstream oss;
    (*generator)(oss, std::forward<T>(args)...);
    return std::move(oss).str();
}

auto to_c_type(builtin_type ty) -> char const *;
auto to_cxx_type(builtin_type ty, bool pub = false) -> char const *;
void generate_c_type(std::ostream &os, data_type const &ty);
void generate_cxx_type(std::ostream &os, data_type const &ty, bool pub = false);
inline auto to_c_type(data_type const &ty) { return to_string(&generate_c_type, ty); }
inline auto to_cxx_type(data_type const &ty, bool pub = false) {
    return to_string(&generate_cxx_type, ty, pub);
}
void generate_cxx_to_c_cast(std::ostream &os, quantifier q, data_type const &ty,
                            std::string_view name);
void generate_c_to_cxx_cast(std::ostream &os, quantifier q, data_type const &ty,
                            std::string_view name);

void generate_docstring(std::ostream &os, std::string const &doc);

void generate_params(
    inst *in, std::function<void(quantifier, data_type const &, std::string_view, std::string_view)>
                  format_arg);
void generate_c_params(std::ostream &os, inst *in);
void generate_cxx_params(std::ostream &os, inst *in, bool pub = false);

void generate_api_builder_cpp(std::ostream &os, objects const &obj);
void generate_api_builder_h(std::ostream &os, objects const &obj);
void generate_api_builder_hpp(std::ostream &os, objects const &obj);

void generate_enum_cpp(std::ostream &os, objects const &obj);
void generate_enum_h(std::ostream &os, objects const &obj);
void generate_enum_hpp(std::ostream &os, objects const &obj);

void generate_inst_class(std::ostream &os, inst *in);
void generate_inst_create(std::ostream &os, inst *in);

void generate_inst_cpp(std::ostream &os, objects const &obj);
void generate_inst_hpp(std::ostream &os, objects const &obj);
void generate_inst_kind_cpp(std::ostream &os, objects const &obj);
void generate_inst_forward_hpp(std::ostream &os, objects const &obj);

void generate_inst_visit_hpp(std::ostream &os, objects const &obj);

} // namespace mochi

#endif // CODEGEN_20250611_HPP
