// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "omochi.hpp"
#include "codegen.hpp"
#include "parser.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace mochi {

auto to_string(generator g) -> char const * {
    switch (g) {
    case generator::api_builder_cpp:
        return "api_builder_cpp";
    case generator::api_builder_h:
        return "api_builder_h";
    case generator::api_builder_hpp:
        return "api_builder_hpp";
    case generator::enum_cpp:
        return "enum_cpp";
    case generator::enum_h:
        return "enum_h";
    case generator::enum_hpp:
        return "enum_hpp";
    case generator::inst_cpp:
        return "inst_cpp";
    case generator::inst_hpp:
        return "inst_hpp";
    case generator::inst_forward_hpp:
        return "inst_forward_hpp";
    case generator::inst_visit_hpp:
        return "inst_visit_hpp";
    case generator::template_:
        return "template";
    case generator::NUM_GENERATORS:
        break;
    }
    return "unknown";
}

void please_do(std::ostream &os, action const &a, std::vector<char const *> const &search_paths) {
    if (a.gen == generator::template_) {
        process_template(os, a.filename, search_paths);
    } else {
        auto obj = parse_file(a.filename, search_paths);
        if (obj) {
            switch (a.gen) {
            case generator::api_builder_cpp:
                generate_api_builder_cpp(os, *obj);
                break;
            case generator::api_builder_h:
                generate_api_builder_h(os, *obj);
                break;
            case generator::api_builder_hpp:
                generate_api_builder_hpp(os, *obj);
                break;
            case generator::enum_cpp:
                generate_enum_cpp(os, *obj);
                break;
            case generator::enum_h:
                generate_enum_h(os, *obj);
                break;
            case generator::enum_hpp:
                generate_enum_hpp(os, *obj);
                break;
            case generator::inst_cpp:
                generate_inst_cpp(os, *obj);
                break;
            case generator::inst_hpp:
                generate_inst_hpp(os, *obj);
                break;
            case generator::inst_forward_hpp:
                generate_inst_forward_hpp(os, *obj);
                break;
            case generator::inst_visit_hpp:
                generate_inst_visit_hpp(os, *obj);
                break;
            default:
                break;
            }
        } else {
            throw std::runtime_error("Could not parse " + a.filename);
        }
    }
}

void process_template(std::ostream &os, std::string const &filename,
                      std::vector<char const *> const &search_paths) {
    auto code_stream = open_file(find_path_to(filename, search_paths));
    for (std::string line; std::getline(code_stream, line);) {
        auto action = lex_omochi(line.size(), line.c_str());
        if (action) {
            please_do(os, *action, search_paths);
        } else {
            os << line << "\n";
        }
    }
}

} // namespace mochi
