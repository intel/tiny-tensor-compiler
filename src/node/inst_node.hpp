// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "error.hpp"
#include "reference_counted.hpp"
#include "support/type_list.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class inst_execution_kind {
    mixed,      ///< mixed instruction on uniform or varying data
    collective, ///< collective instruction on uniform data, distributed among work-items
    spmd        ///< SPMD instruction on varying data

};

using inst_nodes =
    type_list<class alloca_inst, class axpby_inst, class barrier_inst, class arith_inst,
              class arith_unary_inst, class cast_inst, class compare_inst, class expand_inst,
              class fuse_inst, class load_inst, class group_id_inst, class group_size_inst,
              class lifetime_stop_inst, class gemm_inst, class gemv_inst, class ger_inst,
              class for_inst, class foreach_inst, class hadamard_inst, class if_inst,
              class num_subgroups_inst, class parallel_inst, class size_inst, class subview_inst,
              class store_inst, class subgroup_id_inst, class subgroup_local_id_inst,
              class subgroup_size_inst, class sum_inst, class yield_inst>;

using op_range = iterator_range_wrapper<value *>;
using const_op_range = iterator_range_wrapper<value const *>;
using result_range = iterator_range_wrapper<value *>;
using const_result_range = iterator_range_wrapper<value const *>;

} // namespace tinytc

struct tinytc_inst : tinytc::reference_counted {
  public:
    enum inst_kind {
        IK_alloca,
        IK_arith,
        IK_arith_unary,
        IK_barrier,
        IK_cast,
        IK_compare,
        IK_expand,
        IK_fuse,
        IK_load,
        IK_group_id,
        IK_group_size,
        IK_lifetime_stop,
        IK_if,
        IK_num_subgroups,
        IK_parallel,
        IK_size,
        IK_subgroup_id,
        IK_subgroup_local_id,
        IK_subgroup_size,
        IK_subview,
        IK_store,
        IK_yield,
        // blas a2
        IK_blas_a2,
        IK_axpby_blas_a2,
        IK_sum_blas_a2,
        IK_last_blas_a2,
        // blas a3
        IK_blas_a3,
        IK_gemm_blas_a3,
        IK_gemv_blas_a3,
        IK_ger_blas_a3,
        IK_hadamard_blas_a3,
        IK_last_blas_a3,
        // loop inst
        IK_loop,
        IK_for_loop,
        IK_foreach_loop,
        IK_last_loop
    };
    using leaves = tinytc::inst_nodes;

    inline tinytc_inst(std::int64_t tid) : tid_(tid), op_begin_(nullptr), op_end_(nullptr) {}
    inline virtual ~tinytc_inst() {}
    inline auto type_id() const -> std::int64_t { return tid_; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    // Iterator over operands
    inline auto op_begin() -> tinytc::value * { return op_begin_; }
    inline auto op_end() -> tinytc::value * { return op_end_; }
    inline auto operands() -> tinytc::op_range { return tinytc::op_range{op_begin_, op_end_}; }
    inline auto op_begin() const -> tinytc::value const * { return op_begin_; }
    inline auto op_end() const -> tinytc::value const * { return op_end_; }
    inline auto operands() const -> tinytc::const_op_range {
        return tinytc::const_op_range{op_begin_, op_end_};
    }
    inline auto op(std::size_t pos) -> tinytc::value & { return op_begin_[pos]; }
    inline auto op(std::size_t pos) const -> tinytc::value const & { return op_begin_[pos]; }
    inline auto num_operands() const -> std::int64_t { return op_end_ - op_begin_; }

    virtual tinytc::value result() const = 0;
    inline virtual auto results() const -> std::vector<tinytc::value> {
        if (auto r = result(); r) {
            return {std::move(r)};
        }
        return {};
    }
    inline virtual auto num_results() const -> std::size_t { return result() ? 1u : 0u; }
    virtual tinytc::inst_execution_kind kind() const = 0;

  protected:
    inline auto op_range(tinytc::value *begin, tinytc::value *end) {
        op_begin_ = begin;
        op_end_ = end;
    }

  private:
    std::int64_t tid_;
    tinytc::location loc_;
    tinytc::value *op_begin_, *op_end_;
};

namespace tinytc {

using inst_node = ::tinytc_inst;

template <std::int64_t NumValues> class value_container {
  public:
    value_container(std::int64_t num_values) {
        if (num_values != NumValues) {
            throw internal_compiler_error();
        }
    }
    inline auto get() -> tinytc::value * {
        if constexpr (NumValues == 0) {
            return nullptr;
        }
        return ops_.data();
    }

  private:
    std::array<value, NumValues> ops_;
};

template <> class value_container<dynamic> {
  public:
    value_container(std::int64_t num_values) : ops_{std::make_unique<value[]>(num_values)} {}

    auto get() -> tinytc::value * { return ops_.get(); }

  private:
    std::unique_ptr<value[]> ops_;
};

template <std::int64_t NumOperands, std::int64_t NumResults>
class standard_inst : public inst_node {
  public:
    standard_inst(std::int64_t tid, std::int64_t num_operands = NumOperands,
                  std::int64_t num_results = NumResults)
        : inst_node{tid}, ops_{num_operands}, results_{num_results} {
        if (num_operands > 0) {
            op_range(ops_.get(), ops_.get() + num_operands);
        }
    }

