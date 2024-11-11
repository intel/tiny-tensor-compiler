// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ASSIGN_IDS_20241111_HPP
#define ASSIGN_IDS_20241111_HPP

#include "spv/defs.hpp"
#include "spv/instructions.hpp"
#include "spv/visit.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <unordered_map>

namespace tinytc::spv {

class id_assigner : public default_visitor<id_assigner, false> {
  public:
    using default_visitor<id_assigner, false>::operator();

    void visit_result(spv_inst &in);

    // Do nothing by default
    template <typename T> void operator()(T &) {}

    void operator()(spv_inst *&in);
    void operator()(OpPhi &in);

    void run_on_module(tinytc_spv_mod &m);

  private:
    void declare(spv_inst *in);

    std::uint32_t slot_ = 1;
    std::unordered_map<spv_inst const *, std::uint32_t> slot_map_;
};

} // namespace tinytc::spv

#endif // ASSIGN_IDS_20241111_HPP
