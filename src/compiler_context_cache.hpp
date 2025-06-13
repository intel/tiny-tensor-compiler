// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_CONTEXT_CACHE_20240925_HPP
#define COMPILER_CONTEXT_CACHE_20240925_HPP

#include "tinytc/types.h"
#include "util/fnv1a.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace std {
template <> class hash<std::pair<tinytc_data_type_t, std::int64_t>> {
  public:
    auto operator()(std::pair<tinytc_data_type_t, std::int64_t> const &key) const -> std::size_t {
        return tinytc::fnv1a_combine(key.first, key.second);
    }
};
} // namespace std

namespace tinytc {

template <typename T> class unique_storage {
  public:
    ~unique_storage() {
        for (auto &m : map_) {
            delete m.second;
        }
    }

    template <typename EqualFun, typename MakeFun>
    auto get(std::uint64_t hash, EqualFun &&is_equal, MakeFun &&make) -> T {
        auto range = map_.equal_range(hash);
        for (auto it = range.first; it != range.second; ++it) {
            if (is_equal(it->second)) {
                return it->second;
            }
        }

        return map_.emplace(hash, make())->second;
    }

  private:
    std::unordered_multimap<std::uint64_t, T> map_;
};

class compiler_context_cache {
  public:
    compiler_context_cache(tinytc_compiler_context_t ctx);
    ~compiler_context_cache();

    compiler_context_cache(compiler_context_cache const &) = delete;
    compiler_context_cache &operator=(compiler_context_cache const &) = delete;

    std::unique_ptr<tinytc_data_type> void_ty, bool_ty;
    std::array<std::unique_ptr<tinytc_data_type>, TINYTC_NUMBER_OF_SCALAR_TYPES> scalar_tys;
    unique_storage<tinytc_data_type_t> coopmatrix_tys, group_tys, memref_tys;

    unique_storage<tinytc_attr_t> array_attrs, dictionary_attrs, integer_attrs, string_attrs;
    std::unique_ptr<tinytc_attr> false_attr, true_attr;
};

} // namespace tinytc

#endif // COMPILER_CONTEXT_CACHE_20240925_HPP
