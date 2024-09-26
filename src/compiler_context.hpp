// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_CONTEXT_20240924_HPP
#define COMPILER_CONTEXT_20240924_HPP

#include "reference_counted.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
void default_error_reporter(char const *what, const tinytc_location_t *location, void *user_data);

class compiler_context_cache;

} // namespace tinytc

struct tinytc_compiler_context : tinytc::reference_counted {
  public:
    constexpr static const char unavailable_source_name[] = "Source name unavailable";

    tinytc_compiler_context();

    inline auto cache() -> tinytc::compiler_context_cache * { return cache_.get(); }

    inline void set_error_reporter(tinytc::error_reporter_t reporter, void *user_data) {
        reporter_ = reporter;
        user_data_ = user_data;
    }

    // source / error handling
    inline auto add_source(std::string name, std::string text) -> std::int32_t {
        sources_.emplace_back(source_input{std::move(name), std::move(text)});
        return static_cast<std::int32_t>(sources_.size());
    }
    inline auto add_source(char const *name, char const *text) -> std::int32_t {
        sources_.emplace_back(source_input{std::string(name), std::string(text)});
        return static_cast<std::int32_t>(sources_.size());
    }
    auto source_name(std::int32_t source_id) -> std::pair<char const *, std::size_t>;
    auto source_text(std::int32_t source_id) -> std::pair<char const *, std::size_t>;
    void report_error(tinytc_location const &l, char const *what);

  private:
    struct source_input {
        std::string name, text;
    };

    inline bool has_source_id(std::int32_t source_id) const {
        return source_id >= 1 && static_cast<std::size_t>(source_id) <= sources_.size();
    }

    std::unique_ptr<tinytc::compiler_context_cache> cache_;
    tinytc::error_reporter_t reporter_ = &tinytc::default_error_reporter;
    void *user_data_ = nullptr;
    std::vector<source_input> sources_;
};

#endif // COMPILER_CONTEXT_20240924_HPP
