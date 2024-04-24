// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WORK_GROUP_SIZE_20240311_HPP
#define WORK_GROUP_SIZE_20240311_HPP

#include "device_info.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "tiling.hpp"

#include <unordered_set>

namespace tinytc {

class work_group_size {
  public:
    work_group_size(tinytc_core_info const *info);

    /* Inst nodes */
    void operator()(inst_node &);
    void operator()(blas_a2_inst &in);
    void operator()(blas_a3_inst &in);
    void operator()(if_inst &in);
    void operator()(loop_inst &in);

    /* Region nodes */
    void operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    tinytc_core_info const *info_;
    std::unordered_set<blas_shape> shapes_;
};

} // namespace tinytc

#endif // WORK_GROUP_SIZE_20240311_HPP
