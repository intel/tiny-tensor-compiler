// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "omochi.hpp"
#include "codegen.hpp"
#include "parser.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace mochi {

auto to_string(generator g) -> char const * {
    switch (g) {
    case generator::inst_header:
        return "inst_header";
    case generator::inst_cpp:
        return "inst_cpp";
    case generator::inst_visit_header:
        return "inst_visit_header";
    case generator::template_:
        return "template";
    case generator::NUM_GENERATORS:
        break;
    }
    return "unknown";
}

auto find_path_to(std::string const &filename, std::vector<char const *> const &search_paths)
    -> fs::path {
    auto path = fs::path{filename};
    for (auto &s : search_paths) {
        auto candidate_path = fs::path{s} / path;
        if (fs::exists(candidate_path)) {
            return candidate_path;
        }
    }
    return path;
}

auto open_file(fs::path const &p) -> std::ifstream {
    auto code_stream = std::ifstream(p, std::ios_base::in);
    if (!code_stream.good()) {
        auto err = std::ostringstream{} << "Could not open " << p << " for reading.";
        throw std::runtime_error(std::move(err).str());
    }
    return code_stream;
}

void please_do(std::ostream &os, action const &a, std::vector<char const *> const &search_paths) {
    if (a.gen == generator::template_) {
        process_template(os, a.filename, search_paths);
    } else {
        auto code_stream = open_file(find_path_to(a.filename, search_paths));
        auto code = std::string(std::istreambuf_iterator<char>{code_stream}, {});

        auto obj = parse_file(code.size(), code.c_str(), a.filename.c_str());
        if (obj) {
            switch (a.gen) {
            case generator::inst_header:
                generate_inst_header(os, *obj);
                break;
            case generator::inst_cpp:
                generate_inst_cpp(os, *obj);
                break;
            case generator::inst_visit_header:
                generate_inst_visit_header(os, *obj);
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
