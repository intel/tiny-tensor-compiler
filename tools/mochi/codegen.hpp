// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_20250611_HPP
#define CODEGEN_20250611_HPP

#include <functional>
#include <iosfwd>
#include <string_view>

namespace mochi {

class inst;
class objects;
enum class quantifier;

void generate_docstring(std::ostream &os, std::string const &doc);

void generate_params(
    std::ostream &os, inst *in,
    std::function<void(quantifier, std::string_view, std::string_view, std::string_view)>
        format_arg);
void generate_c_params(std::ostream &os, inst *in);
void generate_cxx_params(std::ostream &os, inst *in);

void generate_api_builder_cpp(std::ostream &os, objects const &obj);
void generate_api_builder_h(std::ostream &os, objects const &obj);
void generate_api_builder_hpp(std::ostream &os, objects const &obj);

void generate_enum_cpp(std::ostream &os, objects const &obj);
void generate_enum_h(std::ostream &os, objects const &obj);
void generate_enum_hpp(std::ostream &os, objects const &obj);

void generate_inst_class(std::ostream &os, inst *in);
void generate_inst_create(std::ostream &os, inst *in);

void generate_inst_hpp(std::ostream &os, objects const &obj);
void generate_inst_cpp(std::ostream &os, objects const &obj);

void generate_inst_visit_hpp(std::ostream &os, objects const &obj);

} // namespace mochi

#endif // CODEGEN_20250611_HPP
