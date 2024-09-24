// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/region_node.hpp"

namespace tinytc {

auto ilist_traits<tinytc_inst>::get_parent_region() -> tinytc_region * {
    return reinterpret_cast<tinytc_region *>(reinterpret_cast<char *>(this) -
                                             tinytc_region::inst_list_offset());
}

} // namespace tinytc

