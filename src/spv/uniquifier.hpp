// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UNIQUIFIER_20241107_HPP
#define UNIQUIFIER_20241107_HPP

#include "spv/defs.hpp"
#include "spv/enums.hpp"
#include "spv/module.hpp"
#include "support/fnv1a.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tinytc::spv {

class OpTypeFunction;

class uniquifier {
  public:
    uniquifier(tinytc_compiler_context_t ctx, mod &m);

    auto bool2_ty() -> spv_inst *;
    auto bool_constant(bool b) -> spv_inst *;
    auto builtin_alignment(BuiltIn b) -> std::int32_t;
    auto builtin_pointee_ty(BuiltIn b) -> spv_inst *;
    auto builtin_var(BuiltIn b) -> spv_inst *;
    void capability(Capability cap);
    auto constant(LiteralContextDependentNumber cst) -> spv_inst *;
    auto index3_ty() -> spv_inst *;
    auto null_constant(spv_inst *spv_ty) -> spv_inst *;
    auto opencl_ext() -> spv_inst *;
    auto spv_function_ty(array_view<spv_inst *> params) -> spv_inst *;
    auto spv_pointer_ty(StorageClass cls, spv_inst *pointee_ty) -> spv_inst *;
    auto spv_ty(const_tinytc_data_type_t ty) -> spv_inst *;

  private:
    template <typename Map, typename Key, typename Maker>
    auto lookup(Map &map, Key &&key, Maker &&maker) {
        auto it = map.find(key);
        if (it == map.end()) {
            map[key] = maker(key);
            return map[key];
        }
        return it->second;
    }
    template <typename Maker> auto lookup(spv_inst *&var, Maker &&maker) -> spv_inst * {
        if (!var) {
            var = maker();
        }
        return var;
    }

    struct pointer_key_hash {
        inline auto
        operator()(std::pair<StorageClass, spv_inst *> const &key) const -> std::size_t {
            return fnv1a_combine(key.first, key.second);
        }
    };

    tinytc_compiler_context_t ctx_;
    mod *mod_;
    spv_inst *bool2_ty_ = nullptr;
    spv_inst *bool_true_ = nullptr, *bool_false_ = nullptr;
    spv_inst *index3_ty_ = nullptr;
    spv_inst *opencl_ext_ = nullptr;
    std::unordered_map<BuiltIn, spv_inst *> builtin_;
    std::unordered_set<Capability> capabilities_;
    std::unordered_map<LiteralContextDependentNumber, spv_inst *> cst_map_;
    std::unordered_map<spv_inst *, spv_inst *> null_cst_;
    std::unordered_multimap<std::uint64_t, OpTypeFunction *> spv_function_tys_;
    std::unordered_map<std::pair<StorageClass, spv_inst *>, spv_inst *, pointer_key_hash>
        spv_pointer_tys_;
    std::unordered_map<const_tinytc_data_type_t, spv_inst *> spv_tys_;
};

} // namespace tinytc::spv

#endif // UNIQUIFIER_20241107_HPP
