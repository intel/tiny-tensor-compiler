// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "compiler_context.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/type_list.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class inst_execution_kind {
    mixed,      ///< mixed instruction on uniform or varying data
    collective, ///< collective instruction on uniform data, distributed among work-items
    spmd        ///< SPMD instruction on varying data
};

enum class IK {
    alloca,
    arith,
    arith_unary,
    barrier,
    cast,
    compare,
    constant,
    expand,
    fuse,
    load,
    group_id,
    group_size,
    lifetime_stop,
    if_,
    num_subgroups,
    parallel,
    size,
    subgroup_id,
    subgroup_local_id,
    subgroup_size,
    subview,
    store,
    yield,
    // blas a2
    blas_a2,
    axpby_blas_a2,
    sum_blas_a2,
    last_blas_a2,
    // blas a3
    blas_a3,
    gemm_blas_a3,
    gemv_blas_a3,
    ger_blas_a3,
    hadamard_blas_a3,
    last_blas_a3,
    // loop inst
    loop,
    for_loop,
    foreach_loop,
    last_loop
};
using inst_nodes =
    type_list<class alloca_inst, class axpby_inst, class barrier_inst, class arith_inst,
              class arith_unary_inst, class cast_inst, class compare_inst, class constant_inst,
              class expand_inst, class fuse_inst, class load_inst, class group_id_inst,
              class group_size_inst, class lifetime_stop_inst, class gemm_inst, class gemv_inst,
              class ger_inst, class for_inst, class foreach_inst, class hadamard_inst,
              class if_inst, class num_subgroups_inst, class parallel_inst, class size_inst,
              class subview_inst, class store_inst, class subgroup_id_inst,
              class subgroup_local_id_inst, class subgroup_size_inst, class sum_inst,
              class yield_inst>;

using result_range = iterator_range_wrapper<tinytc_value_t>;
using const_result_range = iterator_range_wrapper<const_tinytc_value_t>;

using region_range = iterator_range_wrapper<tinytc_region_t>;
using const_region_range = iterator_range_wrapper<const_tinytc_region_t>;

} // namespace tinytc

struct tinytc_inst : tinytc::ilist_node_with_parent<tinytc_inst, tinytc_region> {
  public:
    using leaves = tinytc::inst_nodes;

    using op_iterator =
        tinytc::indirect_random_access_iterator<tinytc::use *, tinytc::indirection_kind_get>;
    using const_op_iterator =
        tinytc::indirect_random_access_iterator<const tinytc::use *, tinytc::indirection_kind_get>;

    using op_range = tinytc::iterator_range_wrapper<op_iterator>;
    using const_op_range = tinytc::iterator_range_wrapper<const_op_iterator>;

    static_assert(std::random_access_iterator<op_iterator>);
    static_assert(std::random_access_iterator<const_op_iterator>);
    static_assert(std::ranges::random_access_range<op_range>);
    static_assert(std::ranges::random_access_range<const_op_range>);

    inline tinytc_inst(tinytc::IK tid) : tid_(tid) {}
    virtual ~tinytc_inst() = default;

    tinytc_inst(tinytc_inst const &other) = delete;
    tinytc_inst(tinytc_inst &&other) = delete;
    tinytc_inst &operator=(tinytc_inst const &other) = delete;
    tinytc_inst &operator=(tinytc_inst &&other) = delete;

