// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "error.hpp"
#include "node/program_node.hpp"
// IWYU pragma: begin_keep
#include "pass/dump_cfg.hpp"
#include "pass/dump_def_use.hpp"
#include "pass/dump_gcd.hpp"
#include "pass/dump_ir.hpp"
#include "pass/dump_matrix_ext.hpp"
// IWYU pragma: end_keep
#include "pass/alignment_propagation.hpp"
#include "pass/check_ir.hpp"
#include "pass/constant_propagation.hpp"
#include "pass/convert_to_spirv.hpp"
#include "pass/dead_code_elimination.hpp"
#include "pass/insert_barrier.hpp"
#include "pass/insert_lifetime_stop.hpp"
#include "pass/lower_coopmatrix.hpp"
#include "pass/lower_foreach.hpp"
#include "pass/lower_linalg.hpp"
#include "pass/stack.hpp"
#include "pass/work_group_size.hpp"
#include "passes.hpp"
#include "spv/pass/assemble.hpp"
#include "spv/pass/assign_ids.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <cstring>
#include <iostream> // IWYU pragma: keep
#include <utility>

using namespace tinytc;

namespace tinytc {

template <typename PassT> struct optflag_setter {
    PassT &pass;
    tinytc_compiler_context_t ctx;

    template <typename... Flags> void operator()(Flags &&...flags) {
        (pass.set_opt_flag(flags, ctx->opt_flag(flags)), ...);
    }
};

void apply_default_optimization_pipeline(tinytc_prog_t prg, const_tinytc_core_info_t info) {
    auto ctx = prg->context();
    const auto opt_level = ctx->opt_level();

    // passes
    auto cpp = constant_propagation_pass{};
    optflag_setter{cpp, ctx}(tinytc::optflag::unsafe_fp_math);

    run_function_pass(check_ir_pass{}, *prg);

    if (opt_level >= 1) {
        // We run constant propagation + dead code elimination early to capture dead allocas
        // (later on they are maybe "in use" due to the lifetime_stop instruction)
        run_function_pass(cpp, *prg);
        run_function_pass(dead_code_elimination_pass{}, *prg);
        run_function_pass(alignment_propagation_pass{}, *prg);
    }

    run_function_pass(insert_lifetime_stop_pass{}, *prg);
    run_function_pass(set_stack_ptr_pass{}, *prg);
    run_function_pass(insert_barrier_pass{}, *prg);
    run_function_pass(work_group_size_pass{info}, *prg);

    run_function_pass(lower_linalg_pass{info}, *prg);
    run_function_pass(lower_foreach_pass{info}, *prg);
    if (opt_level >= 1) {
        run_function_pass(cpp, *prg);
        run_function_pass(dead_code_elimination_pass{}, *prg);
        run_function_pass(alignment_propagation_pass{}, *prg);
    }
    run_function_pass(lower_coopmatrix_pass{info}, *prg);

    run_function_pass(check_ir_pass{}, *prg);
}

} // namespace tinytc

extern "C" {

tinytc_status_t tinytc_run_function_pass(char const *pass_name, tinytc_prog_t prg,
                                         const_tinytc_core_info_t info) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
#define FUNCTION_PASS(NAME, CREATE_PASS, ...)                                                      \
    if (strcmp(NAME, pass_name) == 0) {                                                            \
        auto pass = CREATE_PASS;                                                                   \
        optflag_setter{pass, prg->context()}(__VA_ARGS__);                                         \
        return run_function_pass(std::move(pass), *prg);                                           \
    }
#define FUNCTION_PASS_WITH_INFO(NAME, CREATE_PASS)                                                 \
    if (strcmp(NAME, pass_name) == 0) {                                                            \
        return run_function_pass(CREATE_PASS(info), *prg);                                         \
    }
#include "passes.def"
#undef FUNCTION_PASS
#undef FUNCTION_PASS_WITH_INFO
            throw status::unknown_pass_name;
        },
        prg->context());
}

tinytc_status_t tinytc_list_function_passes(uint32_t *names_size, char const *const **names) {
    if (names_size == nullptr || names == nullptr) {
        return tinytc_status_invalid_arguments;
    }
#define FUNCTION_PASS(NAME, CREATE_PASS, ...) NAME,
#define FUNCTION_PASS_WITH_INFO(NAME, CREATE_PASS) NAME,
    static char const *const pass_names[] = {
#include "passes.def"
    };
#undef FUNCTION_PASS
#undef FUNCTION_PASS_WITH_INFO
    *names_size = sizeof(pass_names) / sizeof(char const *);
    *names = pass_names;

    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_compile_to_spirv(tinytc_spv_mod_t *mod, tinytc_prog_t prg,
                                             const_tinytc_core_info_t info) {
    if (mod == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            apply_default_optimization_pipeline(prg, info);

            *mod = convert_to_spirv_pass{info}.run_on_program(*prg).release();
            spv::id_assigner{}.run_on_module(**mod);
        },
        prg->context());
}

tinytc_status_t tinytc_prog_compile_to_spirv_and_assemble(tinytc_binary_t *bin, tinytc_prog_t prg,
                                                          const_tinytc_core_info_t info) {
    if (bin == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    tinytc_spv_mod_t mod;
    TINYTC_CHECK_STATUS(tinytc_prog_compile_to_spirv(&mod, prg, info));
    auto mod_ = spv_mod{mod}; // For clean-up
    TINYTC_CHECK_STATUS(tinytc_spirv_assemble(bin, mod_.get()));
    return tinytc_status_success;
}

tinytc_status_t tinytc_spirv_assemble(tinytc_binary_t *bin, const_tinytc_spv_mod_t mod) {
    if (bin == nullptr || mod == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *bin = spv::assembler{}.run_on_module(*mod).release(); });
}
}
