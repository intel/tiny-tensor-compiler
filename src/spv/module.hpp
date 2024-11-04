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
    memory_model = 1,
    entry_point = 2,
    execution_mode = 3,
    decoration = 4,
    type = 5,
    function = 6
};
inline constexpr std::size_t num_module_sections = 7;

class mod final {
  public:
    using iterator = ilist<spv_inst>::iterator;
    using const_iterator = ilist<spv_inst>::const_iterator;

    mod();
    ~mod();

    inline auto insts(section s) -> ilist<spv_inst> & { return insts_[static_cast<int>(s)]; }
    inline auto insts(section s) const -> ilist<spv_inst> const & {
        return insts_[static_cast<int>(s)];
    }
    inline auto empty(section s) const -> bool { return insts_[static_cast<int>(s)].empty(); }

  private:
    std::array<ilist<spv_inst>, num_module_sections> insts_;
};

} // namespace spv
} // namespace tinytc

#endif // MODULE_20241029_HPP