    inline auto type_id() const -> tinytc::IK { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    // Iterator over operands
    inline auto op_begin() -> op_iterator { return {op_begin_}; }
    inline auto op_end() -> op_iterator { return {op_end_}; }
    inline auto operands() -> op_range { return {op_begin(), op_end()}; }
    inline auto op_begin() const -> const_op_iterator { return {op_begin_}; }
    inline auto op_end() const -> const_op_iterator { return {op_end_}; }
    inline auto operands() const -> const_op_range { return {op_begin(), op_end()}; }
    inline auto op(std::size_t pos) -> tinytc_value & { return *op_begin_[pos].get(); }
    inline auto op(std::size_t pos) const -> tinytc_value const & { return *op_begin_[pos].get(); }
    inline void op(std::size_t pos, tinytc_value_t val) { op_begin_[pos] = val; }
    inline auto get_use(std::size_t pos) -> tinytc::use & { return op_begin_[pos]; }
    inline auto get_use(std::size_t pos) const -> tinytc::use const & { return op_begin_[pos]; }
    inline auto num_operands() const -> std::int64_t { return op_end_ - op_begin_; }

    // Iterator over results
    inline auto result_begin() -> tinytc_value_t { return result_begin_; }
    inline auto result_end() -> tinytc_value_t { return result_end_; }
    inline auto results() -> tinytc::result_range { return {result_begin_, result_end_}; }
    inline auto result_begin() const -> const_tinytc_value_t { return result_begin_; }
    inline auto result_end() const -> const_tinytc_value_t { return result_end_; }
    inline auto results() const -> tinytc::const_result_range {
        return {result_begin_, result_end_};
    }
    inline auto result() const -> tinytc_value_t {
        return num_results() > 0 ? result_begin_ : nullptr;
    }
    inline auto result(std::size_t pos) -> tinytc_value & { return result_begin_[pos]; }
    inline auto result(std::size_t pos) const -> tinytc_value const & { return result_begin_[pos]; }
    inline auto num_results() const -> std::int64_t { return result_end_ - result_begin_; }

    // Iterator over regions
    inline auto child_regions_begin() -> tinytc_region_t { return child_regions_begin_; }
    inline auto child_regions_end() -> tinytc_region_t { return child_regions_end_; }
    inline auto child_regions() -> tinytc::region_range {
        return tinytc::region_range{child_regions_begin(), child_regions_end()};
    }
    inline auto child_regions_begin() const -> const_tinytc_region_t {
        return child_regions_begin_;
    }
    inline auto child_regions_end() const -> const_tinytc_region_t { return child_regions_end_; }
    inline auto child_regions() const -> tinytc::const_region_range {
        return tinytc::const_region_range{child_regions_begin(), child_regions_end()};
    }
    auto child_region(std::size_t pos) -> tinytc_region & { return child_regions_begin_[pos]; }
    auto child_region(std::size_t pos) const -> tinytc_region const & {
        return child_regions_begin_[pos];
    }
    auto num_child_regions() const -> std::int64_t {
        return child_regions_end_ - child_regions_begin_;
    }

    inline constexpr auto kind() const -> tinytc::inst_execution_kind {
        switch (type_id()) {
        case tinytc::IK::alloca:
        case tinytc::IK::barrier:
        case tinytc::IK::lifetime_stop:
        case tinytc::IK::foreach_loop:
        case tinytc::IK::parallel:
        case tinytc::IK::blas_a2:
        case tinytc::IK::axpby_blas_a2:
        case tinytc::IK::sum_blas_a2:
        case tinytc::IK::last_blas_a2:
        case tinytc::IK::blas_a3:
        case tinytc::IK::gemm_blas_a3:
        case tinytc::IK::gemv_blas_a3:
        case tinytc::IK::ger_blas_a3:
        case tinytc::IK::hadamard_blas_a3:
        case tinytc::IK::last_blas_a3:
            return tinytc::inst_execution_kind::collective;
        case tinytc::IK::arith:
        case tinytc::IK::arith_unary:
        case tinytc::IK::cast:
        case tinytc::IK::compare:
        case tinytc::IK::constant:
        case tinytc::IK::expand:
        case tinytc::IK::fuse:
        case tinytc::IK::load:
        case tinytc::IK::group_id:
        case tinytc::IK::group_size:
        case tinytc::IK::if_:
        case tinytc::IK::num_subgroups:
        case tinytc::IK::size:
        case tinytc::IK::subgroup_size:
        case tinytc::IK::subview:
        case tinytc::IK::store:
        case tinytc::IK::yield:
        case tinytc::IK::loop:
        case tinytc::IK::for_loop:
        case tinytc::IK::last_loop:
            return tinytc::inst_execution_kind::mixed;
        case tinytc::IK::subgroup_id:
        case tinytc::IK::subgroup_local_id:
            return tinytc::inst_execution_kind::spmd;
        };
        throw tinytc::internal_compiler_error();
    }

  protected:
    inline auto set_op_range(tinytc::use *begin, tinytc::use *end) noexcept {
        op_begin_ = begin;
        op_end_ = end;
    }
    inline auto set_result_range(tinytc_value_t begin, tinytc_value_t end) noexcept {
        result_begin_ = begin;
        result_end_ = end;
    }
    inline auto set_child_regions_range(tinytc_region_t begin, tinytc_region_t end) noexcept {
        child_regions_begin_ = begin;
        child_regions_end_ = end;
    }

  private:
    tinytc::IK tid_;
    tinytc::location loc_;
    tinytc::use *op_begin_ = nullptr, *op_end_ = nullptr;
    tinytc_value_t result_begin_ = nullptr, result_end_ = nullptr;
    tinytc_region_t child_regions_begin_ = nullptr, child_regions_end_ = nullptr;
};

namespace tinytc {

using inst_node = ::tinytc_inst;

template <typename T, std::int64_t NumObjects> class object_container {
  public:
    object_container(std::int64_t num_objects) {
        // Check that num_objects is not larger than container size
        // Smaller is ok too support optional arguments
        if (num_objects > NumObjects) {
            throw internal_compiler_error();
        }
    }
    auto get() -> T * {
        if constexpr (NumObjects == 0) {
            return nullptr;
        }
        return objs_.data();
    }

  private:
    std::array<T, NumObjects> objs_;
};

template <typename T> class object_container<T, dynamic> {
  public:
    object_container(std::int64_t num_objects) : objs_{std::make_unique<T[]>(num_objects)} {}

    auto get() -> T * { return objs_.get(); }

  private:
    std::unique_ptr<T[]> objs_;
};

template <std::int64_t NumOperands, std::int64_t NumResults, std::int64_t NumChildRegions = 0>
class standard_inst : public inst_node {
  public:
    standard_inst(IK tid, std::int64_t num_operands = NumOperands,
                  std::int64_t num_results = NumResults,
                  std::int64_t num_child_regions = NumChildRegions)
        : inst_node{tid}, ops_{num_operands}, results_{num_results},
          child_regions_{num_child_regions} {
        if (num_operands > 0) {
            auto *op_begin = ops_.get();
            set_op_range(op_begin, op_begin + num_operands);
            if constexpr (NumOperands != 0) {
                for (std::int64_t i = 0; i < num_operands; ++i) {
                    op_begin[i].owner(this);
                }
            }
        }
        if (num_results > 0) {
            auto *result_begin = results_.get();
            set_result_range(result_begin, result_begin + num_results);
        }
        if (num_child_regions > 0) {
            set_child_regions_range(child_regions_.get(), child_regions_.get() + num_child_regions);
        }
    }

