// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BUILDER_20230309_HPP
#define BUILDER_20230309_HPP

#include "tinytc/export.h"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/value.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <source_location>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class prog;
class slice;

TINYTC_EXPORT location to_location(std::source_location const location);

namespace internal {
class TINYTC_EXPORT unique_name_giver {
  public:
    std::string name(std::string const &prefix);
    inline std::unordered_map<std::string, int> const &name_counters() { return names_; }
    inline void name_counters(std::unordered_map<std::string, int> names) {
        names_ = std::move(names);
    }

  private:
    std::unordered_map<std::string, int> names_;
};
} // namespace internal

//! Builder for regions
class TINYTC_EXPORT region_builder : public internal::unique_name_giver {
  public:
    //! ctor; creates empty region
    region_builder();

    //! Returns built product
    region get_product();

    //! @code %prefix = add %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    inline value create_add(value a, value b, std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current()) {
        return create_binary_op(binary_op::add, std::move(a), std::move(b), prefix, std::move(loc));
    }
    //! @code %prefix = sub %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    inline value create_sub(value a, value b, std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current()) {
        return create_binary_op(binary_op::sub, std::move(a), std::move(b), prefix, std::move(loc));
    }
    //! @code %prefix = mul %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    inline value create_mul(value a, value b, std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current()) {
        return create_binary_op(binary_op::mul, std::move(a), std::move(b), prefix, std::move(loc));
    }
    //! @code %prefix = div %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    inline value create_div(value a, value b, std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current()) {
        return create_binary_op(binary_op::div, std::move(a), std::move(b), prefix, std::move(loc));
    }
    //! @code %prefix = rem %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    inline value create_rem(value a, value b, std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current()) {
        return create_binary_op(binary_op::rem, std::move(a), std::move(b), prefix, std::move(loc));
    }
    //! @code %prefix = cast %a, %b : type(%a) -> %to_ty @endcode
    value create_cast(value a, scalar_type to_ty, std::string const &prefix = "",
                      std::source_location const loc = std::source_location::current());
    //! @code %prefix = cmp.%cond %a, %b : type(%a) ; type(%a) == type(%b) @endcode
    value create_cmp(cmp_condition cond, value a, value b, std::string const &prefix = "",
                     std::source_location const loc = std::source_location::current());
    //! @code %prefix = neg %a : type(%a) @endcode
    value create_neg(value a, std::string const &prefix = "",
                     std::source_location const loc = std::source_location::current());
    //! @code %prefix = alloca -> %ty @endcode
    value create_alloca(data_type ty, std::string const &prefix = "",
                        std::source_location const loc = std::source_location::current());
    //! @code axpby.%tA.%atomic %alpha, %A, %beta, %B : type(%alpha), type(%A), type(%beta),
    //! type(%B) @endcode
    void create_axpby(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
                      std::source_location const loc = std::source_location::current());
    //! @code barrier @endcode
    void create_barrier(std::source_location const loc = std::source_location::current());
    //! @code %prefix = expand %arg[%mode -> %expand_shape] : type(%arg) @endcode
    value create_expand(value arg, std::int64_t mode, std::vector<value> expand_shape,
                        std::string const &prefix = "",
                        std::source_location const loc = std::source_location::current());
    //! @code %prefix = fuse %arg[%from, %to] : type(%arg) @endcode
    value create_fuse(value arg, std::int64_t from, std::int64_t to, std::string const &prefix = "",
                      std::source_location const loc = std::source_location::current());
    //! @code %prefix = load %arg[%index_list] : type(%arg) @endcode
    value create_load(value arg, std::vector<value> index_list, std::string const &prefix = "",
                      std::source_location const loc = std::source_location::current());
    //! @code %prefix = group_id @endcode
    value create_group_id(std::string const &prefix = "",
                          std::source_location const loc = std::source_location::current());
    //! @code %prefix = group_size @endcode
    value create_group_size(std::string const &prefix = "",
                            std::source_location const loc = std::source_location::current());
    /**
     * @code
     * gemm.%tA.%tB.%atomic %alpha, %A, %B, %beta, %C
     *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
     * @endcode
     */
    void create_gemm(transpose tA, transpose tB, value alpha, value A, value B, value beta, value C,
                     bool atomic = false,
                     std::source_location const loc = std::source_location::current());
    /**
     * @code
     * gemv.%tA.%atomic %alpha, %A, %B, %beta, %C
     *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
     * @endcode
     */
    void create_gemv(transpose tA, value alpha, value A, value B, value beta, value C,
                     bool atomic = false,
                     std::source_location const loc = std::source_location::current());
    /**
     * @code
     * ger.%atomic %alpha, %A, %B, %beta, %C
     *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
     * @endcode
     */
    void create_ger(value alpha, value A, value B, value beta, value C, bool atomic = false,
                    std::source_location const loc = std::source_location::current());
    /**
     * @code
     * hadamard.%atomic %alpha, %A, %B, %beta, %C
     *     : type(%alpha), type(%A), type(%B), type(%beta), type(%C)
     * @endcode
     */
    void create_hadamard(value alpha, value A, value B, value beta, value C, bool atomic = false,
                         std::source_location const loc = std::source_location::current());
    //! @code %prefix = size %arg[%mode] : type(%arg) @endcode
    value create_size(value arg, std::int64_t mode, std::string const &prefix = "",
                      std::source_location const loc = std::source_location::current());
    //! @code %subview = subview %op[%slices] : type(%arg) @endcode
    value create_subview(value op, std::vector<slice> slices, std::string const &prefix = "",
                         std::source_location const loc = std::source_location::current());
    //! @code store %val, %op[%index_list] : type(%op) @endcode
    void create_store(value val, value op, std::vector<value> index_list,
                      std::string const &prefix = "",
                      std::source_location const loc = std::source_location::current());
    //! @code sum.%tA.%atomic %alpha, %A, %beta, %B : type(%alpha), type(%A), type(%beta), type(%B)
    //! @endcode
    void create_sum(transpose tA, value alpha, value A, value beta, value B, bool atomic = false,
                    std::source_location const loc = std::source_location::current());

    /**
     * @code
     * for %loop_var = %from, %to : type(%loop_var) { %body }
     * ; type(%loop_var) == type(%from)
     * ; type(%loop_var) == type(%to)
     * @endcode
     */
    void create_for(value loop_var, value from, value to, region body,
                    std::source_location const loc = std::source_location::current());
    /**
     * @code
     * for %loop_var = %from, %to, %step : type(%loop_var) { %body }
     * ; type(%loop_var) == type(%from)
     * ; type(%loop_var) == type(%to)
     * ; type(%loop_var) == type(%step)
     * @endcode
     */
    void create_for(value loop_var, value from, value to, value step, region body,
                    std::source_location const loc = std::source_location::current());
    //! Build for-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_for(data_type loop_var_ty, value from, value to, F &&f,
                    std::string const &prefix = "",
                    std::source_location const loc = std::source_location::current()) {
        create_for<F>(std::move(loop_var_ty), std::move(from), std::move(to), nullptr,
                      std::forward<F>(f), prefix, std::move(loc));
    }
    //! Build for-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_for(data_type loop_var_ty, value from, value to, value step, F &&f,
                    std::string const &prefix = "",
                    std::source_location const loc = std::source_location::current()) {
        auto loop_var = value(std::move(loop_var_ty), this->name(prefix));
        auto bb = region_builder{};
        bb.name_counters(this->name_counters());
        f(bb);
        create_for(std::move(loop_var), std::move(from), std::move(to), std::move(step),
                   bb.get_product(), std::move(loc));
    }
    /**
     * @code
     * foreach %loop_var = %from, %to : type(%loop_var) { %body }
     * ; type(%loop_var) == type(%from)
     * ; type(%loop_var) == type(%to)
     * @endcode
     */
    void create_foreach(value loop_var, value from, value to, region body,
                        std::source_location const loc = std::source_location::current());
    //! Build foreach-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_foreach(data_type loop_var_ty, value from, value to, F &&f,
                        std::string const &prefix = "",
                        std::source_location const loc = std::source_location::current()) {
        auto loop_var = value(std::move(loop_var_ty), this->name(prefix));
        auto bb = region_builder{};
        bb.name_counters(this->name_counters());
        f(bb);
        create_foreach(std::move(loop_var), std::move(from), std::move(to), bb.get_product(),
                       std::move(loc));
    }

    /**
     * @code
     * if %condition { %then } else { %otherwise }
     * @endcode
     *
     * Set otherwise = nullptr to omit the else region
     */
    void create_if(value condition, region then, region otherwise = nullptr,
                   std::source_location const loc = std::source_location::current());
    //! Build if with functor then(region_builder&) -> void
    template <typename F>
    void create_if(value condition, F &&then,
                   std::source_location const loc = std::source_location::current()) {
        auto bb = region_builder{};
        bb.name_counters(this->name_counters());
        then(bb);
        create_if(std::move(condition), bb.get_product(), std::move(loc));
    }
    //! Build if/else with functors then(region_builder&) -> void and otherwise(region_builder&) ->
    //! void
    template <typename F, typename G>
    void create_ifelse(value condition, F &&then, G &&otherwise,
                       std::source_location const loc = std::source_location::current()) {
        auto bb1 = region_builder{};
        bb1.name_counters(this->name_counters());
        then(bb1);
        auto bb2 = region_builder{};
        bb2.name_counters(this->name_counters());
        otherwise(bb2);
        create_if(std::move(condition), bb1.get_product(), bb2.get_product(), std::move(loc));
    }

  private:
    value create_binary_op(binary_op op, value a, value b, std::string const &prefix = "",
                           std::source_location const loc = std::source_location::current());
    value insert(std::shared_ptr<inst_node> s, std::string const &prefix = "");
    region reg_;
};

