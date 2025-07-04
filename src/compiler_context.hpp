// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_CONTEXT_20240924_HPP
#define COMPILER_CONTEXT_20240924_HPP

#include "compiler_context_cache.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tinytc {
void default_error_reporter(char const *what, const tinytc_location_t *location, void *user_data);
} // namespace tinytc

struct tinytc_compiler_context : tinytc::reference_counted {
  public:
    constexpr static const char unavailable_source_name[] = "Source name unavailable";
    constexpr static std::array<std::array<bool, TINYTC_ENUM_NUM_OPTFLAG>, 3u> default_opt_flags = {
        {{false}, {false}, {true}}};

    tinytc_compiler_context();

    inline auto cache() -> tinytc::compiler_context_cache * { return cache_.get(); }

    inline void set_error_reporter(tinytc_error_reporter_t reporter, void *user_data) {
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
    void report_error(tinytc_location const &l,
                      tinytc::array_view<const_tinytc_value_t> const &ref_values, char const *what);
    void report_error(tinytc_location const &l,
                      tinytc::array_view<const_tinytc_value_t> const &ref_values);

    auto opt_flag(tinytc_optflag_t flag) const -> bool;
    inline void opt_flag(tinytc_optflag_t flag, std::int32_t state) { opt_flags_[flag] = state; }
    inline auto opt_flag(tinytc::optflag flag) const -> bool {
        return opt_flag(static_cast<tinytc_optflag_t>(flag));
    }
    inline void opt_flag(tinytc::optflag flag, std::int32_t state) {
        opt_flag(static_cast<tinytc_optflag_t>(flag), state);
    }

    inline auto opt_level() const noexcept -> std::int32_t { return opt_level_; }
    inline void opt_level(std::int32_t level) noexcept { opt_level_ = level; }

    inline auto index_bit_width() const noexcept -> std::size_t { return 64; }

  private:
    struct source_input {
        std::string name, text;
    };

    inline bool has_source_id(std::int32_t source_id) const {
        return source_id >= 1 && static_cast<std::size_t>(source_id) <= sources_.size();
    }

    std::unique_ptr<tinytc::compiler_context_cache> cache_;
    tinytc_error_reporter_t reporter_ = &tinytc::default_error_reporter;
    void *user_data_ = nullptr;
    std::vector<source_input> sources_;
    std::array<std::int32_t, TINYTC_ENUM_NUM_OPTFLAG> opt_flags_;
    std::int32_t opt_level_ = 2;
};

#endif // COMPILER_CONTEXT_20240924_HPP
