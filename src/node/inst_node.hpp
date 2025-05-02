// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "error.hpp"
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
    builtin,
    cast,
    compare,
    constant,
    cooperative_matrix_apply,
    cooperative_matrix_extract,
    cooperative_matrix_insert,
    cooperative_matrix_load,
    cooperative_matrix_mul_add,
    cooperative_matrix_prefetch,
    cooperative_matrix_scale,
    cooperative_matrix_store,
    expand,
    fuse,
    load,
    lifetime_stop,
    if_,
    math_unary_,
    parallel,
    size,
    subgroup_add,
    subgroup_broadcast,
    subgroup_max,
    subgroup_min,
    subview,
    store,
    yield,
    // blas a2
    blas_a2,
    axpby_blas_a2,
    cumsum_blas_a2,
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
    type_list<class alloca_inst, class axpby_inst, class arith_inst, class arith_unary_inst,
              class builtin_inst, class barrier_inst, class cast_inst, class compare_inst,
              class constant_inst, class cooperative_matrix_apply_inst,
              class cooperative_matrix_extract_inst, class cooperative_matrix_insert_inst,
              class cooperative_matrix_load_inst, class cooperative_matrix_mul_add_inst,
              class cooperative_matrix_prefetch_inst, class cooperative_matrix_scale_inst,
              class cooperative_matrix_store_inst, class cumsum_inst, class expand_inst,
              class fuse_inst, class load_inst, class lifetime_stop_inst, class gemm_inst,
              class gemv_inst, class ger_inst, class for_inst, class foreach_inst,
              class hadamard_inst, class if_inst, class math_unary_inst, class parallel_inst,
              class size_inst, class subgroup_add_inst, class subgroup_broadcast_inst,
              class subgroup_max_inst, class subgroup_min_inst, class subview_inst,
              class store_inst, class sum_inst, class yield_inst>;

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

    auto context() const -> tinytc_compiler_context_t;
    inline auto type_id() const -> tinytc::IK { return tid_; }

    inline auto attr() const noexcept -> tinytc_attr_t { return attr_; }
    inline void attr(tinytc_attr_t attr) noexcept { attr_ = attr; }

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

    void subs(tinytc_value_t old_value, tinytc_value_t new_value, bool recursive = true);

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

    auto kind() const -> tinytc::inst_execution_kind;

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
    tinytc_attr_t attr_ = nullptr;
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
                 tinytc_value_t B, bool atomic, location const &lc);

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
                 tinytc_value_t beta, tinytc_value_t C, bool atomic, location const &lc);

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

class loop_inst : public standard_inst<dynamic, dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK::loop && i.type_id() <= IK::last_loop;
    }
    inline loop_inst(IK tid, std::int64_t num_operands, std::int64_t num_results)
        : standard_inst{tid, num_operands, num_results} {}

    inline auto body() -> tinytc_region & { return child_region(0); }
    inline auto body() const -> tinytc_region const & { return child_region(0); }
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
    arith_inst(arithmetic op, tinytc_value_t a, tinytc_value_t b, tinytc_data_type_t ty,
               location const &lc = {});

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
    arith_unary_inst(arithmetic_unary op, tinytc_value_t a, tinytc_data_type_t ty,
                     location const &lc = {});

    inline arithmetic_unary operation() const { return operation_; }
    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }

  private:
    arithmetic_unary operation_;
};

class barrier_inst : public standard_inst<0, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::barrier; }
    inline barrier_inst(tinytc_address_spaces_t fence_flags, location const &lc = {})
        : standard_inst{IK::barrier}, fence_flags_(fence_flags) {
        loc(lc);
    }

    inline auto fence_flags() const -> tinytc_address_spaces_t { return fence_flags_; }
    inline auto fence_flags(tinytc_address_spaces_t fence_flags) { fence_flags_ = fence_flags; }
    inline auto has_fence(address_space as) const {
        return (fence_flags_ & static_cast<tinytc_address_spaces_t>(as)) > 0;
    }

  private:
    tinytc_address_spaces_t fence_flags_;
};