  private:
    value_container<NumOperands> ops_;
    value_container<NumResults> results_;
};

class blas_a2_inst : public standard_inst<4, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK_blas_a2 && i.type_id() <= IK_last_blas_a2;
    }
    enum op_number { op_alpha = 0, op_A = 1, op_beta = 2, op_B = 3 };
    blas_a2_inst(std::int64_t tid, value alpha, value A, value beta, value B, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() const -> value const & { return op(op_alpha); }
    inline auto A() const -> value const & { return op(op_A); }
    inline auto beta() const -> value const & { return op(op_beta); }
    inline auto B() const -> value const & { return op(op_B); }
    inline value result() const override { return value{}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }

  protected:
    bool atomic_;
};

class blas_a3_inst : public standard_inst<5, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK_blas_a3 && i.type_id() <= IK_last_blas_a3;
    }
    enum op_number { op_alpha = 0, op_A = 1, op_B = 2, op_beta = 3, op_C = 4 };
    blas_a3_inst(std::int64_t tid, value alpha, value A, value B, value beta, value C, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() const -> value const & { return op(op_alpha); }
    inline auto A() const -> value const & { return op(op_A); }
    inline auto B() const -> value const & { return op(op_B); }
    inline auto beta() const -> value const & { return op(op_beta); }
    inline auto C() const -> value const & { return op(op_C); }
    inline value result() const override { return value{}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }

  protected:
    bool atomic_;
};

class loop_inst : public standard_inst<4, 1> {
  public:
    inline static bool classof(inst_node const &i) {
        return i.type_id() >= IK_loop && i.type_id() <= IK_last_loop;
    }
    enum op_number { op_loop_var = 0, op_from = 1, op_to = 2, op_step = 3 };
    loop_inst(std::int64_t tid, value loop_var, value from, value to, value step, region body,
              location const &loc = {});
    inline auto loop_var() const -> value const & { return op(op_loop_var); }
    inline auto from() const -> value const & { return op(op_from); }
    inline auto to() const -> value const & { return op(op_to); }
    inline auto step() const -> value const & { return op(op_step); }
    inline auto body() const -> region const & { return body_; }
    inline value result() const override { return value{}; }

  private:
    region body_;
};

class alloca_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_alloca; }
    alloca_inst(data_type ty, location const &loc = {});

    inline value result() const override { return result_; }
    inline std::int64_t stack_ptr() const { return stack_ptr_; }
    inline void stack_ptr(std::int64_t ptr) { stack_ptr_ = ptr; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }

  private:
    value result_;
    std::int64_t stack_ptr_;
};

