// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_20250611_HPP
#define CODEGEN_20250611_HPP

#include <iosfwd>

namespace mochi {

class inst;
class objects;

void generate_inst_class(std::ostream &os, inst *in);
void generate_inst_create_prototype(std::ostream &os, inst *in, bool insert_class_name = false);
void generate_inst_create(std::ostream &os, inst *in);

void generate_inst_header(std::ostream &os, objects const &obj);
void generate_inst_cpp(std::ostream &os, objects const &obj);

} // namespace mochi

#endif // CODEGEN_20250611_HPP
