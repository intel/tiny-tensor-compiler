// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UNIQUIFIER_20241107_HPP
#define UNIQUIFIER_20241107_HPP

#include "spv/defs.hpp"
#include "spv/enums.hpp"
#include "support/fnv1a.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace tinytc {
enum class address_space;
enum class scalar_type;
enum class vector_size;
class matrix_ext_info;
} // namespace tinytc

namespace tinytc::spv {

auto address_space_to_storage_class(address_space as) -> StorageClass;

class uniquifier {
  public:
    uniquifier(tinytc_spv_mod &m, matrix_ext_info const &matrix);

    auto asm_target() -> spv_inst *;
    auto bool2_ty() -> spv_inst *;
    auto bool_constant(bool b) -> spv_inst *;
    auto builtin_alignment(BuiltIn b) -> std::int32_t;
    auto builtin_pointee_ty(BuiltIn b) -> spv_inst *;
    auto builtin_var(BuiltIn b) -> spv_inst *;
    void capability(Capability cap);
    auto constant(LiteralContextDependentNumber cst) -> spv_inst *;
    void extension(char const *ext_name);
    auto index3_ty() -> spv_inst *;
    auto null_constant(spv_inst *spv_ty) -> spv_inst *;
    auto opencl_ext() -> spv_inst *;
    auto spv_array_ty(spv_inst *element_ty, std::int32_t length) -> spv_inst *;
    auto spv_function_ty(spv_inst *return_ty, array_view<spv_inst *> params) -> spv_inst *;
    auto spv_pointer_ty(StorageClass cls, spv_inst *pointee_ty, std::int32_t alignment)
        -> spv_inst *;
    auto spv_ty(const_tinytc_data_type_t ty) -> spv_inst *;
    auto spv_ty(scalar_type sty) -> spv_inst *;
    auto spv_vec_ty(spv_inst *component_ty, std::int32_t length) -> spv_inst *;
    auto spv_vec_ty(spv_inst *component_ty, vector_size length) -> spv_inst *;
    auto void_ty() -> spv_inst *;

  private:
    struct array_key_hash {
        inline auto operator()(std::pair<spv_inst *, std::int32_t> const &key) const
            -> std::size_t {
            return fnv1a_combine(key.first, key.second);
        }
    };
    struct pointer_key_hash {
        inline auto operator()(std::tuple<StorageClass, spv_inst *, std::int32_t> const &key) const
            -> std::size_t {
            return fnv1a_combine(std::get<0>(key), std::get<1>(key), std::get<2>(key));
        }
    };

    tinytc_spv_mod_t mod_;
    matrix_ext_info const *matrix_;
    spv_inst *asm_target_ = nullptr;
    spv_inst *bool_true_ = nullptr, *bool_false_ = nullptr;
    spv_inst *opencl_ext_ = nullptr;
    std::unordered_map<BuiltIn, spv_inst *> builtin_;
    std::unordered_set<Capability> capabilities_;
    std::unordered_map<LiteralContextDependentNumber, spv_inst *> cst_map_;
    std::unordered_set<char const *> extensions_;
    std::unordered_map<spv_inst *, spv_inst *> null_cst_;
    std::unordered_map<std::pair<spv_inst *, std::int32_t>, spv_inst *, array_key_hash>
        spv_array_tys_;
    std::unordered_multimap<std::uint64_t, OpTypeFunction *> spv_function_tys_;
    std::unordered_map<std::pair<spv_inst *, std::int32_t>, spv_inst *, array_key_hash>
        spv_vec_tys_;
    std::unordered_map<std::tuple<StorageClass, spv_inst *, std::int32_t>, spv_inst *,
                       pointer_key_hash>
        spv_pointer_tys_;
    std::unordered_map<const_tinytc_data_type_t, spv_inst *> spv_tys_;
    std::unordered_map<const_tinytc_data_type_t, spv_inst *> spv_matrix_tys_;
};

} // namespace tinytc::spv

#endif // UNIQUIFIER_20241107_HPP
