// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_NODE_20230327_HPP
#define INST_NODE_20230327_HPP

#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <clir/virtual_type_list.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class inst_kind {
    mixed,      ///< mixed instruction on uniform or varying data
    collective, ///< collective instruction on uniform data, distributed among work-items
    spmd        ///< SPMD instruction on varying data

};

using inst_nodes = clir::virtual_type_list<
    class alloca_inst, class axpby_inst, class barrier_inst, class arith_inst,
    class arith_unary_inst, class cast_inst, class compare_inst, class expand_inst, class fuse_inst,
    class load_inst, class group_id_inst, class group_size_inst, class lifetime_stop_inst,
    class gemm_inst, class gemv_inst, class ger_inst, class for_inst, class foreach_inst,
    class hadamard_inst, class if_inst, class num_subgroups_inst, class parallel_inst,
    class size_inst, class subview_inst, class store_inst, class subgroup_id_inst,
    class subgroup_local_id_inst, class subgroup_size_inst, class sum_inst, class yield_inst>;

} // namespace tinytc

struct tinytc_inst : tinytc::reference_counted, tinytc::inst_nodes {
  public:
    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    // Iterator over operands
    virtual auto begin() -> tinytc::value * = 0;
    virtual auto end() -> tinytc::value * = 0;
    virtual auto cbegin() const -> tinytc::value const * = 0;
    virtual auto cend() const -> tinytc::value const * = 0;
    inline auto begin() const -> tinytc::value const * { return cbegin(); }
    inline auto end() const -> tinytc::value const * { return cend(); }

    virtual tinytc::value result() const = 0;
    inline virtual auto results() const -> std::vector<tinytc::value> {
        if (auto r = result(); r) {
            return {std::move(r)};
        }
        return {};
    }
    inline virtual auto num_results() const -> std::size_t { return result() ? 1u : 0u; }
    virtual tinytc::inst_kind kind() const = 0;

  private:
    tinytc::location loc_;
};

namespace tinytc {

using inst_node = ::tinytc_inst;

template <std::size_t NumOperands> class standard_inst : public inst_node {
  public:
    template <typename... Ts> inline standard_inst(Ts &&...ts) : ops_{std::forward<Ts>(ts)...} {}

    inline auto begin() -> tinytc::value * override { return ops_.data(); }
    inline auto end() -> tinytc::value * override { return ops_.data() + ops_.size(); }
    inline auto cbegin() const -> tinytc::value const * override { return ops_.data(); }
    inline auto cend() const -> tinytc::value const * override { return ops_.data() + ops_.size(); }

    inline auto op(std::size_t pos) -> value & { return ops_[pos]; }
    inline auto op(std::size_t pos) const -> value const & { return ops_[pos]; }

  private:
    std::array<value, NumOperands> ops_;
};
class standard_variadic_inst : public inst_node {
  public:
    template <typename... Ts>
    inline standard_variadic_inst(Ts &&...ts) : ops_{std::forward<Ts>(ts)...} {}

    inline auto begin() -> tinytc::value * override { return ops_.data(); }
    inline auto end() -> tinytc::value * override { return ops_.data() + ops_.size(); }
    inline auto cbegin() const -> tinytc::value const * override { return ops_.data(); }
    inline auto cend() const -> tinytc::value const * override { return ops_.data() + ops_.size(); }

    inline auto op(std::size_t pos) -> value & { return ops_[pos]; }
    inline auto op(std::size_t pos) const -> value const & { return ops_[pos]; }
    inline auto ops() -> std::vector<value> & { return ops_; }
    inline auto ops() const -> std::vector<value> const & { return ops_; }

  private:
    std::vector<value> ops_;
};

class blas_a2_inst : public standard_inst<4u> {
  public:
    blas_a2_inst(value alpha, value A, value beta, value B, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() const -> value const & { return op(0); }
    inline auto A() const -> value const & { return op(1); }
    inline auto beta() const -> value const & { return op(2); }
    inline auto B() const -> value const & { return op(3); }
    inline value result() const override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  protected:
    bool atomic_;
};

class blas_a3_inst : public standard_inst<5u> {
  public:
    blas_a3_inst(value alpha, value A, value B, value beta, value C, bool atomic);

    inline bool atomic() const { return atomic_; }
    inline void atomic(bool a) { atomic_ = a; }
    inline auto alpha() const -> value const & { return op(0); }
    inline auto A() const -> value const & { return op(1); }
    inline auto B() const -> value const & { return op(2); }
    inline auto beta() const -> value const & { return op(3); }
    inline auto C() const -> value const & { return op(4); }
    inline value result() const override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  protected:
    bool atomic_;
};

class loop_inst : public standard_inst<4u> {
  public:
    loop_inst(value loop_var, value from, value to, region body, location const &loc = {});
    loop_inst(value loop_var, value from, value to, value step, region body,
              location const &loc = {});
    inline auto loop_var() const -> value const & { return op(0); }
    inline auto from() const -> value const & { return op(1); }
    inline auto to() const -> value const & { return op(2); }
    inline auto step() const -> value const & { return op(3); }
    inline auto body() const -> region const & { return body_; }
    inline value result() const override { return value{}; }