class axpby_inst : public blas_a2_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_axpby_blas_a2; }
    axpby_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
               location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class arith_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_arith; }
    enum op_number { op_a = 0, op_b = 1 };
    arith_inst(arithmetic op, value a, value b, location const &lc = {});

    inline arithmetic operation() const { return operation_; }
    inline auto a() const -> value const & { return op(op_a); }
    inline auto b() const -> value const & { return op(op_b); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    arithmetic operation_;
    value result_;
};

class arith_unary_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_arith_unary; }
    enum op_number { op_a = 0 };
    arith_unary_inst(arithmetic_unary op, value a, location const &lc = {});

    inline arithmetic_unary operation() const { return operation_; }
    inline auto a() const -> value const & { return op(op_a); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    arithmetic_unary operation_;
    value result_;
};

class barrier_inst : public standard_inst<0, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_barrier; }
    inline barrier_inst() : standard_inst{IK_barrier} {}

    inline value result() const override { return value{}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }
};

class cast_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_cast; }
    enum op_number { op_a = 0 };
    cast_inst(value a, scalar_type to_ty, location const &lc = {});
    inline auto a() const -> value const & { return op(op_a); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class compare_inst : public standard_inst<2, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_compare; }
    enum op_number { op_a = 0, op_b = 1 };
    compare_inst(cmp_condition cond, value a, value b, location const &lc = {});

    inline cmp_condition cond() const { return cond_; }
    inline auto a() const -> value const & { return op(op_a); }
    inline auto b() const -> value const & { return op(op_b); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    cmp_condition cond_;
    value result_;
};

class expand_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_expand; }
    expand_inst(value op, std::int64_t mode, std::vector<value> const &expand_shape,
                location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }
    inline auto expand_shape() { return operands() | std::views::drop(1); }
    inline auto expand_shape() const { return operands() | std::views::drop(1); }
    inline auto expand_shape(std::int64_t i) const -> value const & { return op(i + 1); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
    std::int64_t mode_;
};

class fuse_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_fuse; }
    fuse_inst(value op, std::int64_t from, std::int64_t to, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t from() const { return from_; }
    inline std::int64_t to() const { return to_; }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
    std::int64_t from_, to_;
};

class load_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_load; }
    load_inst(value op, std::vector<value> const &index_list, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline auto index_list() const { return operands() | std::views::drop(1); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class group_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_group_id; }
    inline group_id_inst(location const &lc = {})
        : standard_inst{IK_group_id}, result_{make_value(scalar_type::index)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class group_size_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_group_size; }
    inline group_size_inst(location const &lc = {})
        : standard_inst{IK_group_size}, result_{make_value(scalar_type::index)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class lifetime_stop_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_lifetime_stop; }
    inline lifetime_stop_inst(value obj) : standard_inst{IK_lifetime_stop} {
        op(0) = std::move(obj);
    }
    inline auto object() const -> value const & { return op(0); }
    inline value result() const override { return value{}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }
};

class gemm_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_gemm_blas_a3; }
    gemm_inst(transpose tA, transpose tB, value alpha, value A, value B, value beta, value C,
              bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }
    inline transpose tB() const { return tB_; }

  private:
    transpose tA_, tB_;
};

class gemv_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_gemv_blas_a3; }
    gemv_inst(transpose tA, value alpha, value A, value B, value beta, value C, bool atomic = false,
              location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class ger_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_ger_blas_a3; }
    ger_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
             location const &lc = {});
};

class for_inst : public loop_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_for_loop; }
    inline for_inst(value loop_var, value from, value to, region body, location const &loc = {})
        : loop_inst{
              IK_for_loop, std::move(loop_var), std::move(from), std::move(to), {}, std::move(body),
              loc} {}
    inline for_inst(value loop_var, value from, value to, value step, region body,
                    location const &loc = {})
        : loop_inst{IK_for_loop,
                    std::move(loop_var),
                    std::move(from),
                    std::move(to),
                    std::move(step),
                    std::move(body),
                    loc} {}
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }
};

class foreach_inst : public loop_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_foreach_loop; }
    inline foreach_inst(value loop_var, value from, value to, region body, location const &loc = {})
        : loop_inst{IK_foreach_loop,
                    std::move(loop_var),
                    std::move(from),
                    std::move(to),
                    {},
                    std::move(body),
                    loc} {}
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }
};

