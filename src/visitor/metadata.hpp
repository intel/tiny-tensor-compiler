// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef METADATA_20240412_HPP
#define METADATA_20240412_HPP

#include "kernel_metadata.hpp"
#include "node/function_node.hpp"
#include "node/program_node.hpp"

#include <cstdint>
#include <unordered_map>

namespace tinytc {

class metadata {
  public:
    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

    inline auto get_result() const -> std::unordered_map<std::string, kernel_metadata> {
        return metadata_;
    }

  private:
    std::unordered_map<std::string, kernel_metadata> metadata_;
};

} // namespace tinytc

#endif // METADATA_20240412_HPP
