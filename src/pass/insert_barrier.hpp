// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INSERT_BARRIER_20230310_HPP
#define INSERT_BARRIER_20230310_HPP

#include "analysis/aa_results.hpp"
#include "node/function_node.hpp"
#include "node/region_node.hpp"

#include <cstddef>
#include <unordered_set>

namespace tinytc {

class insert_barrier_pass {
  public:
    void run_on_function(function_node &fn);

  private:
    class reads_writes {
      public:
        constexpr static std::array<address_space, 2u> address_spaces = {address_space::global,
                                                                         address_space::local};

        void clear(address_space as);
        void merge(reads_writes &&other);
        void emplace_read(address_space as, ::tinytc_value const *val);
        void emplace_write(address_space as, ::tinytc_value const *val);

        bool raw(address_space as, reads_writes const &rw, aa_results const &aa);
        bool war(address_space as, reads_writes const &rw, aa_results const &aa);
        bool waw(address_space as, reads_writes const &rw, aa_results const &aa);
        bool raw_war_or_waw(address_space as, reads_writes const &rw, aa_results const &aa);

      private:
        auto address_space_to_index(address_space as) -> std::size_t;

        std::array<std::unordered_set<::tinytc_value const *>, address_spaces.size()> reads, writes;
    };

    auto run_on_region(region_node &reg, aa_results const &aa,
                       const bool insert_barriers = true) -> reads_writes;
};

} // namespace tinytc

#endif // INSERT_BARRIER_20230310_HPP
