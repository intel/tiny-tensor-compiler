// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OVERLOADED_20250620_HPP
#define OVERLOADED_20250620_HPP

namespace tinytc {

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace tinytc

#endif // OVERLOADED_20250620_HPP
