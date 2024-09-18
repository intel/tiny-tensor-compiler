// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOWER_LINALG_20240801_HPP
#define LOWER_LINALG_20240801_HPP

#include "device_info.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tiling.hpp"
#include "tinytc/types.h"

#include <vector>

namespace tinytc {

class lower_linalg_pass {
  public:
    lower_linalg_pass(::tinytc_core_info const *info);

    /* Data type nodes */
    // bool operator()(void_data_type &);
    // bool operator()(group_data_type &b);
    // bool operator()(memref_data_type &m);
    // bool operator()(scalar_data_type &s);

    //[> Value nodes <]
    // value_node *operator()(int_imm &v);
    // value_node *operator()(float_imm &v);
    // value_node *operator()(val &v);

    /* Stmt nodes */
    inst operator()(inst_node &);
    inst operator()(loop_inst &p);
    inst operator()(ger_inst &g);
    inst operator()(if_inst &in);
    inst operator()(parallel_inst &p);

    /* Region nodes */
    void operator()(region_node &b);

    /* Func nodes */
    void operator()(function_node &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    auto get_memref_type(value_node const &v) const -> const memref_data_type *;

    ::tinytc_core_info const *info_;
    local_tiling tiling_ = {};
    core_config core_cfg_ = {};
};

} // namespace tinytc

#endif // LOWER_LINALG_20240801_HPP
