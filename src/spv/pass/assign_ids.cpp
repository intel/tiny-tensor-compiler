// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/assign_ids.hpp"
#include "spv/module.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "tinytc/types.hpp"

#include <utility>
#include <vector>

namespace tinytc::spv {

void id_assigner::declare(spv_inst *in) {
    if (!slot_map_.contains(in)) {
        const auto slot = slot_++;
        slot_map_[in] = slot;
        in->id(slot);
    }
}

void id_assigner::visit_result(spv_inst &in) { declare(&in); }

void id_assigner::operator()(spv_inst *&in) {
    if (!slot_map_.contains(in)) {
        if (isa<OpFunction>(*in) || isa<OpVariable>(*in) || isa<OpLabel>(*in) ||
            isa<OpTypePointer>(*in)) {
            declare(in);
        } else {
            throw status::spirv_forbidden_forward_declaration;
        }
    }
}

void id_assigner::operator()(OpPhi &in) {
    pre_visit(in);
    this->operator()(in.type());
    this->visit_result(in);
    for (auto &op : in.op0()) {
        // Forward references are allowed in phi instructions
        declare(op.first);
        this->operator()(op);
    }
    post_visit(in);
}

void id_assigner::run_on_module(tinytc_spv_mod &m) {
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto &i : m.insts(enum_cast<section>(s))) {
            visit(*this, i);
        }
    }
}

} // namespace tinytc::spv

