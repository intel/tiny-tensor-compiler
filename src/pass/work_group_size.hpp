// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WORK_GROUP_SIZE_20240311_HPP
#define WORK_GROUP_SIZE_20240311_HPP

#include "device_info.hpp"
#include "node/function_node.hpp"

namespace tinytc {

class work_group_size_pass {
  public:
    work_group_size_pass(tinytc_core_info const *info);

    void run_on_function(function &fn);

  private:
    tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // WORK_GROUP_SIZE_20240311_HPP