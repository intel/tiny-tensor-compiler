// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/ir/value.hpp"

#include <clir/virtual_type_list.hpp>

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc::ir::internal {

class TINYTC_EXPORT inst_node
    : public clir::virtual_type_list<
          class alloca_inst, class axpby_inst, class barrier_inst, class binary_op_inst,
          class cast_inst, class compare_inst, class expand_inst, class fuse_inst, class load_inst,
          class group_id_inst, class group_size_inst, class lifetime_stop_inst, class gemm_inst,
          class gemv_inst, class ger_inst, class for_inst, class foreach_inst, class hadamard_inst,
          class if_inst, class neg_inst, class size_inst, class subview_inst, class store_inst,
          class sum_inst, class yield_inst> {
  public:
    inline location const &loc() const { return loc_; }
    inline void loc(location const &loc) { loc_ = loc; }

    virtual value result() = 0;
    virtual auto results() -> std::vector<value> { return {}; }
    virtual inst_kind kind() const = 0;

  private:
    location loc_;
};

class TINYTC_EXPORT scalar_inst : public inst_node {};

class TINYTC_EXPORT blas_a2_inst : public inst_node {
  public:
    blas_a2_inst(value alpha, value A, value beta, value B, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline value &alpha() { return alpha_; }
    inline value &A() { return A_; }
    inline value &beta() { return beta_; }
    inline value &B() { return B_; }
    inline value result() override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  protected:
    value alpha_, A_, beta_, B_;
    bool atomic_;
};

class TINYTC_EXPORT blas_a3_inst : public inst_node {
  public:
    blas_a3_inst(value alpha, value A, value B, value beta, value C, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline value &alpha() { return alpha_; }
    inline value &A() { return A_; }
    inline value &B() { return B_; }
    inline value &beta() { return beta_; }
    inline value &C() { return C_; }
    inline value result() override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  protected:
    value alpha_, A_, B_, beta_, C_;
    bool atomic_;
};

class TINYTC_EXPORT loop_inst : public inst_node {
  public:
    loop_inst(value loop_var, value from, value to, region body, location const &loc = {});
    loop_inst(value loop_var, value from, value to, value step, region body,
              location const &loc = {});
    inline value &loop_var() { return loop_var_; }
    inline value &from() { return from_; }
    inline value &to() { return to_; }
    inline value &step() { return step_; }
    inline region &body() { return body_; }
    inline value result() override { return value{}; }

  private:
    value loop_var_, from_, to_, step_;
    region body_;
};

class TINYTC_EXPORT alloca_inst : public clir::visitable<alloca_inst, inst_node> {
  public:
    alloca_inst(data_type ty, location const &loc = {});

    inline value result() override { return result_; }
    inline std::int64_t stack_ptr() const { return stack_ptr_; }
    inline void stack_ptr(std::int64_t ptr) { stack_ptr_ = ptr; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  private:
    value result_;
    std::int64_t stack_ptr_;
};

class TINYTC_EXPORT axpby_inst : public clir::visitable<axpby_inst, blas_a2_inst> {
  public:
    using super = clir::visitable<axpby_inst, blas_a2_inst>;
    axpby_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
               location const &lc = {});

    inline transpose tA() { return tA_; }

  private:
    transpose tA_;
};

class TINYTC_EXPORT barrier_inst : public clir::visitable<barrier_inst, inst_node> {
  public:
    inline value result() override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }
};

class TINYTC_EXPORT binary_op_inst : public clir::visitable<binary_op_inst, scalar_inst> {
  public:
    binary_op_inst(binary_op op, value a, value b, location const &lc = {});

    inline binary_op op() { return op_; }
    inline value &a() { return a_; }
    inline value &b() { return b_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    binary_op op_;
    value a_, b_, result_;
};

class TINYTC_EXPORT cast_inst : public clir::visitable<cast_inst, scalar_inst> {
  public:
    cast_inst(value a, scalar_type to_ty, location const &lc = {});
    inline value &a() { return a_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value a_, result_;
};

class TINYTC_EXPORT compare_inst : public clir::visitable<compare_inst, scalar_inst> {
  public:
    compare_inst(cmp_condition cond, value a, value b, location const &lc = {});

    inline cmp_condition cond() { return cond_; }
    inline value &a() { return a_; }
    inline value &b() { return b_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    cmp_condition cond_;
    value a_, b_, result_;
};

class TINYTC_EXPORT expand_inst : public clir::visitable<expand_inst, inst_node> {
  public:
    expand_inst(value op, std::int64_t mode, std::vector<value> expand_shape,
                location const &lc = {});

    inline value &operand() { return op_; }
    inline std::int64_t mode() const { return mode_; }
    inline auto &expand_shape() { return expand_shape_; }
    inline value &expand_shape(std::int64_t i) { return expand_shape_[i]; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value op_, result_;
    std::int64_t mode_;
    std::vector<value> expand_shape_;
};

class TINYTC_EXPORT fuse_inst : public clir::visitable<fuse_inst, inst_node> {
  public:
    fuse_inst(value op, std::int64_t from, std::int64_t to, location const &lc = {});

    inline value &operand() { return op_; }
    inline std::int64_t from() const { return from_; }
    inline std::int64_t to() const { return to_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value op_, result_;
    std::int64_t from_, to_;
};

class TINYTC_EXPORT load_inst : public clir::visitable<load_inst, inst_node> {
  public:
    load_inst(value op, std::vector<value> index_list, location const &lc = {});

    inline value &operand() { return op_; }
    inline std::vector<value> &index_list() { return index_list_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value op_;
    std::vector<value> index_list_;
    value result_;
};

class TINYTC_EXPORT group_id_inst : public clir::visitable<group_id_inst, scalar_inst> {
  public:
    inline group_id_inst() : result_{data_type(scalar_type::index)} {}
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value result_;
};

class TINYTC_EXPORT group_size_inst : public clir::visitable<group_size_inst, scalar_inst> {
  public:
    inline group_size_inst() : result_{data_type(scalar_type::index)} {}
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value result_;
};

class TINYTC_EXPORT lifetime_stop_inst : public clir::visitable<lifetime_stop_inst, inst_node> {
  public:
    inline lifetime_stop_inst(value obj) : obj_(std::move(obj)) {}
    inline value &object() { return obj_; }
    inline value result() override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  private:
    value obj_;
};

class TINYTC_EXPORT gemm_inst : public clir::visitable<gemm_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<gemm_inst, blas_a3_inst>;
    gemm_inst(transpose tA, transpose tB, value alpha, value A, value B, value beta, value C,
              bool atomic = false, location const &lc = {});

    inline transpose tA() { return tA_; }
    inline transpose tB() { return tB_; }

  private:
    transpose tA_, tB_;
};

class TINYTC_EXPORT gemv_inst : public clir::visitable<gemv_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<gemv_inst, blas_a3_inst>;
    gemv_inst(transpose tA, value alpha, value A, value B, value beta, value C, bool atomic = false,
              location const &lc = {});

    inline transpose tA() { return tA_; }

  private:
    transpose tA_;
};

class TINYTC_EXPORT ger_inst : public clir::visitable<ger_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<ger_inst, blas_a3_inst>;
    ger_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
             location const &lc = {});
};

class TINYTC_EXPORT for_inst : public clir::visitable<for_inst, loop_inst> {
  public:
    using super = clir::visitable<for_inst, loop_inst>;
    using super::super;
    inline inst_kind kind() const override { return inst_kind::replicated; }
};

class TINYTC_EXPORT foreach_inst : public clir::visitable<foreach_inst, loop_inst> {
  public:
    using super = clir::visitable<foreach_inst, loop_inst>;
    inline foreach_inst(value loop_var, value from, value to, region body, location const &loc = {})
        : super(std::move(loop_var), std::move(from), std::move(to), std::move(body), loc) {}
    inline inst_kind kind() const override { return inst_kind::collective; }
};

class TINYTC_EXPORT hadamard_inst : public clir::visitable<hadamard_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<hadamard_inst, blas_a3_inst>;
    hadamard_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
                  location const &lc = {});
};

class TINYTC_EXPORT if_inst : public clir::visitable<if_inst, inst_node> {
  public:
    if_inst(value condition, region then, region otherwise = nullptr,
            std::vector<scalar_type> const &return_types = {});
    inline value &condition() { return condition_; }
    inline region &then() { return then_; }
    inline region &otherwise() { return otherwise_; }
    inline value result() override { return results_.size() > 0 ? results_.front() : value{}; }
    inline auto results() -> std::vector<value> override { return results_; }
    inline auto results_ref() -> std::vector<value> & { return results_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value condition_;
    region then_, otherwise_;
    std::vector<value> results_;
};

class TINYTC_EXPORT neg_inst : public clir::visitable<neg_inst, scalar_inst> {
  public:
    neg_inst(value a, location const &lc = {});

    inline value &a() { return a_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value a_, result_;
};

class TINYTC_EXPORT size_inst : public clir::visitable<size_inst, inst_node> {
  public:
    size_inst(value op, std::int64_t mode, location const &lc = {});

    inline value &operand() { return op_; }
    inline std::int64_t mode() const { return mode_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value op_, result_;
    std::int64_t mode_;
};

class TINYTC_EXPORT subview_inst : public clir::visitable<subview_inst, inst_node> {
  public:
    subview_inst(value op, std::vector<slice> slices, location const &lc = {});

    inline std::vector<slice> &slices() { return slices_; }
    inline value &operand() { return op_; }
    inline value result() override { return result_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value op_;
    std::vector<slice> slices_;
    value result_;
};

class TINYTC_EXPORT store_inst : public clir::visitable<store_inst, inst_node> {
  public:
    store_inst(value val, value op, std::vector<value> index_list, location const &lc = {});

    inline value &val() { return val_; }
    inline value &operand() { return op_; }
    inline std::vector<value> &index_list() { return index_list_; }
    inline value result() override { return {}; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    value val_, op_;
    std::vector<value> index_list_;
};

class TINYTC_EXPORT sum_inst : public clir::visitable<sum_inst, blas_a2_inst> {
  public:
    using super = clir::visitable<sum_inst, blas_a2_inst>;
    sum_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
             location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class TINYTC_EXPORT yield_inst : public clir::visitable<yield_inst, inst_node> {
  public:
    inline yield_inst(std::vector<value> vals) : vals_(std::move(vals)) {}
    inline value result() override { return value{}; }
    inline auto vals() -> std::vector<value> & { return vals_; }
    inline inst_kind kind() const override { return inst_kind::replicated; }

  private:
    std::vector<value> vals_;
};

} // namespace tinytc::ir::internal

#endif // INST_NODE_20230327_HPP