  private:
    object_container<use, NumOperands> ops_;
    object_container<tinytc_value, NumResults> results_;
    object_container<tinytc_region, NumChildRegions> child_regions_;
};

class blas_a2_inst : public standard_inst<4, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK::blas_a2 && i.type_id() <= IK::last_blas_a2;
    }
    enum op_number { op_alpha = 0, op_A = 1, op_beta = 2, op_B = 3 };
    blas_a2_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t beta,
                 tinytc_value_t B, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() -> tinytc_value & { return op(op_alpha); }
    inline auto alpha() const -> tinytc_value const & { return op(op_alpha); }
    inline auto A() -> tinytc_value & { return op(op_A); }
    inline auto A() const -> tinytc_value const & { return op(op_A); }
    inline auto beta() -> tinytc_value & { return op(op_beta); }
    inline auto beta() const -> tinytc_value const & { return op(op_beta); }
    inline auto B() -> tinytc_value & { return op(op_B); }
    inline auto B() const -> tinytc_value const & { return op(op_B); }

  protected:
    bool atomic_;
};

class blas_a3_inst : public standard_inst<5, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK::blas_a3 && i.type_id() <= IK::last_blas_a3;
    }
    enum op_number { op_alpha = 0, op_A = 1, op_B = 2, op_beta = 3, op_C = 4 };
    blas_a3_inst(IK tid, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
                 tinytc_value_t beta, tinytc_value_t C, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() -> tinytc_value & { return op(op_alpha); }
    inline auto alpha() const -> tinytc_value const & { return op(op_alpha); }
    inline auto A() -> tinytc_value & { return op(op_A); }
    inline auto A() const -> tinytc_value const & { return op(op_A); }
    inline auto B() -> tinytc_value & { return op(op_B); }
    inline auto B() const -> tinytc_value const & { return op(op_B); }
    inline auto beta() -> tinytc_value & { return op(op_beta); }
    inline auto beta() const -> tinytc_value const & { return op(op_beta); }
    inline auto C() -> tinytc_value & { return op(op_C); }
    inline auto C() const -> tinytc_value const & { return op(op_C); }

  protected:
    bool atomic_;
};

