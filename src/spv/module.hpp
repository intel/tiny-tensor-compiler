// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MODULE_20241029_HPP
#define MODULE_20241029_HPP

#include "reference_counted.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"

#include <array>
#include <cstddef>

namespace tinytc {

namespace spv {
class spv_inst;
}

template <> struct ilist_callbacks<spv::spv_inst> {
    void node_added(spv::spv_inst *node);
    void node_removed(spv::spv_inst *node);
};

namespace spv {

enum class section {
    capability = 0,
    ext_inst = 1,
    memory_model = 2,
    entry_point = 3,
    execution_mode = 4,
    decoration = 5,
    type_const_var = 6,
    function = 7
};
inline constexpr std::size_t num_module_sections = 8;

class mod final {
  public:
    using iterator = ilist<spv_inst>::iterator;
    using const_iterator = ilist<spv_inst>::const_iterator;

    mod(std::int32_t major_version = 1, std::int32_t minor_version = 6);
    ~mod();

    auto bound() const -> std::int32_t;

    inline auto insts(section s) -> ilist<spv_inst> & { return insts_[static_cast<int>(s)]; }
    inline auto insts(section s) const -> ilist<spv_inst> const & {
        return insts_[static_cast<int>(s)];
    }
    inline auto empty(section s) const -> bool { return insts_[static_cast<int>(s)].empty(); }

    inline auto major_version() const -> std::int32_t { return major_version_; }
    inline auto minor_version() const -> std::int32_t { return minor_version_; }

  private:
    std::array<ilist<spv_inst>, num_module_sections> insts_;
    std::int32_t major_version_, minor_version_;
};

} // namespace spv
} // namespace tinytc

#endif // MODULE_20241029_HPP