//! Builder for functions
class TINYTC_EXPORT function_builder : public internal::unique_name_giver {
  public:
    //! ctor; creates function \@name
    function_builder(std::string name);

    //! Returns built product
    func get_product();

    //! @code %prefix: %ty @endcode
    value argument(data_type ty, std::string const &prefix = "",
                   std::source_location const loc = std::source_location::current());

    //! @code work_group_size(%m, %n) @endcode
    inline void work_group_size(std::uint32_t m, std::uint32_t n) { work_group_size_ = {m, n}; }
    //! @code subgroup_size(%subgroup_size) @endcode
    inline void subgroup_size(std::uint32_t subgroup_size) { subgroup_size_ = subgroup_size; }

    //! Build function body with functor f(region_builder&) -> void
    template <typename F> void body(F &&f) {
        auto bb = region_builder{};
        bb.name_counters(this->name_counters());
        f(bb);
        body_ = bb.get_product();
    }

  private:
    func proto_;
    region body_;
    std::array<std::uint32_t, 2> work_group_size_ = {0, 0};
    std::uint32_t subgroup_size_ = 0;
};

//! Builder for programs
class TINYTC_EXPORT program_builder {
  public:
    //! create function \@name with functor f(function_builder&) -> void
    template <typename F> void create(std::string name, F &&f) {
        auto fb = function_builder(std::move(name));
        f(fb);
        add(fb.get_product());
    }
    //! Add function
    void add(func f);
    //! Returns built product
    prog get_product();

  private:
    std::vector<func> functions_;
};

} // namespace tinytc

#endif // BUILDER_20230309_HPP