class loop_inst : public standard_inst<3, 0, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK::loop && i.type_id() <= IK::last_loop;
    }
    enum op_number { op_from = 0, op_to = 1, op_step = 2 };
    loop_inst(IK tid, tinytc_value_t from, tinytc_value_t to, tinytc_value_t step,
              tinytc_data_type_t loop_var_type, location const &loc = {});
    inline auto from() const -> tinytc_value const & { return op(op_from); }
    inline auto to() const -> tinytc_value const & { return op(op_to); }
    inline auto has_step() const -> bool { return get_use(op_step).get() != nullptr; }
    inline auto step() const -> tinytc_value const & { return op(op_step); }
    inline auto body() -> tinytc_region & { return child_region(0); }
    inline auto body() const -> tinytc_region const & { return child_region(0); }
    inline auto loop_var() -> tinytc_value & { return body().param(0); }
    inline auto loop_var() const -> tinytc_value const & { return body().param(0); }
};

class alloca_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::alloca; }
    alloca_inst(tinytc_data_type_t ty, location const &loc = {});

    inline std::int64_t stack_ptr() const { return stack_ptr_; }
    inline void stack_ptr(std::int64_t ptr) { stack_ptr_ = ptr; }

  private:
    std::int64_t stack_ptr_;
};

class axpby_inst : public blas_a2_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::axpby_blas_a2; }
    axpby_inst(transpose tA, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t beta,
               tinytc_value_t B, bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class arith_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::arith; }
    enum op_number { op_a = 0, op_b = 1 };
    arith_inst(arithmetic op, tinytc_value_t a, tinytc_value_t b, location const &lc = {});

    inline arithmetic operation() const { return operation_; }
    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
    inline auto b() -> tinytc_value & { return op(op_b); }
    inline auto b() const -> tinytc_value const & { return op(op_b); }

  private:
    arithmetic operation_;
};

class arith_unary_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::arith_unary; }
    enum op_number { op_a = 0 };
    arith_unary_inst(arithmetic_unary op, tinytc_value_t a, location const &lc = {});

    inline arithmetic_unary operation() const { return operation_; }
    inline auto a() const -> tinytc_value const & { return op(op_a); }

  private:
    arithmetic_unary operation_;
};

class barrier_inst : public standard_inst<0, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::barrier; }
    inline barrier_inst(std::int32_t fence_flags, location const &lc = {})
        : standard_inst{IK::barrier}, fence_flags_(fence_flags) {
        loc(lc);
    }

    inline auto fence_flags() const -> std::int32_t { return fence_flags_; }
    inline auto fence_flags(std::int32_t fence_flags) { fence_flags_ = fence_flags; }
    inline auto has_fence(address_space as) const {
        return (fence_flags_ & static_cast<std::int32_t>(as)) > 0;
    }

  private:
    std::int32_t fence_flags_;
};

class cast_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::cast; }
    enum op_number { op_a = 0 };
    cast_inst(tinytc_value_t a, tinytc_data_type_t to_ty, location const &lc = {});
    inline auto a() const -> tinytc_value const & { return op(op_a); }
};

