// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_CONTEXT_CACHE_20240925_HPP
#define COMPILER_CONTEXT_CACHE_20240925_HPP

#include "node/data_type_node.hpp"
#include "support/util.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <unordered_map>

namespace std {
template <> class hash<std::pair<tinytc_data_type_t, std::int64_t>> {
  public:
    auto operator()(std::pair<tinytc_data_type_t, std::int64_t> const &key) const -> std::size_t {
        return tinytc::fnv1a(key.first, key.second);
    }
};
} // namespace std

namespace tinytc {

class compiler_context_cache {
  public:
    compiler_context_cache(tinytc_compiler_context_t ctx);
    ~compiler_context_cache();

    compiler_context_cache(compiler_context_cache const &) = delete;
    compiler_context_cache &operator=(compiler_context_cache const &) = delete;

    std::array<std::unique_ptr<tinytc_data_type>, TINYTC_NUMBER_OF_SCALAR_TYPES> scalar_tys;
    std::unordered_multimap<std::uint64_t, tinytc_data_type_t> memref_tys;
    std::unordered_map<std::pair<tinytc_data_type_t, std::int64_t>, tinytc_data_type_t> group_tys;
};

} // namespace tinytc

#endif // COMPILER_CONTEXT_CACHE_20240925_HPP