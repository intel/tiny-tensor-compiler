// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_RECIPE_HANDLER_20240423_HPP
#define CL_RECIPE_HANDLER_20240423_HPP

#include "../recipe.hpp"
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_cl.hpp>
#include <tinytc/types.h>

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <array>
#include <cstdint>
#include <vector>

namespace tinytc {

struct cl_recipe_handler : ::tinytc_recipe_handler {
  public:
    //! Signature of clSetKernelArgMemPointerINTEL function
    using clSetKernelArgMemPointerINTEL_t = cl_int (*)(cl_kernel kernel, cl_uint arg_index,
                                                       const void *arg_value);

    cl_recipe_handler(recipe rec, cl_context context, cl_device_id device);

    void active_kernel(std::uint32_t kernel_num) override;
    void arg(std::uint32_t arg_index, std::size_t arg_size, const void *arg_value) override;
    void mem_arg(std::uint32_t arg_index, tinytc_mem_t const &mem) override;
    void howmany(std::uint32_t num) override;

    auto kernel() -> cl_kernel;
    auto local_size() const -> std::array<std::size_t, 3u> const &;
    inline auto global_size() const -> std::array<std::size_t, 3u> const & { return global_size_; }

  private:
    shared_handle<cl_program> module_;
    std::vector<std::array<std::size_t, 3u>> local_size_;
    std::vector<shared_handle<cl_kernel>> kernels_;
    std::uint32_t active_kernel_ = 0;
    std::array<std::size_t, 3u> global_size_;
    clSetKernelArgMemPointerINTEL_t clSetKernelArgMemPointerINTEL_ = nullptr;
};

} // namespace tinytc

#endif // CL_RECIPE_HANDLER_20240423_HPP
