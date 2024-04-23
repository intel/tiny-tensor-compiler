// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REFERENCE_COUNTED_20240409_HPP
#define REFERENCE_COUNTED_20240409_HPP

#include <atomic>
#include <cstdint>

namespace tinytc {

struct reference_counted {
  public:
    inline reference_counted(std::uint64_t initial = 1) : ref_count{initial} {}
    ~reference_counted() = default;
    reference_counted(reference_counted const &other) = delete;
    reference_counted(reference_counted &&other) = delete;
    reference_counted &operator=(reference_counted const &other) = delete;
    reference_counted &operator=(reference_counted &&other) = delete;

    [[maybe_unused]] inline auto inc_ref() noexcept { return ++ref_count; }
    [[maybe_unused]] inline auto dec_ref() noexcept { return --ref_count; }

  private:
    std::atomic<std::uint64_t> ref_count;
};

} // namespace tinytc

#endif // REFERENCE_COUNTED_20240409_HPP