class compare_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::compare; }
    enum op_number { op_a = 0, op_b = 1 };
    compare_inst(cmp_condition cond, tinytc_value_t a, tinytc_value_t b, location const &lc = {});

    inline cmp_condition cond() const { return cond_; }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
    inline auto b() const -> tinytc_value const & { return op(op_b); }

  private:
    cmp_condition cond_;
};

class constant_inst : public standard_inst<0, 1> {
  public:
    using value_type = std::variant<std::int64_t, double, std::complex<double>>;

    inline static bool classof(inst_node const &i) { return i.type_id() == IK::constant; }
    constant_inst(value_type const &value, tinytc_data_type_t ty, location const &lc = {});

    auto value() const -> value_type const & { return value_; }
    auto is_zero() const -> bool;
    auto is_identity() const -> bool;

  private:
    value_type value_;
};

class expand_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::expand; }
    expand_inst(tinytc_value_t op, std::int64_t expanded_mode,
                array_view<std::int64_t> static_expand_shape,
                array_view<tinytc_value_t> expand_shape, location const &lc = {});

    inline std::int64_t expanded_mode() const { return expanded_mode_; }
    inline auto static_expand_shape() const -> array_view<std::int64_t> {
        return static_expand_shape_;
    }

    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline auto expand_shape() { return operands() | std::views::drop(1); }
    inline auto expand_shape() const { return operands() | std::views::drop(1); }
    inline auto expand_shape(std::int64_t i) const -> tinytc_value const & { return op(i + 1); }

  private:
    std::int64_t expanded_mode_;
    std::vector<std::int64_t> static_expand_shape_;
};

class fuse_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::fuse; }
    fuse_inst(tinytc_value_t op, std::int64_t from, std::int64_t to, location const &lc = {});

    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline std::int64_t from() const { return from_; }
    inline std::int64_t to() const { return to_; }

  private:
    std::int64_t from_, to_;
};

class load_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::load; }
    load_inst(tinytc_value_t op, array_view<tinytc_value_t> index_list, location const &lc = {});

    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline auto index_list() const { return operands() | std::views::drop(1); }
};

class group_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::group_id; }
    inline group_id_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::group_id} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::index), this, lc};
    }
};

class group_size_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::group_size; }
    inline group_size_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::group_size} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::index), this, lc};
    }
};

class lifetime_stop_inst : public standard_inst<1, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::lifetime_stop; }
    inline lifetime_stop_inst(tinytc_value_t obj) : standard_inst{IK::lifetime_stop} { op(0, obj); }
    inline auto object() const -> tinytc_value const & { return op(0); }
};

class gemm_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::gemm_blas_a3; }
    gemm_inst(transpose tA, transpose tB, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
              tinytc_value_t beta, tinytc_value_t C, bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }
    inline transpose tB() const { return tB_; }

  private:
    transpose tA_, tB_;
};

class gemv_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::gemv_blas_a3; }
    gemv_inst(transpose tA, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B,
              tinytc_value_t beta, tinytc_value_t C, bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class ger_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::ger_blas_a3; }
    ger_inst(tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B, tinytc_value_t beta,
             tinytc_value_t C, bool atomic = false, location const &lc = {});
};

class for_inst : public loop_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::for_loop; }
    inline for_inst(tinytc_value_t from, tinytc_value_t to, tinytc_data_type_t loop_var_type,
                    location const &loc = {})
        : for_inst{std::move(from), std::move(to), {}, loop_var_type, loc} {}
    inline for_inst(tinytc_value_t from, tinytc_value_t to, tinytc_value_t step,
                    tinytc_data_type_t loop_var_type, location const &loc = {})
        : loop_inst{IK::for_loop,    std::move(from), std::move(to),
                    std::move(step), loop_var_type,   loc} {}
};

class foreach_inst : public loop_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::foreach_loop; }
    foreach_inst(tinytc_value_t from, tinytc_value_t to, tinytc_data_type_t loop_var_type,
                 location const &loc = {});
};

