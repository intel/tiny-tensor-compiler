// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VISIT_20240903_HPP
#define VISIT_20240903_HPP

#include "casting.hpp"

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace tinytc {

namespace detail {
/**
 * @brief Computes \prod_{i=0}^{MaxMode-1} Size_i, where Size_0 = Head, and Size_i = Tail_i for i >
 * 0. Always returns 1 for MaxMode = 0.
 */
template <std::size_t MaxMode, std::size_t Head, std::size_t... Tail>
constexpr auto partial_product(std::index_sequence<Head, Tail...>) -> std::size_t {
    if constexpr (MaxMode == 0) {
        return 1;
    } else {
        return Head * partial_product<MaxMode - 1, Tail...>(std::index_sequence<Tail...>{});
    }
}

/**
 * In an ND-tensor, the ND-index (i_0, ..., i_n) is flattend via
 * Index = i_0 + i_1 * Size_0 + ... + i_n * Size_0 * ... * Size_{n-1}
 * This function takes the compile-time flat Index and recovers the ND-index, and returns the
 * ND-index as index_sequence.
 */
template <std::size_t Index, std::size_t... Size>
constexpr auto unflatten(std::index_sequence<Size...>) {
    return []<std::size_t... Modes>(std::index_sequence<Modes...>) {
        return std::index_sequence<(Index / partial_product<Modes>(std::index_sequence<Size...>{}) %
                                    Size)...>{};
    }(std::make_index_sequence<sizeof...(Size)>{});
}
} // namespace detail

template <typename T>
concept type_id_return_type = std::is_integral_v<T> || std::is_enum_v<T>;

template <typename T>
concept visitable = requires(T ty) {
    typename T::leaves;
    { ty.type_id() } -> type_id_return_type;
};

/**
 * Multiple dispatch for class hierachies that implement LLVM-style RTTI and have
 * a "leaves" type-list that lists all leaf classes (i.e. classes that have no children).
 *
 * The function works as following:
 *
 * 1. Input are one or multiple objects with LLVM-style RTTI and leaves type-list (T &...).
 *    The leaf type list can be different for each t.
 * 2. Enumerate all potential cases of type combinations (see table_size).
 * 3. Call the compile_time_switch lambda for all case numbers.
 * 4. Use the unflatten function to get the type ND-index, i.e. the position in the type list for
 *    each t.
 * 5. Use the "is-a all" lambda to check whether t... is of the type combination covered in the
 *    case.
 * 6. If the case number matches the actual type combination of t..., then call the visitor, casting
 *    each t... to its actual type.
 */
template <typename Visitor, visitable... T> auto visit(Visitor &&visitor, T &...t) {
    auto compile_time_switch = [&]<std::size_t... Case>(std::index_sequence<Case...>) {
        const auto isa_all = [&]<std::size_t... Index>(std::index_sequence<Index...>) -> bool {
            return (isa<typename std::decay_t<T>::leaves::template type_at<Index>>(t) && ...);
        };
        const auto dispatch = [&]<std::size_t... Index>(std::index_sequence<Index...>) {
            return visitor(*cast<typename std::decay_t<T>::leaves::template type_at<Index>>(&t)...);
        };

        using size = std::index_sequence<std::decay_t<T>::leaves::number_of_types()...>;

        using return_type =
            std::common_type_t<decltype(dispatch(detail::unflatten<Case>(size{})))...>;

        if constexpr (std::is_same_v<return_type, void>) {
            [[maybe_unused]] int discard = ((isa_all(detail::unflatten<Case>(size{}))
                                             ? dispatch(detail::unflatten<Case>(size{})),
                                             0 : 0) ||
                                            ...);
        } else {
            return_type ret = {};
            [[maybe_unused]] int discard = ((isa_all(detail::unflatten<Case>(size{}))
                                             ? (ret = dispatch(detail::unflatten<Case>(size{}))),
                                             0 : 0) ||
                                            ...);
            return ret;
        }
    };

    constexpr std::size_t table_size = (std::decay_t<T>::leaves::number_of_types() * ...);
    return compile_time_switch(std::make_index_sequence<table_size>{});
}

} // namespace tinytc

#endif // VISIT_20240903_HPP
