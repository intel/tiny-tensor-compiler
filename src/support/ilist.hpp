// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ILIST_20240923_HPP
#define ILIST_20240923_HPP

#include "support/ilist_base.hpp"

namespace tinytc {

template <typename NodeT> struct ilist_callbacks {
    void node_added(NodeT *) {}
    void node_removed(NodeT *) {}
};

template <typename NodeT, typename IListCallback = ilist_callbacks<NodeT>>
class ilist : public ilist_base<NodeT, IListCallback> {
  public:
    ilist() = default;

    ilist(ilist const &other) = delete;
    ilist(ilist &&other) = default;
    ilist &operator=(ilist const &other) = delete;
    ilist &operator=(ilist &&other) = default;
};

} // namespace tinytc

#endif // ILIST_20240923_HPP
