// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>

namespace tinytc {

//! Instruction classification
enum class inst_execution_kind {
    mixed,      ///< mixed instruction on uniform or varying data
    collective, ///< collective instruction on uniform data, distributed among work-items
    spmd        ///< SPMD instruction on varying data
};

using result_iterator = std::reverse_iterator<tinytc_value_t>;
using result_range = iterator_range_wrapper<result_iterator>;

using op_iterator =
    tinytc::indirect_random_access_iterator<tinytc::use *, tinytc::indirection_kind_get>;
using op_range = tinytc::iterator_range_wrapper<op_iterator>;
static_assert(std::random_access_iterator<op_iterator>);
static_assert(std::ranges::random_access_range<op_range>);

using region_range = iterator_range_wrapper<tinytc_region_t>;

struct inst_layout {
    std::int32_t num_results;
    std::int32_t num_operands;
    std::uint32_t sizeof_properties;
    std::int32_t num_child_regions;
};

enum class IK;

} // namespace tinytc

struct alignas(8) tinytc_inst : tinytc::ilist_node_with_parent<tinytc_inst, tinytc_region> {
  public:
    static auto create(tinytc::IK tid, tinytc::inst_layout layout, tinytc_location const &lc)
        -> tinytc_inst_t;
    static void destroy(tinytc_inst_t in);

    auto context() -> tinytc_compiler_context_t;
    inline auto type_id() const -> tinytc::IK { return tid_; }

    inline auto attr() const noexcept -> tinytc_attr_t { return attr_; }
    inline void attr(tinytc_attr_t attr) noexcept { attr_ = attr; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    // Iterator over operands
    inline auto op_begin() -> tinytc::op_iterator { return {use_ptr(0)}; }
    inline auto op_end() -> tinytc::op_iterator { return {use_ptr(layout_.num_operands)}; }
    inline auto operands() -> tinytc::op_range { return {op_begin(), op_end()}; }
    inline auto op(std::size_t pos) -> tinytc_value & { return *use_ptr(pos)->get(); }
    inline void op(std::size_t pos, tinytc_value_t val) { *use_ptr(pos) = val; }
    inline auto get_use(std::size_t pos) -> tinytc::use & { return *use_ptr(pos); }
    inline auto num_operands() const -> std::int32_t { return layout_.num_operands; }

    void subs(tinytc_value_t old_value, tinytc_value_t new_value, bool recursive = true);

    // Iterator over results
    inline auto result_begin() -> tinytc::result_iterator {
        return tinytc::result_iterator(reinterpret_cast<tinytc_value_t>(this));
    }
    inline auto result_end() -> tinytc::result_iterator {
        return tinytc::result_iterator(reinterpret_cast<tinytc_value_t>(this) -
                                       layout_.num_results);
    }
    inline auto results() -> tinytc::result_range { return {result_begin(), result_end()}; }
    inline auto result(std::size_t pos) -> tinytc_value & { return *result_ptr(pos); }
    inline auto num_results() const -> std::int32_t { return layout_.num_results; }

    // Properties
    inline auto props() -> void * { return use_ptr(layout_.num_operands); }

    // Iterator over regions
    inline auto child_regions_begin() -> tinytc_region_t { return child_region_ptr(0); }
    inline auto child_regions_end() -> tinytc_region_t {
        return child_region_ptr(layout_.num_child_regions);
    }
    inline auto child_regions() -> tinytc::region_range {
        return tinytc::region_range{child_regions_begin(), child_regions_end()};
    }
    auto child_region(std::size_t pos) -> tinytc_region & { return *child_region_ptr(pos); }
    auto num_child_regions() const -> std::int32_t { return layout_.num_child_regions; }

    auto kind() -> tinytc::inst_execution_kind;

    auto layout() const -> tinytc::inst_layout const & { return layout_; }

  private:
    inline tinytc_inst(tinytc::IK tid, tinytc::inst_layout layout, tinytc_location const &lc)
        : tid_(tid), layout_(layout), loc_{lc} {}
    ~tinytc_inst();

    tinytc_inst(tinytc_inst const &other) = delete;
    tinytc_inst(tinytc_inst &&other) = delete;
    tinytc_inst &operator=(tinytc_inst const &other) = delete;
    tinytc_inst &operator=(tinytc_inst &&other) = delete;

    inline auto result_ptr(std::int32_t no) -> tinytc_value_t {
        return reinterpret_cast<tinytc_value_t>(this) - ++no;
    }
    inline auto use_ptr(std::int32_t no) -> tinytc::use * {
        return reinterpret_cast<tinytc::use *>(this + 1) + no;
    }
    inline auto child_region_ptr(std::int32_t no) -> tinytc_region_t {
        std::uint8_t *props_end = static_cast<std::uint8_t *>(props()) + layout_.sizeof_properties;
        return reinterpret_cast<tinytc_region_t>(props_end) + no;
    }

    tinytc::IK tid_;
    tinytc::inst_layout layout_;
    tinytc::location loc_;
    tinytc_attr_t attr_ = nullptr;
};

namespace tinytc {

using inst_node = ::tinytc_inst;

} // namespace tinytc

#endif // INST_NODE_20230327_HPP
