// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CASTING_20240903_HPP
#define CASTING_20240903_HPP

#include <type_traits>

// LLVM-style RTTI, cf. https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html

namespace tinytc {

template <typename T, typename S> struct copy_cv {
  private:
    using T1 = std::conditional_t<std::is_const<S>::value, std::add_const_t<T>, T>;
    using T2 = std::conditional_t<std::is_volatile<S>::value, std::add_volatile_t<T1>, T1>;

  public:
    using type = T2;
};
template <typename Target, typename Source>
using copy_cv_t = typename copy_cv<Target, Source>::type;

template <typename To, typename From>
requires(std::is_base_of_v<std::decay_t<From>, std::decay_t<To>>)
auto isa(From const &obj) -> bool {
    return To::classof(obj);
}

template <typename To, typename From>
requires(std::is_base_of_v<std::decay_t<From>, std::decay_t<To>>)
auto cast(From *obj) {
    return (copy_cv_t<To, From> *)obj;
}

template <typename To, typename From>
requires(std::is_base_of_v<std::decay_t<From>, std::decay_t<To>>)
auto dyn_cast(From *obj) -> To * {
    if (obj != nullptr && isa<To>(*obj)) {
        return cast<To, From>(obj);
    }
    return nullptr;
}

} // namespace tinytc

#endif // CASTING_20240903_HPP
