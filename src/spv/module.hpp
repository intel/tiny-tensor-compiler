// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MODULE_20241029_HPP
#define MODULE_20241029_HPP

#include "reference_counted.hpp"
#include "spv/defs.hpp"
#include "support/ilist.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

namespace tinytc {

template <> struct ilist_callbacks<spv::spv_inst> {
    void node_added(spv::spv_inst *node);
    void node_moved(spv::spv_inst *node);
    void node_removed(spv::spv_inst *node);
};

namespace spv {

enum class section {
    capability = 0,
    extension = 1,
    ext_inst = 2,
    memory_model = 3,
    entry_point = 4,
    execution_mode = 5,
    decoration = 6,
    type_const_var = 7,
    function = 8
};
inline constexpr std::int32_t num_module_sections = 9;

} // namespace spv
} // namespace tinytc

struct tinytc_spv_mod final : tinytc::reference_counted {
  public:
    using iterator = tinytc::ilist<tinytc::spv::spv_inst>::iterator;
    using const_iterator = tinytc::ilist<tinytc::spv::spv_inst>::const_iterator;

    tinytc_spv_mod(tinytc::compiler_context ctx, tinytc_core_feature_flags_t core_features,
                   std::int32_t major_version = 1, std::int32_t minor_version = 6);
    ~tinytc_spv_mod();

    inline auto context() const -> tinytc_compiler_context_t { return ctx_.get(); }
    inline auto share_context() const -> tinytc::compiler_context { return ctx_; }
    inline auto core_features() const -> tinytc_core_feature_flags_t { return core_features_; }

    auto bound() const -> std::uint32_t;

    inline auto insts(tinytc::spv::section s) -> tinytc::ilist<tinytc::spv::spv_inst> & {
        return insts_[static_cast<int>(s)];
    }
    inline auto insts(tinytc::spv::section s) const
        -> tinytc::ilist<tinytc::spv::spv_inst> const & {
        return insts_[static_cast<int>(s)];
    }
    inline auto empty(tinytc::spv::section s) const -> bool {
        return insts_[static_cast<int>(s)].empty();
    }

    inline auto major_version() const -> std::int32_t { return major_version_; }
    inline auto minor_version() const -> std::int32_t { return minor_version_; }

    template <typename T, typename... Args>
    auto add_to(tinytc::spv::section s, Args &&...args) -> T * {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...).release();
        insts(s).push_back(ptr);
        return ptr;
    }
    template <typename T, typename... Args> auto add(Args &&...args) -> T * {
        return add_to<T>(tinytc::spv::section::function, std::forward<Args>(args)...);
    }

  private:
    tinytc::compiler_context ctx_;
    tinytc_core_feature_flags_t core_features_;
    std::array<tinytc::ilist<tinytc::spv::spv_inst>, tinytc::spv::num_module_sections> insts_;
    std::int32_t major_version_, minor_version_;
};

namespace tinytc::spv {
// using mod = ::tinytc_spv_mod;
} // namespace tinytc::spv

#endif // MODULE_20241029_HPP
