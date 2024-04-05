// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/inst.hpp"

#include "clir/virtual_type_list.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace tinytc::ir::internal {

class TINYTC_EXPORT region_node : public clir::virtual_type_list<class rgn> {
  public:
};

class TINYTC_EXPORT rgn : public clir::visitable<rgn, region_node> {
  public:
    inline rgn(std::vector<inst> insts = {}) : insts_(std::move(insts)) {}

    inline std::vector<inst> &insts() { return insts_; }
    inline void insts(std::vector<inst> insts) { insts_ = std::move(insts); }

  private:
    std::vector<inst> insts_;
};

} // namespace tinytc::ir::internal

#endif // REGION_NODE_20230908_HPP
