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
 * @brief Get support level of SYCL device
 *
 * @param dev Device
 *
 * @return Support Level
 */
TINYTC_EXPORT auto get_support_level(sycl::device const &dev) -> support_level;

/**
 * @brief Query core info from SYCL runtime
 *
 * @param dev Device
 *
 * @return core info
 */
TINYTC_EXPORT auto make_core_info(sycl::device const &dev) -> core_info;

////////////////////////////
////////// Kernel //////////
////////////////////////////

/**
 * @brief Make SYCL kernel bundle from tinytc program
 *
 * @param ctx Context
 * @param dev Device
 * @param prg Program
 * @param core_features requested core features; must be 0 (default) or a combination of
 * tinytc_core_feature_flag_t
 *
 * @return SYCL kernel bundle
 */
TINYTC_EXPORT auto make_kernel_bundle(sycl::context const &ctx, sycl::device const &dev, prog prg,
                                      tinytc_core_feature_flags_t core_features = 0)
    -> sycl::kernel_bundle<sycl::bundle_state::executable>;

/**
 * @brief Make SYCL kernel bundle from tinytc binary
 *
 * @param ctx Context
 * @param dev Device
 * @param bin Binary
 *
 * @return SYCL kernel bundle
 */
TINYTC_EXPORT auto
make_kernel_bundle(sycl::context const &ctx, sycl::device const &dev,
                   binary const &bin) -> sycl::kernel_bundle<sycl::bundle_state::executable>;

/**
 * @brief Make SYCL kernel
 *
 * @param bundle Kernel bundle
 * @param name Kernel name
 *
 * @return SYCL kernel
 */
TINYTC_EXPORT auto make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &bundle,
                               char const *name) -> sycl::kernel;

/**
 * @brief Get work-group size
 *
 * @param krnl Kernel
 *
 * @return Work-group size
 */
TINYTC_EXPORT auto get_group_size(sycl::kernel const &krnl) -> sycl::range<3u>;

/**
 * @brief Convert group size to SYCL range
 *
 * @param howmany Group size
 * @param local_size Work-group size
 *
 * @return Global size
 */
TINYTC_EXPORT auto get_global_size(std::int64_t howmany,
                                   sycl::range<3u> const &local_size) -> sycl::range<3u>;

/**
 * @brief Get SYCL nd_range
 *
 * @param krnl Kernel
 * @param howmany Group size
 *
 * @return ND range
 */
TINYTC_EXPORT auto get_execution_range(sycl::kernel const &krnl,
                                       std::int64_t howmany) -> sycl::nd_range<3u>;

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Recipe handler for the SYCL runtime
 */
class TINYTC_EXPORT sycl_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    /**
     * @brief Launch recipe with submit call
     *
     * @param h Handler
     */
    void parallel_for(sycl::handler &h);
    /**
     * @brief Submit recipe to queue
     *
     * @param q Queue
     *
     * @return Event
     */
    auto submit(sycl::queue q) -> sycl::event;
    /**
     * @brief Submit recipe to queue
     *
     * @param q Queue
     * @param dep_event Event to wait on
     *
     * @return Event
     */
    auto submit(sycl::queue q, sycl::event const &dep_event) -> sycl::event;
    /**
     * @brief Submit recipe to queue
     *
     * @param q Queue
     * @param dep_events Events to wait on
     *
     * @return Event
     */
    auto submit(sycl::queue q, std::vector<sycl::event> const &dep_events) -> sycl::event;
};

/**
 * @brief Make recipe handler
 *
 * @param ctx Context
 * @param dev Device
 * @param rec Recipe
 *
 * @return SYCL recipe handler
 */
TINYTC_EXPORT auto make_recipe_handler(sycl::context const &ctx, sycl::device const &dev,
                                       recipe const &rec) -> sycl_recipe_handler;
/**
 * @brief Make recipe handler
 *
 * @param q Queue
 * @param rec Recipe
 *
 * @return SYCL recipe handler
 */
TINYTC_EXPORT auto make_recipe_handler(sycl::queue const &q,
                                       recipe const &rec) -> sycl_recipe_handler;

} // namespace tinytc

#endif // TINYTC_SYCL_20240403_HPP