class builtin_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::builtin; }
    builtin_inst(builtin btype, tinytc_data_type_t ty, location const &lc = {});

    inline auto builtin_type() const -> builtin { return btype_; }

    auto kind() const -> tinytc::inst_execution_kind;

  private:
    builtin btype_;
};

class cast_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::cast; }
    enum op_number { op_a = 0 };
    cast_inst(tinytc_value_t a, tinytc_data_type_t to_ty, location const &lc = {});

    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
};

class compare_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::compare; }
    enum op_number { op_a = 0, op_b = 1 };
    compare_inst(cmp_condition cond, tinytc_value_t a, tinytc_value_t b, tinytc_data_type_t ty,
                 location const &lc = {});

    inline cmp_condition cond() const { return cond_; }
    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
    inline auto b() -> tinytc_value & { return op(op_b); }
    inline auto b() const -> tinytc_value const & { return op(op_b); }

  private:
    cmp_condition cond_;
};

class constant_inst : public standard_inst<0, 1> {
  public:
    using value_type = std::variant<bool, std::int64_t, double, std::complex<double>>;

    inline static bool classof(inst_node const &i) { return i.type_id() == IK::constant; }
    constant_inst(value_type const &value, tinytc_data_type_t ty, location const &lc = {});

    auto value() const -> value_type const & { return value_; }
    auto is_zero() const -> bool;
    auto is_identity() const -> bool;

  private:
    value_type value_;
};

class cooperative_matrix_apply_inst : public standard_inst<1, 1, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_apply;
    }
    cooperative_matrix_apply_inst(tinytc_value_t a, tinytc_data_type_t ty,
                                  location const &loc = {});

    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }

    inline auto body() -> tinytc_region & { return child_region(0); }
    inline auto body() const -> tinytc_region const & { return child_region(0); }
    inline auto row() -> tinytc_value & { return body().param(0); }
    inline auto row() const -> tinytc_value const & { return body().param(0); }
    inline auto col() -> tinytc_value & { return body().param(1); }
    inline auto col() const -> tinytc_value const & { return body().param(1); }
    inline auto val() -> tinytc_value & { return body().param(2); }
    inline auto val() const -> tinytc_value const & { return body().param(2); }
};

class cooperative_matrix_extract_inst : public standard_inst<1, 1, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_extract;
    }
    cooperative_matrix_extract_inst(tinytc_value_t mat, std::int64_t index, tinytc_data_type_t ty,
                                    location const &loc = {});

    inline auto mat() -> tinytc_value & { return op(0); }
    inline auto mat() const -> tinytc_value const & { return op(0); }
    inline auto index() const -> std::int64_t { return index_; }

  private:
    std::int64_t index_;
};

class cooperative_matrix_insert_inst : public standard_inst<2, 1, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_insert;
    }
    cooperative_matrix_insert_inst(tinytc_value_t val, tinytc_value_t mat, std::int64_t index,
                                   tinytc_data_type_t ty, location const &loc = {});

    inline auto val() -> tinytc_value & { return op(0); }
    inline auto val() const -> tinytc_value const & { return op(0); }
    inline auto mat() -> tinytc_value & { return op(1); }
    inline auto mat() const -> tinytc_value const & { return op(1); }
    inline auto index() const -> std::int64_t { return index_; }

  private:
    std::int64_t index_;
};

class cooperative_matrix_load_inst : public standard_inst<3, 1, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_load;
    }
    enum op_number { op_operand = 0, op_pos0 = 1, op_pos1 = 2 };
    cooperative_matrix_load_inst(transpose t, checked_flag flag, tinytc_value_t op0,
                                 tinytc_value_t p0, tinytc_value_t p1, tinytc_data_type_t to_ty,
                                 location const &lc = {});

    inline auto t() const -> transpose { return t_; }
    inline auto checked() const -> checked_flag { return flag_; }
    inline auto operand() -> tinytc_value & { return op(op_operand); }
    inline auto operand() const -> tinytc_value const & { return op(op_operand); }
    inline auto pos0() -> tinytc_value & { return op(op_pos0); }
    inline auto pos0() const -> tinytc_value const & { return op(op_pos0); }
    inline auto pos1() -> tinytc_value & { return op(op_pos1); }
    inline auto pos1() const -> tinytc_value const & { return op(op_pos1); }

    auto kind() const -> tinytc::inst_execution_kind;

  private:
    transpose t_;
    checked_flag flag_;
};

