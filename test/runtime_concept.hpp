// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RUNTIME_CONCEPT_20241023_HPP
#define RUNTIME_CONCEPT_20241023_HPP

#include "tinytc/tinytc.hpp"

#include <cstddef>

namespace tinytc::test {

template <typename T>
concept test_runtime_gpu =
    requires(T rt, std::size_t bytes, typename T::mem_t buf, typename T::const_mem_t const_buf,
             void *dst, void const *src, int value, tinytc::recipe const &rec, tinytc::prog p,
             typename T::kernel_bundle_t const &bundle, char const *name,
             typename T::kernel_t &kernel, std::uint32_t arg_index, std::size_t arg_size,
             const void *arg_value, ::tinytc::mem_type type, std::int64_t howmany) {
        typename T::device_t;
        typename T::context_t;
        typename T::command_list_t;
        typename T::recipe_handler_t;
        typename T::kernel_bundle_t;
        typename T::kernel_t;
        typename T::mem_t;
        typename T::const_mem_t;
        { rt.create_buffer(bytes) } -> std::same_as<typename T::mem_t>;
        rt.free_buffer(buf);
        rt.fill_buffer(buf, value, bytes);
        rt.memcpy_h2d(buf, src, bytes);
        rt.memcpy_d2h(dst, const_buf, bytes);
        { rt.get_core_info() } -> std::same_as<tinytc::core_info>;
        { rt.get_device() } -> std::same_as<typename T::device_t>;
        { rt.get_context() } -> std::same_as<typename T::context_t>;
        { rt.get_command_list() } -> std::same_as<typename T::command_list_t>;
        { rt.get_recipe_handler(rec) } -> std::same_as<typename T::recipe_handler_t>;
        { rt.get_kernel_bundle(p) } -> std::same_as<typename T::kernel_bundle_t>;
        { rt.get_kernel(bundle, name) } -> std::same_as<typename T::kernel_t>;
        rt.set_arg(kernel, arg_index, arg_size, arg_value);
        rt.set_mem_arg(kernel, arg_index, arg_value, type);
        rt.submit(kernel, howmany);
        { rt.supports_fp64() } -> std::same_as<bool>;
        rt.synchronize();
    };

} // namespace tinytc::test

#endif // RUNTIME_CONCEPT_20241023_HPP
