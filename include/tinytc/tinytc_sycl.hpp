// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_SYCL_20240403_HPP
#define TINYTC_SYCL_20240403_HPP

#include "tinytc.hpp"
#include "tinytc/export.h"
#include "tinytc_cl.hpp"
#include "tinytc_ze.hpp"

#include <cstdint>
#include <sycl/sycl.hpp>
#include <utility>

namespace tinytc {

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from SYCL runtime
 *
 * @param dev [in] device handle
 *
 * @return core info
 */
TINYTC_EXPORT auto make_core_info(sycl::device const &dev) -> core_info;

////////////////////////////
////////// Kernel //////////
////////////////////////////

TINYTC_EXPORT auto make_kernel_bundle(sycl::context const &ctx, sycl::device const &dev,
                                      binary const &bin)
    -> sycl::kernel_bundle<sycl::bundle_state::executable>;

TINYTC_EXPORT auto make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &bundle,
                               char const *name) -> sycl::kernel;

TINYTC_EXPORT auto get_group_size(sycl::kernel const &krnl) -> sycl::range<3u>;

TINYTC_EXPORT auto get_global_size(std::uint32_t howmany, sycl::range<3u> const &local_size)
    -> sycl::range<3u>;

TINYTC_EXPORT auto get_execution_range(sycl::kernel const &krnl, std::uint32_t howmany)
    -> sycl::nd_range<3u>;

////////////////////////////
////////// Recipe //////////
////////////////////////////

class TINYTC_EXPORT sycl_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    void parallel_for(sycl::handler &h);
    auto submit(sycl::queue q) -> sycl::event;
    auto submit(sycl::queue q, sycl::event const &dep_event) -> sycl::event;
    auto submit(sycl::queue q, std::vector<sycl::event> const &dep_events) -> sycl::event;
};

TINYTC_EXPORT auto make_recipe_handler(sycl::context const &ctx, sycl::device const &dev,
                                       recipe const &rec) -> sycl_recipe_handler;
TINYTC_EXPORT auto make_recipe_handler(sycl::queue const &q, recipe const &rec)
    -> sycl_recipe_handler;

} // namespace tinytc

#endif // TINYTC_SYCL_20240403_HPP
