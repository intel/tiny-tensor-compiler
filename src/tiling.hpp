// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TILING_20240306_HPP
#define TILING_20240306_HPP

#include "device_info.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <typeindex>
#include <vector>

namespace tinytc {

enum class scalar_type;

//! Size of 2D subgroup grid
class local_tiling : public std::array<std::uint32_t, 2> {
  public:
    //! Number of subgroups working on the row blocks (M-loop)
    inline auto m_tiles() const { return operator[](0); }
    //! Number of subgroups working on the column blocks (N-loop)
    inline auto n_tiles() const { return operator[](1); }
    /**
     * @brief Calculate work-group size
     *
     * The work-group size is given by {m_tiles() * subgroup_size, n_tiles}.
     * The returned values can be used in the work_group_size function attribute
     */
    inline auto work_group_size(std::uint32_t subgroup_size) const
        -> std::array<std::uint32_t, 2u> {
        return {subgroup_size * m_tiles(), n_tiles()};
    }
    //! Compute number of work items
    inline auto number_of_work_items(std::uint32_t subgroup_size) const -> std::uint32_t {
        auto const wgs = work_group_size(subgroup_size);
        return wgs[0] * wgs[1];
    }
};

/**
 * Matrix shape and element type
 */
struct blas_shape {
    scalar_type ty;                                         ///< Element type
    std::array<std::int64_t, 2u> shape;                     ///< Matrix shape
    auto operator==(blas_shape const &other) const -> bool; ///< equal
    auto operator!=(blas_shape const &other) const -> bool; ///< not equal
};

/**
 * @brief Suggest a subgroup size based on blas sizes
 *
 * @param shapes Shapes that occur in kernel
 * @param available_subgroup_sizes Subgroup sizes supported by device; must be in ascending order
 */
auto suggest_subgroup_size(std::vector<blas_shape> const &shapes,
                           std::vector<std::uint32_t> const &available_subgroup_sizes)
    -> std::uint32_t;

/**
 * @brief Suggest a local tiling based on blas size
 *
 * @param bshape Shape that occurs in kernel
 * @param core_cfg Core configuration for subgroup size
 *
 * @return
 */
auto suggest_local_tiling(blas_shape const &bshape, core_config const &core_cfg) -> local_tiling;
/**
 * @brief Suggest a local tiling based on blas sizes
 *
 * @param shapes Shapes that occur in kernel
 * @param core_cfg Core configuration for subgroup size
 *
 * @return
 */
auto suggest_local_tiling(std::vector<blas_shape> const &shapes, core_config const &core_cfg)
    -> local_tiling;

/**
 * @brief Suggest both, subgroup size and tiling, based on blas sizes.
 *
 * @param shapes Shapes that occur in kernel
 * @param dev_info Device info
 *
 * @return {subgroup size, local tiling}
 */
auto suggest_subgroup_size_and_tiling(std::vector<blas_shape> const &shapes,
                                      ::tinytc_core_info const &dev_info)
    -> std::tuple<std::uint32_t, local_tiling>;

} // namespace tinytc

//! Hash function for blas_shape
template <> struct std::hash<tinytc::blas_shape> {
    std::size_t operator()(tinytc::blas_shape const &x) const; ///< Compute hash
};

#endif // TILING_20240306_HPP
