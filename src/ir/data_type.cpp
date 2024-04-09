// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/data_type.hpp"
#include "ir/node/data_type_node.hpp"

#include <utility>

namespace tinytc::ir {

data_type::data_type(scalar_type type) : handle(std::make_shared<scalar_data_type>(type)) {}

data_type void_type() { return data_type(std::make_shared<void_data_type>()); }

data_type memref_type(scalar_type scalar_ty, std::vector<std::int64_t> shape,
                      std::vector<std::int64_t> stride, location const &lc) {
    return data_type(
        std::make_shared<memref_data_type>(scalar_ty, std::move(shape), std::move(stride), lc));
}

data_type group_type(data_type ty) {
    return data_type(std::make_shared<group_data_type>(std::move(ty)));
}

} // namespace tinytc::ir