  private:
    region body_;
};

class alloca_inst : public clir::visitable<alloca_inst, standard_inst<0u>> {
  public:
    alloca_inst(data_type ty, location const &loc = {});

    inline value result() const override { return result_; }
    inline std::int64_t stack_ptr() const { return stack_ptr_; }
    inline void stack_ptr(std::int64_t ptr) { stack_ptr_ = ptr; }
    inline inst_kind kind() const override { return inst_kind::collective; }

  private:
    value result_;
    std::int64_t stack_ptr_;
};

class axpby_inst : public clir::visitable<axpby_inst, blas_a2_inst> {
  public:
    using super = clir::visitable<axpby_inst, blas_a2_inst>;
    axpby_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
               location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class arith_inst : public clir::visitable<arith_inst, standard_inst<2u>> {
  public:
    using super = clir::visitable<arith_inst, standard_inst<2u>>;
    arith_inst(arithmetic op, value a, value b, location const &lc = {});

    inline arithmetic operation() const { return operation_; }
    inline auto a() const -> value const & { return op(0); }
    inline auto b() const -> value const & { return op(1); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    arithmetic operation_;
    value result_;
};

class arith_unary_inst : public clir::visitable<arith_unary_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<arith_unary_inst, standard_inst<1u>>;
    arith_unary_inst(arithmetic_unary op, value a, location const &lc = {});

    inline arithmetic_unary operation() const { return operation_; }
    inline auto a() const -> value const & { return op(0); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    arithmetic_unary operation_;
    value result_;
};

class barrier_inst : public clir::visitable<barrier_inst, standard_inst<0u>> {
  public:
    inline value result() const override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }
};

class cast_inst : public clir::visitable<cast_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<cast_inst, standard_inst<1u>>;
    cast_inst(value a, scalar_type to_ty, location const &lc = {});
    inline auto a() const -> value const & { return op(0); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class compare_inst : public clir::visitable<compare_inst, standard_inst<2u>> {
  public:
    using super = clir::visitable<compare_inst, standard_inst<2u>>;
    compare_inst(cmp_condition cond, value a, value b, location const &lc = {});

    inline cmp_condition cond() const { return cond_; }
    inline auto a() const -> value const & { return op(0); }
    inline auto b() const -> value const & { return op(1); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    cmp_condition cond_;
    value result_;
};

class expand_inst : public clir::visitable<expand_inst, standard_variadic_inst> {
  public:
    using super = clir::visitable<expand_inst, standard_variadic_inst>;
    expand_inst(value op, std::int64_t mode, std::vector<value> const &expand_shape,
                location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }
    inline auto expand_shape() { return ops() | std::views::drop(1); }
    inline auto expand_shape() const { return ops() | std::views::drop(1); }
    inline auto expand_shape(std::int64_t i) const -> value const & { return op(i + 1); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
    std::int64_t mode_;
};

class fuse_inst : public clir::visitable<fuse_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<fuse_inst, standard_inst<1u>>;
    fuse_inst(value op, std::int64_t from, std::int64_t to, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t from() const { return from_; }
    inline std::int64_t to() const { return to_; }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
    std::int64_t from_, to_;
};

class load_inst : public clir::visitable<load_inst, standard_variadic_inst> {
  public:
    using super = clir::visitable<load_inst, standard_variadic_inst>;
    load_inst(value op, std::vector<value> const &index_list, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline auto index_list() const { return ops() | std::views::drop(1); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class group_id_inst : public clir::visitable<group_id_inst, standard_inst<0u>> {
  public:
    inline group_id_inst(location const &lc = {}) : result_{make_value(scalar_type::index)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class group_size_inst : public clir::visitable<group_size_inst, standard_inst<0u>> {
  public:
    inline group_size_inst(location const &lc = {}) : result_{make_value(scalar_type::index)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class lifetime_stop_inst : public clir::visitable<lifetime_stop_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<lifetime_stop_inst, standard_inst<1u>>;
    inline lifetime_stop_inst(value obj) : super{std::move(obj)} {}
    inline auto object() const -> value const & { return op(0); }
    inline value result() const override { return value{}; }
    inline inst_kind kind() const override { return inst_kind::collective; }
};

class gemm_inst : public clir::visitable<gemm_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<gemm_inst, blas_a3_inst>;
    gemm_inst(transpose tA, transpose tB, value alpha, value A, value B, value beta, value C,
              bool atomic = false, location const &lc = {});

    inline transpose tA() const { return tA_; }
    inline transpose tB() const { return tB_; }

  private:
    transpose tA_, tB_;
};

class gemv_inst : public clir::visitable<gemv_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<gemv_inst, blas_a3_inst>;
    gemv_inst(transpose tA, value alpha, value A, value B, value beta, value C, bool atomic = false,
              location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class ger_inst : public clir::visitable<ger_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<ger_inst, blas_a3_inst>;
    ger_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
             location const &lc = {});
};

class for_inst : public clir::visitable<for_inst, loop_inst> {
  public:
    using super = clir::visitable<for_inst, loop_inst>;
    using super::super;
    inline inst_kind kind() const override { return inst_kind::mixed; }
};

class foreach_inst : public clir::visitable<foreach_inst, loop_inst> {
  public:
    using super = clir::visitable<foreach_inst, loop_inst>;
    inline foreach_inst(value loop_var, value from, value to, region body, location const &loc = {})
        : super(std::move(loop_var), std::move(from), std::move(to), std::move(body), loc) {}
    inline inst_kind kind() const override { return inst_kind::collective; }
};

class hadamard_inst : public clir::visitable<hadamard_inst, blas_a3_inst> {
  public:
    using super = clir::visitable<hadamard_inst, blas_a3_inst>;
    hadamard_inst(value alpha, value A, value B, value beta, value C, bool atomic = false,
                  location const &lc = {});
};

class if_inst : public clir::visitable<if_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<if_inst, standard_inst<1u>>;
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
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    region then_, otherwise_;
    std::vector<value> results_;
};

class num_subgroups_inst : public clir::visitable<num_subgroups_inst, standard_inst<0u>> {
  public:
    inline num_subgroups_inst(location const &lc = {}) : result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class parallel_inst : public clir::visitable<parallel_inst, standard_inst<0u>> {
  public:
    using super = clir::visitable<parallel_inst, loop_inst>;
    inline parallel_inst(region body, location const &lc = {}) : body_(std::move(body)) { loc(lc); }
    inline auto body() const -> region const & { return body_; }
    inline inst_kind kind() const override { return inst_kind::collective; }
    inline value result() const override { return value{}; }

  private:
    region body_;
};

class size_inst : public clir::visitable<size_inst, standard_inst<1u>> {
  public:
    using super = clir::visitable<size_inst, standard_inst<1u>>;
    size_inst(value op, std::int64_t mode, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    inline std::int64_t mode() const { return mode_; }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
    std::int64_t mode_;
};

class subgroup_id_inst : public clir::visitable<subgroup_id_inst, standard_inst<0u>> {
  public:
    inline subgroup_id_inst(location const &lc = {}) : result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::spmd; }

  private:
    value result_;
};

class subgroup_local_id_inst : public clir::visitable<subgroup_local_id_inst, standard_inst<0u>> {
  public:
    inline subgroup_local_id_inst(location const &lc = {}) : result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::spmd; }

  private:
    value result_;
};

class subgroup_size_inst : public clir::visitable<subgroup_size_inst, standard_inst<0u>> {
  public:
    inline subgroup_size_inst(location const &lc = {}) : result_{make_value(scalar_type::i32)} {
        loc(lc);
    }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class subview_inst : public clir::visitable<subview_inst, standard_variadic_inst> {
  public:
    using super = clir::visitable<subview_inst, standard_variadic_inst>;
    subview_inst(value op, std::vector<value> const &offset_list,
                 std::vector<value> const &size_list, location const &lc = {});

    inline auto operand() const -> value const & { return op(0); }
    // We have ops().size() = 1 + 2 * num_indices()
    inline auto num_indices() const { return (ops().size() - 1) / 2; }
    inline auto offset_list() const {
        return ops() | std::views::drop(1) | std::views::take(num_indices());
    }
    inline auto size_list() const { return ops() | std::views::drop(1 + num_indices()); }
    inline value result() const override { return result_; }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    value result_;
};

class store_inst : public clir::visitable<store_inst, standard_variadic_inst> {
  public:
    using super = clir::visitable<store_inst, standard_variadic_inst>;
    store_inst(value val, value op, std::vector<value> const &index_list, location const &lc = {});

    inline auto val() const -> value const & { return op(0); }
    inline auto operand() const -> value const & { return op(1); }
    inline auto index_list() const { return ops() | std::views::drop(2); }
    inline value result() const override { return {}; }
    inline inst_kind kind() const override { return inst_kind::mixed; }
};

class sum_inst : public clir::visitable<sum_inst, blas_a2_inst> {
  public:
    using super = clir::visitable<sum_inst, blas_a2_inst>;
    sum_inst(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
             location const &lc = {});

    inline transpose tA() const { return tA_; }

  private:
    transpose tA_;
};

class yield_inst : public clir::visitable<yield_inst, standard_variadic_inst> {
  public:
    using super = clir::visitable<yield_inst, standard_variadic_inst>;
    inline yield_inst(std::vector<value> const &vals, location const &lc = {}) {
        loc(lc);
        ops().insert(ops().end(), vals.begin(), vals.end());
    }
    inline value result() const override { return value{}; }
    inline auto vals() const -> std::vector<value> const & { return ops(); }
    inline inst_kind kind() const override { return inst_kind::mixed; }

  private:
    std::vector<value> vals_;
};

} // namespace tinytc

#endif // INST_NODE_20230327_HPP
