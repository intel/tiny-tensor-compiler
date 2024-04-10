// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/prog.hpp"
#include "ir/node/program_node.hpp"
#include "tinytc/ir/func.hpp"

#include <memory>
#include <new>
#include <utility>

namespace tinytc {

prog::prog(func fun) : prog(std::make_shared<program>(std::vector<func>{std::move(fun)})) {}

prog::prog(std::vector<func> funs) : prog(std::make_shared<program>(std::move(funs))) {}

} // namespace tinytc

/*extern "C" {

tinytc_status_t tinytc_prog_create(tinytc_prog_t *program, uint32_t function_count,
                                   tinytc_func_t *functions) {
    if (program == nullptr) {
        return tinytc_invalid_arguments;
    }

    auto uprog = std::unique_ptr<tinytc_program>{};
    auto try {
        auto funcs = std::vector<tinytc_func_t>();
        uprog = std::make_unique<tinytc::program>();
        auto *program = new tinytc::program();
    } catch (std::bad_alloc const& {
        return tinytc_bad_alloc;
    } catch (...) {
        return tinytc_runtime_error;
    }

    inline program(std::vector<func> decls) : decls_(std::move(decls)) {}
    inline std::vector<func> &declarations() {
        return decls_; }

    try {
        auto ef = get_engine_factory(kind, get_default_runtime(kind));
        VERROR_ENGINE(ef != nullptr, invalid_arguments, VERBOSE_INVALID_ENGINE_KIND,
                      dnnl_engine_kind2str(kind));
        VERROR_ENGINE(index < ef->count(), invalid_arguments, VERBOSE_INVALID_ENGINE_IDX,
                      ef->count(), dnnl_engine_kind2str(kind), index);

        const status_t engine_status = ef->engine_create(engine, index);

        if (engine_status != success) {
            VERROR(common, runtime, VERBOSE_ENGINE_CREATION_FAIL, dnnl_engine_kind2str(kind),
                   index);
        }

        return engine_status;
    } catch (...) {
        VERROR(common, runtime, VERBOSE_INVALID_DEVICE_ENV, dnnl_engine_kind2str(kind), index);
        return runtime_error;
    }

    auto *f{static_cast<libfoo *>(std::malloc(sizeof(libfoo_foo)))};
    if (nullptr == f) {
        return nullptr;
    }
    try {
        // Construct the C++ object inside the allocated memory.
        // You can avoid separating allocation and construction in this case,
        // I keep them separated because we will talk about this later.
        ::new (&f->foo) InternalFoo{};
        return f;
    } catch (...) // We can't let exceptions cross ABI boundaries
    {
        std::free(f);
        return nullptr;
    };
    std::make_shared<program>(std::vector<func>{std::move(fun)});
}
    tinytc_status_t tinytc_prog_destroy(tinytc_prog_t program) {}
    }*/
