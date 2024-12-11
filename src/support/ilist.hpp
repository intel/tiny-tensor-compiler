// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ILIST_20240923_HPP
#define ILIST_20240923_HPP

#include "support/ilist_base.hpp"

namespace tinytc {

template <typename NodeT> struct ilist_callbacks {
    void node_added(NodeT *) {}
    void node_moved(NodeT *) {}
    void node_removed(NodeT *) {}
};

template <typename NodeT, typename IListCallback = ilist_callbacks<NodeT>,
          ilist_clear_order ClearOrder = ilist_clear_order::reverse>
class ilist : public ilist_base<NodeT, IListCallback, ClearOrder> {
  public:
    ilist() = default;
};

} // namespace tinytc

#endif // ILIST_20240923_HPP
