// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/binary.hpp"
#include "error.hpp"
#include "ir/node/function_node.hpp"
#include "ir/node/program_node.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/internal/compiler_options.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/passes.hpp"
#include "tinytc/opencl_cc.hpp"
#include "tinytc/types.hpp"
#include "util.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>
#include <clir/visitor/codegen_opencl.hpp>
#include <clir/visitor/unique_names.hpp>

#include <algorithm>
#include <functional>
#include <sstream>
#include <string_view>
#include <utility>

using clir::visit;

namespace tinytc {

binary::binary(std::vector<std::uint8_t> data, bundle_format format,
               std::unordered_map<std::string, kernel_metadata> metadata_map,
               std::uint32_t core_features)
    : data_(std::move(data)), format_(format), metadata_(std::move(metadata_map)),
      core_features_(core_features) {}

auto optimize_and_make_binary(prog prog, bundle_format format, std::shared_ptr<core_info> info,
                              error_reporter_function err) -> std::shared_ptr<binary> {
    auto result = std::shared_ptr<binary>{nullptr};

    // passes
    if (!check_ir(prog, err)) {
        return nullptr;
    }
    try {
        insert_barriers(prog);
        insert_lifetime_stop_inst(prog);
        set_stack_ptrs(prog);
        set_work_group_size(prog, info);

        // Get work group sizes
        auto metadata = std::unordered_map<std::string, kernel_metadata>{};
        auto *prog_node = dynamic_cast<program *>(prog.get());
        if (prog_node == nullptr) {
            throw compilation_error(location{}, status::internal_compiler_error,
                                    "Expected program node");
        }
        for (auto &decl : prog_node->declarations()) {
            visit(overloaded{[&metadata](function &f) {
                                 auto const name = visit(
                                     overloaded{
                                         [](prototype &p) -> std::string_view { return p.name(); },
                                         [](auto &f) -> std::string {
                                             throw compilation_error(
                                                 f.loc(), status::internal_compiler_error,
                                                 "Expected prototype");
                                         }},
                                     *f.prototype());
                                 auto m = kernel_metadata{};
                                 m.subgroup_size = f.subgroup_size();
                                 m.work_group_size = f.work_group_size();
                                 metadata[std::string(name)] = m;
                             },
                             [](auto &) {}},
                  *decl);
        }

        // opencl
        auto ast = generate_opencl_ast(std::move(prog), info);
        clir::make_names_unique(ast);
        auto oss = std::ostringstream{};
        clir::generate_opencl(oss, ast);

        // Compile
        auto ext = internal::required_extensions(std::move(ast));
        auto compiler_options = internal::default_compiler_options;

        auto const core_features = info->core_features();
        if (core_features & static_cast<std::uint32_t>(core_feature_flag::large_register_file)) {
            compiler_options.push_back(internal::large_register_file_compiler_option_ze);
        }
        auto bin = compile_opencl_c(oss.str(), format, info->ip_version(), compiler_options, ext);
        result =
            std::make_shared<binary>(std::move(bin), format, std::move(metadata), core_features);
    } catch (compilation_error const &e) {
        err(e.loc(), e.what());
    }
    return result;
}

} // namespace tinytc