class cooperative_matrix_mul_add_inst : public standard_inst<3, 1, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_mul_add;
    }
    enum op_number { op_a = 0, op_b = 1, op_c = 2 };
    cooperative_matrix_mul_add_inst(tinytc_value_t a0, tinytc_value_t b0, tinytc_value_t c0,
                                    tinytc_data_type_t to_ty, location const &lc = {});

    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
    inline auto b() -> tinytc_value & { return op(op_b); }
    inline auto b() const -> tinytc_value const & { return op(op_b); }
    inline auto c() -> tinytc_value & { return op(op_c); }
    inline auto c() const -> tinytc_value const & { return op(op_c); }

    auto is_c_zero() const -> bool;
};

class cooperative_matrix_prefetch_inst : public standard_inst<3, 0, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_prefetch;
    }
    enum op_number { op_operand = 0, op_pos0 = 1, op_pos1 = 2 };
    cooperative_matrix_prefetch_inst(std::int32_t cache_level, tinytc_value_t op0,
                                     tinytc_value_t p0, tinytc_value_t p1, std::int32_t rows,
                                     std::int32_t cols, location const &lc = {});

    inline auto cache_level() const -> std::int32_t { return cache_level_; }
    inline auto operand() -> tinytc_value & { return op(op_operand); }
    inline auto operand() const -> tinytc_value const & { return op(op_operand); }
    inline auto pos0() -> tinytc_value & { return op(op_pos0); }
    inline auto pos0() const -> tinytc_value const & { return op(op_pos0); }
    inline auto pos1() -> tinytc_value & { return op(op_pos1); }
    inline auto pos1() const -> tinytc_value const & { return op(op_pos1); }
    inline auto rows() const -> std::int32_t { return rows_; }
    inline auto cols() const -> std::int32_t { return cols_; }

  private:
    std::int32_t cache_level_, rows_, cols_;
};

class cooperative_matrix_scale_inst : public standard_inst<2, 1, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_scale;
    }
    enum op_number { op_a = 0, op_b = 1 };
    cooperative_matrix_scale_inst(tinytc_value_t a0, tinytc_value_t b0, tinytc_data_type_t ty,
                                  location const &lc = {});

    inline auto a() -> tinytc_value & { return op(op_a); }
    inline auto a() const -> tinytc_value const & { return op(op_a); }
    inline auto b() -> tinytc_value & { return op(op_b); }
    inline auto b() const -> tinytc_value const & { return op(op_b); }
};

class cooperative_matrix_store_inst : public standard_inst<4, 0, 0> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() == IK::cooperative_matrix_store;
    }
    enum op_number { op_val = 0, op_operand = 1, op_pos0 = 2, op_pos1 = 3 };
    cooperative_matrix_store_inst(checked_flag cflag, store_flag sflag, tinytc_value_t val0,
                                  tinytc_value_t op0, tinytc_value_t p0, tinytc_value_t p1,
                                  location const &lc = {});

    inline auto checked() const -> checked_flag { return cflag_; }
    inline auto flag() const -> store_flag { return sflag_; }
    inline auto val() -> tinytc_value & { return op(op_val); }
    inline auto val() const -> tinytc_value const & { return op(op_val); }
    inline auto operand() -> tinytc_value & { return op(op_operand); }
    inline auto operand() const -> tinytc_value const & { return op(op_operand); }
    inline auto pos0() -> tinytc_value & { return op(op_pos0); }
    inline auto pos0() const -> tinytc_value const & { return op(op_pos0); }
    inline auto pos1() -> tinytc_value & { return op(op_pos1); }
    inline auto pos1() const -> tinytc_value const & { return op(op_pos1); }

  private:
    checked_flag cflag_;
    store_flag sflag_;
};