class hadamard_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::hadamard_blas_a3; }
    hadamard_inst(tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B, tinytc_value_t beta,
                  tinytc_value_t C, bool atomic = false, location const &lc = {});
};

class if_inst : public standard_inst<1, dynamic, 2> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::if_; }
    enum child_region_number { child_region_then = 0, child_region_otherwise = 1 };
    if_inst(tinytc_value_t condition, array_view<tinytc_data_type_t> return_types = {},
            location const &lc = {});
    inline auto condition() const -> tinytc_value const & { return op(0); }
    inline auto then() -> tinytc_region & { return child_region(child_region_then); }
    inline auto then() const -> tinytc_region const & { return child_region(child_region_then); }
    inline auto otherwise() -> tinytc_region & { return child_region(child_region_otherwise); }
    inline auto otherwise() const -> tinytc_region const & {
        return child_region(child_region_otherwise);
    }
    inline bool is_otherwise_empty() const { return otherwise().insts().empty(); }
};

class num_subgroups_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::num_subgroups; }
    inline num_subgroups_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::num_subgroups} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::i32), this, lc};
    }
};

class parallel_inst : public standard_inst<0, 0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::parallel; }
    parallel_inst(location const &lc = {});

    inline auto body() -> tinytc_region & { return child_region(0); }
    inline auto body() const -> tinytc_region const & { return child_region(0); }
};

class size_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::size; }
    size_inst(tinytc_value_t op, std::int64_t mode, location const &lc = {});

    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }

  private:
    std::int64_t mode_;
};

class subgroup_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_id; }
    inline subgroup_id_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::subgroup_id} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::i32), this, lc};
    }
};

class subgroup_local_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_local_id; }
    inline subgroup_local_id_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::subgroup_local_id} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::i32), this, lc};
    }
};

class subgroup_size_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_size; }
    inline subgroup_size_inst(tinytc_compiler_context_t ctx, location const &lc = {})
        : standard_inst{IK::subgroup_size} {
        loc(lc);
        result(0) = value_node{scalar_data_type::get(ctx, scalar_type::i32), this, lc};
    }
};

class subview_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subview; }
    subview_inst(tinytc_value_t op, array_view<std::int64_t> static_offsets,
                 array_view<std::int64_t> static_sizes, array_view<tinytc_value_t> offsets,
                 array_view<tinytc_value_t> sizes, location const &lc = {});

    inline auto static_offsets() const -> array_view<std::int64_t> { return static_offsets_; }
    inline auto static_sizes() const -> array_view<std::int64_t> { return static_sizes_; }

    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline auto offsets() const {
        return operands() | std::views::drop(1) | std::views::take(num_dyn_offsets_);
    }
    inline auto sizes() const { return operands() | std::views::drop(1 + num_dyn_offsets_); }

  private:
    std::vector<std::int64_t> static_offsets_, static_sizes_;
    std::int32_t num_dyn_offsets_;
};

class store_inst : public standard_inst<dynamic, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::store; }
    enum op_number { op_val = 0, op_operand = 1 };
    store_inst(store_flag flag, tinytc_value_t val, tinytc_value_t op,
               array_view<tinytc_value_t> index_list, location const &lc = {});

    inline auto flag() const -> store_flag { return flag_; }
    inline auto val() const -> tinytc_value const & { return op(op_val); }
    inline auto operand() const -> tinytc_value const & { return op(op_operand); }
    inline auto index_list() const { return operands() | std::views::drop(2); }

  private:
    store_flag flag_;
};

class sum_inst : public blas_a2_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::sum_blas_a2; }
    sum_inst(transpose tA, tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t beta,
             tinytc_value_t B, bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class yield_inst : public standard_inst<dynamic, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::yield; }
    inline yield_inst(array_view<tinytc_value_t> vals, location const &lc = {})
        : standard_inst{IK::yield, static_cast<std::int64_t>(vals.size())} {
        loc(lc);
        for (std::size_t i = 0; i < vals.size(); ++i) {
            op(i, vals[i]);
        }
    }
};

} // namespace tinytc

#endif // INST_NODE_20230327_HPP
