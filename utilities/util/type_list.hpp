// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TYPE_LIST_20240903_HPP
#define TYPE_LIST_20240903_HPP

#include <cstdint>

namespace tinytc {

template <std::size_t Index, typename... Types> class type_at {
  private:
    static_assert(Index < sizeof...(Types), "Type index out of bounds");

    template <std::size_t I, typename... T> struct impl;
    template <std::size_t I, typename Head, typename... Tail> struct impl<I, Head, Tail...> {
        using type = typename impl<I - 1, Tail...>::type;
    };
    template <typename Head, typename... Tail> struct impl<0, Head, Tail...> {
        using type = Head;
    };

  public:
    using type = typename impl<Index, Types...>::type;
};

template <std::size_t Index, typename... Types>
using type_at_t = typename type_at<Index, Types...>::type;

/**
 * @brief Simple type list that allows to query the type at index 0,...,number_of_types()-1
 *
 * @tparam Types
 */
template <typename... Types> struct type_list {
    template <std::size_t Index> using type_at = type_at_t<Index, Types...>;
    static constexpr auto number_of_types() { return sizeof...(Types); }
};

} // namespace tinytc

#endif // TYPE_LIST_20240903_HPP