class cumsum_inst : public blas_a2_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::cumsum_blas_a2; }
    cumsum_inst(tinytc_value_t alpha, tinytc_value_t A, std::int64_t mode, tinytc_value_t beta,
                tinytc_value_t B, bool atomic = false, location const &lc = {});

    inline std::int64_t mode() const { return mode_; }

  private:
    std::int64_t mode_;
};

class expand_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::expand; }
    expand_inst(tinytc_value_t op, std::int64_t expanded_mode,
                array_view<std::int64_t> static_expand_shape,
                array_view<tinytc_value_t> expand_shape, tinytc_data_type_t ty,
                location const &lc = {});

    inline std::int64_t expanded_mode() const { return expanded_mode_; }
    inline auto static_expand_shape() const -> array_view<std::int64_t> {
        return static_expand_shape_;
    }

    inline auto operand() -> tinytc_value & { return op(0); }
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
    fuse_inst(tinytc_value_t op, std::int64_t from, std::int64_t to, tinytc_data_type_t ty,
              location const &lc = {});

    inline auto operand() -> tinytc_value & { return op(0); }
    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline std::int64_t from() const { return from_; }
    inline std::int64_t to() const { return to_; }

  private:
    std::int64_t from_, to_;
};

class load_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::load; }
    load_inst(tinytc_value_t op, array_view<tinytc_value_t> index_list, tinytc_data_type_t ty,
              location const &lc = {});

    inline auto operand() -> tinytc_value & { return op(0); }
    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline auto index_list() { return operands() | std::views::drop(1); }
    inline auto index_list() const { return operands() | std::views::drop(1); }
};

class lifetime_stop_inst : public standard_inst<1, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::lifetime_stop; }
    inline lifetime_stop_inst(tinytc_value_t obj, location const &lc = {})
        : standard_inst{IK::lifetime_stop} {
        op(0, obj);
        loc(lc);
    }

    inline auto object() -> tinytc_value & { return op(0); }
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
    enum op_number { op_from = 0, op_to = 1, op_step = 2 };
    for_inst(tinytc_data_type_t loop_var_type, tinytc_value_t from, tinytc_value_t to,
             tinytc_value_t step, array_view<tinytc_value_t> init_values,
             array_view<tinytc_data_type_t> return_types, location const &loc = {});

    inline auto from() -> tinytc_value & { return op(op_from); }
    inline auto from() const -> tinytc_value const & { return op(op_from); }
    inline auto to() -> tinytc_value & { return op(op_to); }
    inline auto to() const -> tinytc_value const & { return op(op_to); }
    inline auto has_step() const -> bool { return op_init() == 3; }
    inline auto step() -> tinytc_value & { return op(op_step); }
    inline auto step() const -> tinytc_value const & { return op(op_step); }
    inline auto loop_var() -> tinytc_value & { return body().param(0); }
    inline auto loop_var() const -> tinytc_value const & { return body().param(0); }
    inline auto iter_arg(std::int64_t no) -> tinytc_value & { return body().param(no + 1); }
    inline auto iter_arg(std::int64_t no) const -> tinytc_value const & {
        return body().param(no + 1);
    }
    inline auto iter_init(std::int64_t no) -> tinytc_value & { return op(op_init() + no); }
    inline auto iter_init(std::int64_t no) const -> tinytc_value const & {
        return op(op_init() + no);
    }
    inline auto iter_init() { return operands() | std::views::drop(op_init()); }
    inline auto iter_init() const { return operands() | std::views::drop(op_init()); }

  private:
    inline auto op_init() const -> std::int64_t { return num_operands() - num_results(); }
};

