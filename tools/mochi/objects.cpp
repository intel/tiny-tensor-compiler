// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "objects.hpp"
#include "util/fnv1a.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace mochi {

template <typename T>
auto find_in_list(std::vector<std::unique_ptr<T>> const &list, std::string_view name,
                  std::uint64_t hash) -> T * {
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        const auto candidate_hash = tinytc::fnv1a((*it)->name());
        if (hash == candidate_hash && name == (*it)->name()) {
            return it->get();
        }
        if (T *i = find_in_list((*it)->children(), name, hash); i) {
            return i;
        }
    }
    return nullptr;
}

template <typename T>
auto find_in_list(std::vector<std::unique_ptr<T>> const &list, std::string_view name) {
    return find_in_list(list, name, tinytc::fnv1a(name));
}

void objects::add(std::unique_ptr<enum_> e) { enums_.emplace_back(std::move(e)); }

void objects::add(inst *parent, std::unique_ptr<inst> i) {
    auto &parent_list = parent ? parent->children() : insts_;
    parent_list.emplace_back(std::move(i));
}

void objects::add(type *parent, std::unique_ptr<type> t) {
    auto &parent_list = parent ? parent->children() : types_;
    parent_list.emplace_back(std::move(t));
}

void objects::add(objects &&other) {
    for (auto &&e : std::move(other.enums_)) {
        enums_.emplace_back(std::move(e));
    }
    for (auto &&i : std::move(other.insts_)) {
        insts_.emplace_back(std::move(i));
    }
    for (auto &&t : std::move(other.types_)) {
        types_.emplace_back(std::move(t));
    }
}

auto objects::find_enum(std::string_view name) -> enum_ * {
    const auto hash = tinytc::fnv1a(name);
    for (auto it = enums_.rbegin(); it != enums_.rend(); ++it) {
        if (hash == tinytc::fnv1a((*it)->name()) && name == (*it)->name()) {
            return it->get();
        }
    }
    return nullptr;
}
auto objects::find_inst(std::string_view name) -> inst * { return find_in_list(insts_, name); }
auto objects::find_type(std::string_view name) -> type * { return find_in_list(types_, name); }

} // namespace mochi
