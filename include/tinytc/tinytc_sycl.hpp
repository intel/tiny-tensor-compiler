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
TINYTC_EXPORT auto create_core_info(sycl::device const &dev) -> shared_handle<tinytc_core_info_t>;

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
TINYTC_EXPORT auto create_kernel_bundle(sycl::context const &ctx, sycl::device const &dev,
                                        tinytc_prog_t prg,
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
TINYTC_EXPORT auto create_kernel_bundle(sycl::context const &ctx, sycl::device const &dev,
                                        const_tinytc_binary_t bin)
    -> sycl::kernel_bundle<sycl::bundle_state::executable>;

/**
 * @brief Make SYCL kernel
 *
 * @param bundle Kernel bundle
 * @param name Kernel name
 *
 * @return SYCL kernel
 */
TINYTC_EXPORT auto create_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &bundle,
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
 * **Important:** num_groups is in SYCL ZYX order, meaning that the range should contain
 * {num_groups_z, num_groups_y, num_groups_x}.
 *
 * @param num_groups Number of groups
 * @param local_size Work-group size
 *
 * @return Global size
 */
TINYTC_EXPORT auto get_global_size(sycl::range<3u> const &num_groups,
                                   sycl::range<3u> const &local_size) -> sycl::range<3u>;

/**
 * @brief Get SYCL nd_range
 *
 * **Important:** num_groups is in SYCL ZYX order, meaning that the range should contain
 * {num_groups_z, num_groups_y, num_groups_x}.
 *
 * @param krnl Kernel
 * @param num_groups Number of groups
 *
 * @return ND range
 */
TINYTC_EXPORT auto get_execution_range(sycl::kernel const &krnl, sycl::range<3u> const &num_groups)
    -> sycl::nd_range<3u>;

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Launch recipe with submit call
 *
 * @param handler recipe handler
 * @param cgh Handler
 */
TINYTC_EXPORT void parallel_for(tinytc_recipe_handler_t handler, sycl::handler &cgh);
/**
 * @brief Submit recipe to queue
 *
 * @param handler recipe handler
 * @param q Queue
 *
 * @return Event
 */
TINYTC_EXPORT auto submit(tinytc_recipe_handler_t handler, sycl::queue q) -> sycl::event;
/**
 * @brief Submit recipe to queue
 *
 * @param handler recipe handler
 * @param q Queue
 * @param dep_event Event to wait on
 *
 * @return Event
 */
TINYTC_EXPORT auto submit(tinytc_recipe_handler_t handler, sycl::queue q,
                          sycl::event const &dep_event) -> sycl::event;
/**
 * @brief Submit recipe to queue
 *
 * @param handler recipe handler
 * @param q Queue
 * @param dep_events Events to wait on
 *
 * @return Event
 */
TINYTC_EXPORT auto submit(tinytc_recipe_handler_t handler, sycl::queue q,
                          std::vector<sycl::event> const &dep_events) -> sycl::event;

/**
 * @brief Make recipe handler
 *
 * @param ctx Context
 * @param dev Device
 * @param rec Recipe
 *
 * @return SYCL recipe handler
 */
TINYTC_EXPORT auto create_recipe_handler(sycl::context const &ctx, sycl::device const &dev,
                                         tinytc_recipe_t rec)
    -> shared_handle<tinytc_recipe_handler_t>;
/**
 * @brief Make recipe handler
 *
 * @param q Queue
 * @param rec Recipe
 *
 * @return SYCL recipe handler
 */
TINYTC_EXPORT auto create_recipe_handler(sycl::queue const &q, tinytc_recipe_t rec)
    -> shared_handle<tinytc_recipe_handler_t>;

} // namespace tinytc

#endif // TINYTC_SYCL_20240403_HPP
