// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/attr_node.hpp"
#include "compiler_context.hpp"
#include "compiler_context_cache.hpp"
#include "support/casting.hpp"
#include "support/fnv1a.hpp"
#include "support/fnv1a_array_view.hpp" // IWYU pragma: keep

#include <algorithm>
#include <memory>
#include <utility>

namespace tinytc {

auto array_attr::get(tinytc_compiler_context_t ctx,
                     array_view<tinytc_attr_t> values) -> tinytc_attr_t {
    const auto hash = fnv1a_combine(values);
    const auto is_equal = [&](tinytc_attr_t a) {
        const auto aa = dyn_cast<array_attr>(a);
        return aa &&
               std::equal(values.begin(), values.end(), aa->values().begin(), aa->values().end());
    };
    const auto make = [&]() { return new array_attr(ctx, values); };

    auto &attrs = ctx->cache()->array_attrs;
    return attrs.get(hash, std::move(is_equal), std::move(make));
}

array_attr::array_attr(tinytc_compiler_context_t ctx, std::vector<tinytc_attr_t> values)
    : tinytc_attr(AK::array, ctx), values_{std::move(values)} {}

auto boolean_attr::get(tinytc_compiler_context_t ctx, bool value) -> tinytc_attr_t {
    auto cache = ctx->cache();
    return value ? cache->true_attr.get() : cache->false_attr.get();
}

boolean_attr::boolean_attr(tinytc_compiler_context_t ctx, bool value)
    : tinytc_attr(AK::boolean, ctx), value_{value} {}

auto dictionary_attr::get(tinytc_compiler_context_t ctx,
                          array_view<named_attr> sorted_attrs) -> tinytc_attr_t {
    const auto hash = [&] {
        auto h = fnv1a0();
        for (auto const &na : sorted_attrs) {
            h = fnv1a_step(h, na.name);
            h = fnv1a_step(h, na.attr);
        }
        return h;
    };
    const auto is_equal = [&](tinytc_attr_t a) {
        const auto da = dyn_cast<dictionary_attr>(a);
        return da && std::equal(sorted_attrs.begin(), sorted_attrs.end(), da->begin(), da->end(),
                                [](named_attr const &a, named_attr const &b) {
                                    return a.name == b.name && a.attr == b.attr;
                                });
    };
    const auto make = [&]() { return new dictionary_attr(ctx, sorted_attrs); };

    auto &attrs = ctx->cache()->dictionary_attrs;
    return attrs.get(hash(), std::move(is_equal), std::move(make));
}

auto dictionary_attr::get_name_string(tinytc_attr_t name) -> std::string_view {
    auto stra = dyn_cast<string_attr>(name);
    if (stra) {
        return stra->str();
    }
    throw status::ir_expected_string_attribute;
}

void dictionary_attr::sort(mutable_array_view<named_attr> unsorted_attrs) {
    if (unsorted_attrs.empty()) {
        return;
    }
    std::sort(unsorted_attrs.begin(), unsorted_attrs.end(),
              [](named_attr const &a, named_attr const &b) {
                  return get_name_string(a.name) < get_name_string(b.name);
              });
    for (std::size_t i = 1; i < unsorted_attrs.size(); ++i) {
        if (unsorted_attrs[i - 1].name == unsorted_attrs[i].name) {
            throw status::ir_duplicate_key_in_dictionary;
        }
    }
}

dictionary_attr::dictionary_attr(tinytc_compiler_context_t ctx,
                                 std::vector<named_attr> sorted_attrs)
    : tinytc_attr(AK::dictionary, ctx), attrs_{std::move(sorted_attrs)} {}

auto dictionary_attr::find(tinytc_attr_t name) -> tinytc_attr_t {
    if (!attrs_.empty() && name) {
        auto namestr = get_name_string(name);
        std::size_t lb = 0;
        std::size_t ub = attrs_.size();

        do {
            std::size_t mid = (lb + ub) / 2;
            auto cmp = namestr.compare(get_name_string(attrs_[mid].name));
            if (cmp == 0) {
                return attrs_[mid].attr;
            } else if (cmp < 0) {
                ub = mid;
            } else {
                lb = mid + 1;
            }
        } while (ub > lb);
    }
    return nullptr;
}
auto dictionary_attr::find(std::string_view name) -> tinytc_attr_t {
    return find(string_attr::get(context(), name));
}

auto integer_attr::get(tinytc_compiler_context_t ctx, std::int64_t value) -> tinytc_attr_t {
    const auto hash = fnv1a_combine(value);
    const auto is_equal = [&](tinytc_attr_t a) {
        const auto ia = dyn_cast<integer_attr>(a);
        return ia && value == ia->value();
    };
    const auto make = [&]() { return new integer_attr(ctx, value); };

    auto &attrs = ctx->cache()->integer_attrs;
    return attrs.get(hash, std::move(is_equal), std::move(make));
}

integer_attr::integer_attr(tinytc_compiler_context_t ctx, std::int64_t value)
    : tinytc_attr(AK::integer, ctx), value_{value} {}

auto string_attr::get(tinytc_compiler_context_t ctx, std::string_view str) -> tinytc_attr_t {
    const auto hash = fnv1a_combine(str);
    const auto is_equal = [&](tinytc_attr_t a) {
        const auto stra = dyn_cast<string_attr>(a);
        return stra && str == stra->str();
    };
    const auto make = [&]() { return new string_attr(ctx, std::string{str}); };

    auto &attrs = ctx->cache()->string_attrs;
    return attrs.get(hash, std::move(is_equal), std::move(make));
}

string_attr::string_attr(tinytc_compiler_context_t ctx, std::string str)
    : tinytc_attr(AK::string, ctx), str_{std::move(str)} {}

auto get_attr(tinytc_attr_t dict, tinytc_attr_t name) -> tinytc_attr_t {
    if (auto da = dyn_cast<dictionary_attr>(dict); da) {
        return da->find(name);
    }
    return nullptr;
}
auto get_attr(tinytc_attr_t dict, std::string_view name) -> tinytc_attr_t {
    if (auto da = dyn_cast<dictionary_attr>(dict); da) {
        return da->find(name);
    }
    return nullptr;
}

} // namespace tinytc
