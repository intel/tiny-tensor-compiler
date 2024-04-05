// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WORK_GROUP_SIZE_20240311_HPP
#define WORK_GROUP_SIZE_20240311_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/internal/function_node.hpp"
#include "tinytc/ir/internal/inst_node.hpp"
#include "tinytc/ir/internal/program_node.hpp"
#include "tinytc/ir/internal/region_node.hpp"
#include "tinytc/ir/tiling.hpp"

#include <memory>
#include <unordered_set>

namespace tinytc {
class core_info;
}

namespace tinytc::ir::internal {

class TINYTC_EXPORT work_group_size {
  public:
    work_group_size(std::shared_ptr<core_info> info);

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
    std::shared_ptr<core_info> info_;
    std::unordered_set<blas_shape> shapes_;
};

} // namespace tinytc::ir::internal

#endif // WORK_GROUP_SIZE_20240311_HPP