class foreach_inst : public loop_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::foreach_loop; }
    foreach_inst(tinytc_data_type_t loop_var_type, array_view<tinytc_value_t> from,
                 array_view<tinytc_value_t> to, location const &lc = {});

    inline auto dim() const -> std::int64_t { return num_operands() / 2; }
    inline auto loop_vars() { return body().params(); }
    inline auto loop_vars() const { return body().params(); }
    inline auto from() { return operands() | std::views::take(dim()); }
    inline auto from() const { return operands() | std::views::take(dim()); }
    inline auto to() { return operands() | std::views::drop(dim()); }
    inline auto to() const { return operands() | std::views::drop(dim()); }
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

    inline auto condition() -> tinytc_value & { return op(0); }
    inline auto condition() const -> tinytc_value const & { return op(0); }
    inline auto then() -> tinytc_region & { return child_region(child_region_then); }
    inline auto then() const -> tinytc_region const & { return child_region(child_region_then); }
    inline auto otherwise() -> tinytc_region & { return child_region(child_region_otherwise); }
    inline auto otherwise() const -> tinytc_region const & {
        return child_region(child_region_otherwise);
    }
    inline bool is_otherwise_empty() const { return otherwise().insts().empty(); }
};

class math_unary_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::math_unary_; }
    math_unary_inst(math_unary op, tinytc_value_t a, tinytc_data_type_t ty,
                    location const &lc = {});

    inline auto operation() const -> math_unary { return operation_; }
    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }

  private:
    math_unary operation_;
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
    size_inst(tinytc_value_t op, std::int64_t mode, tinytc_data_type_t ty, location const &lc = {});

    inline auto operand() -> tinytc_value & { return op(0); }
    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }

  private:
    std::int64_t mode_;
};

class subgroup_add_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_add; }
    subgroup_add_inst(group_operation operation, tinytc_value_t a, tinytc_data_type_t ty,
                      location const &lc = {});

    inline auto operation() const -> group_operation { return operation_; }
    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }

  private:
    group_operation operation_;
};

class subgroup_broadcast_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_broadcast; }
    subgroup_broadcast_inst(tinytc_value_t a, tinytc_value_t idx, tinytc_data_type_t ty,
                            location const &lc = {});

    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }
    inline auto idx() -> tinytc_value & { return op(1); }
    inline auto idx() const -> tinytc_value const & { return op(1); }
};

class subgroup_max_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_max; }
    subgroup_max_inst(group_operation operation, tinytc_value_t a, tinytc_data_type_t ty,
                      location const &lc = {});

    inline auto operation() const -> group_operation { return operation_; }
    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }

  private:
    group_operation operation_;
};

class subgroup_min_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subgroup_min; }
    subgroup_min_inst(group_operation operation, tinytc_value_t a, tinytc_data_type_t ty,
                      location const &lc = {});

    inline auto operation() const -> group_operation { return operation_; }
    inline auto a() -> tinytc_value & { return op(0); }
    inline auto a() const -> tinytc_value const & { return op(0); }

  private:
    group_operation operation_;
};

class subview_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK::subview; }
    subview_inst(tinytc_value_t op, array_view<std::int64_t> static_offsets,
                 array_view<std::int64_t> static_sizes, array_view<tinytc_value_t> offsets,
                 array_view<tinytc_value_t> sizes, tinytc_data_type_t ty, location const &lc = {});

    inline auto static_offsets() const -> array_view<std::int64_t> { return static_offsets_; }
    inline auto static_sizes() const -> array_view<std::int64_t> { return static_sizes_; }

    inline auto operand() -> tinytc_value & { return op(0); }
    inline auto operand() const -> tinytc_value const & { return op(0); }
    inline auto offsets() {
        return operands() | std::views::drop(1) | std::views::take(num_dyn_offsets_);
    }
    inline auto offsets() const {
        return operands() | std::views::drop(1) | std::views::take(num_dyn_offsets_);
    }
    inline auto sizes() { return operands() | std::views::drop(1 + num_dyn_offsets_); }
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
    inline auto val() -> tinytc_value & { return op(op_val); }
    inline auto val() const -> tinytc_value const & { return op(op_val); }
    inline auto operand() -> tinytc_value & { return op(op_operand); }
    inline auto operand() const -> tinytc_value const & { return op(op_operand); }
    inline auto index_list() { return operands() | std::views::drop(2); }
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
    yield_inst(array_view<tinytc_value_t> vals, location const &lc = {});
};

} // namespace tinytc

#endif // INST_NODE_20230327_HPP
