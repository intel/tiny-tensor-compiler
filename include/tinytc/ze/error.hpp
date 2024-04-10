// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// Code COPIED from Double-Batched FFT Library
// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_ERROR_20240305_HPP
#define ZE_ERROR_20240305_HPP

#include "tinytc/export.h"

#include <level_zero/ze_api.h>

#include <exception>
#include <string>

/**
 * @brief Checks Level Zero call for error and throws exception if call was not successful
 *
 * @param X Level Zero call or ze_result_t status code
 */
#define ZE_CHECK(X)                                                                                \
    [](ze_result_t status) {                                                                       \
        if (status != ZE_RESULT_SUCCESS) {                                                         \
            char what[256];                                                                        \
            snprintf(what, sizeof(what), "%s in %s on line %d returned %s (%d).\n", #X, __FILE__,  \
                     __LINE__, ::tinytc::ze_result_to_string(status), status);                     \
            throw ::tinytc::level_zero_error(what, status);                                        \
        }                                                                                          \
    }(X)

namespace tinytc {

/**
 * @brief Level Zero error
 */
class TINYTC_EXPORT level_zero_error : public std::exception {
  public:
    /**
     * @brief Constructor
     *
     * @param what Explanatory string
     * @param status Status code returned by Level Zero
     */
    level_zero_error(std::string what, ze_result_t status);
    /**
     * @brief Constructor
     *
     * @param what Explanatory string
     * @param status Status code returned by Level Zero
     */
    level_zero_error(char const *what, ze_result_t status);
    /**
     * @brief Explanation
     *
     * @return Explanatory string
     */
    char const *what() const noexcept override;
    /**
     * @brief Level Zero status code
     *
     * @return Status code
     */
    ze_result_t status_code() const noexcept;

  private:
    std::string what_;
    ze_result_t status_;
};

//! Convert Level Zero return code to string
TINYTC_EXPORT char const *ze_result_to_string(ze_result_t status);

} // namespace tinytc

#endif // ZE_ERROR_20240305_HPP