class hadamard_inst : public blas_a3_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_hadamard_blas_a3; }
    hadamard_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
                  location const &lc = {});
};

class if_inst : public standard_inst<1, dynamic> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_if; }
    if_inst(value condition, region then, region otherwise = {},
            std::vector<scalar_type> const &return_types = {}, location const &lc = {});
    inline auto condition() const -> value const & { return op(0); }
    inline auto then() const -> region const & { return then_; }
    inline auto otherwise() const -> region const & { return otherwise_; }
    inline value result() const override {
        return results_.size() > 0 ? results_.front() : value{};
    }
    inline auto results() const -> std::vector<value> override { return results_; }
    inline auto num_results() const -> std::size_t override { return results_.size(); }
    inline auto results_ref() -> std::vector<value> & { return results_; }
    inline auto results_ref() const -> std::vector<value> const & { return results_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    region then_, otherwise_;
    std::vector<value> results_;
};

class num_subgroups_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_num_subgroups; }
    inline num_subgroups_inst(location const &lc = {})
        : standard_inst{IK_num_subgroups}, result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class parallel_inst : public standard_inst<0, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_parallel; }
    inline parallel_inst(region body, location const &lc = {})
        : standard_inst{IK_parallel}, body_(std::move(body)) {
        loc(lc);
    }
    inline auto body() const -> region const & { return body_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::collective; }
    inline value result() const override { return value{}; }

  private:
    region body_;
};

class size_inst : public standard_inst<1, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_size; }
    size_inst(value op, std::int64_t mode, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
    std::int64_t mode_;
};

class subgroup_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_subgroup_id; }
    inline subgroup_id_inst(location const &lc = {})
        : standard_inst{IK_subgroup_id}, result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::spmd; }

  private:
    value result_;
};

class subgroup_local_id_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_subgroup_local_id; }
    inline subgroup_local_id_inst(location const &lc = {})
        : standard_inst{IK_subgroup_local_id}, result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::spmd; }

  private:
    value result_;
};

class subgroup_size_inst : public standard_inst<0, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_subgroup_size; }
    inline subgroup_size_inst(location const &lc = {})
        : standard_inst{IK_subgroup_size}, result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class subview_inst : public standard_inst<dynamic, 1> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_subview; }
    subview_inst(value op, std::vector<value> const &offset_list,
                 std::vector<value> const &size_list, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    // We have num_operands() = 1 + 2 * num_indices()
    inline auto num_indices() const { return (num_operands() - 1) / 2; }
    inline auto offset_list() const {
        return operands() | std::views::drop(1) | std::views::take(num_indices());
    }
    inline auto size_list() const { return operands() | std::views::drop(1 + num_indices()); }
    inline value result() const override { return result_; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }

  private:
    value result_;
};

class store_inst : public standard_inst<dynamic, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_store; }
    enum op_number { op_val = 0, op_operand = 1 };
    store_inst(value val, value op, std::vector<value> const &index_list, location const &lc = {});

    inline auto val() const -> value const & { return op(op_val); }
    inline auto operand() const -> value const & { return op(op_operand); }
    inline auto index_list() const { return operands() | std::views::drop(2); }
    inline value result() const override { return {}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }
};

class sum_inst : public blas_a2_inst {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_sum_blas_a2; }
    sum_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
             location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class yield_inst : public standard_inst<dynamic, 0> {
  public:
    inline static bool classof(inst_node const &i) { return i.type_id() == IK_yield; }
    inline yield_inst(std::vector<value> const &vals, location const &lc = {})
        : standard_inst{IK_yield, static_cast<std::int64_t>(vals.size())} {
        loc(lc);
        for (std::size_t i = 0; i < vals.size(); ++i) {
            op(i) = vals[i];
        }
    }
    inline value result() const override { return value{}; }
    inline inst_execution_kind kind() const override { return inst_execution_kind::mixed; }
};

} // namespace tinytc

#endif // INST_NODE_20230327_HPP
