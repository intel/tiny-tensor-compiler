// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DATA_TYPE_20230309_HPP
#define DATA_TYPE_20230309_HPP

/*#include "tinytc/export.h"
#include "tinytc/ir/location.hpp"

#include <clir/handle.hpp>*/

#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <limits>
/*#include <memory>
#include <vector>*/

// struct tinytc_data_type;

namespace tinytc {

// enum class scalar_type;
//! Special int64 value reserved for dynamic modes ('?')
constexpr static std::int64_t dynamic = std::numeric_limits<std::int64_t>::min();

//! Check if mode i is dynamic ('?')
inline bool is_dynamic_value(std::int64_t i) { return i == dynamic; }

//! Reference-counted data type handle
/*class TINYTC_EXPORT data_type : public clir::handle<::tinytc_data_type> {
  public:
    using clir::handle<::tinytc_data_type>::handle;
    //! ctor; create from scalar_type
    data_type(scalar_type type);
};

//! Create void type
TINYTC_EXPORT data_type void_type();
//! @code memref<%scalar_ty x %shape, strided<%stride>> @endcode
TINYTC_EXPORT data_type memref_type(scalar_type scalar_ty, std::vector<std::int64_t> shape,
                                    std::vector<std::int64_t> stride = {}, location const &lc = {});
//! @code group<%ty> @endcode
TINYTC_EXPORT data_type group_type(data_type ty);*/

} // namespace tinytc

#endif // DATA_TYPE_20230309_HPP