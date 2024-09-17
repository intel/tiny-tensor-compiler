// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/insert_barrier.hpp"
#include "analysis/alias.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/builtin_type.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace tinytc {

auto intersects(std::unordered_set<::tinytc_value const *> const &a,
                std::unordered_set<::tinytc_value const *> const &b, aa_results const &aa) {
    for (auto &av : a) {
        for (auto &bv : b) {
            if (aa.alias(*av, *bv)) {
                return true;
            }
        }
    }
    return false;
}

void insert_barrier_pass::reads_writes::clear(address_space as) {
    const auto space = address_space_to_index(as);
    reads[space].clear();
    writes[space].clear();
}

void insert_barrier_pass::reads_writes::merge(reads_writes &&other) {
    for (std::size_t i = 0; i < reads.size(); ++i) {
        reads[i].merge(std::move(other.reads[i]));
    }
    for (std::size_t i = 0; i < reads.size(); ++i) {
        writes[i].merge(std::move(other.writes[i]));
    }
}

void insert_barrier_pass::reads_writes::emplace_read(address_space as, ::tinytc_value const *val) {
    const auto space = address_space_to_index(as);
    reads[space].emplace(val);
}
void insert_barrier_pass::reads_writes::emplace_write(address_space as, ::tinytc_value const *val) {
    const auto space = address_space_to_index(as);
    writes[space].emplace(val);
}

bool insert_barrier_pass::reads_writes::raw(address_space as, reads_writes const &rw,
                                            aa_results const &aa) {
    const auto space = address_space_to_index(as);
    return intersects(rw.reads[space], writes[space], aa);
}
bool insert_barrier_pass::reads_writes::war(address_space as, reads_writes const &rw,
                                            aa_results const &aa) {
    const auto space = address_space_to_index(as);
    return intersects(rw.writes[space], reads[space], aa);
}
bool insert_barrier_pass::reads_writes::waw(address_space as, reads_writes const &rw,
                                            aa_results const &aa) {
    const auto space = address_space_to_index(as);
    return intersects(rw.writes[space], writes[space], aa);
}
bool insert_barrier_pass::reads_writes::raw_war_or_waw(address_space as, reads_writes const &rw,
                                                       aa_results const &aa) {
    return raw(as, rw, aa) || war(as, rw, aa) || waw(as, rw, aa);
}

auto insert_barrier_pass::reads_writes::address_space_to_index(address_space as) -> std::size_t {
    for (std::size_t i = 0; i < address_spaces.size(); ++i) {
        if (as == address_spaces[i]) {
            return i;
        }
    }
    throw internal_compiler_error{};
}

auto insert_barrier_pass::run_on_region(rgn &reg, aa_results const &aa,
                                        const bool insert_barriers) -> reads_writes {
    auto invisible_rw = reads_writes{};
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        if (auto *barrier = dyn_cast<barrier_inst>(it->get()); insert_barriers && barrier) {
            for (auto &as : reads_writes::address_spaces) {
                if (barrier->has_fence(as)) {
                    invisible_rw.clear(as);
                }
            }
        } else {
            auto rw = reads_writes{};
            for (auto &subreg : (*it)->child_regions()) {
                const bool insert_barriers_sub =
                    insert_barriers && subreg->kind() != region_kind::spmd;
                rw.merge(run_on_region(*subreg, aa, insert_barriers_sub));
            }

            auto const emplace_read = [&rw](value const &v) {
                if (auto *m = dyn_cast<memref_data_type>(v->ty().get()); m) {
                    rw.emplace_read(m->addrspace(), v.get());
                }
            };
            auto const emplace_write = [&rw](value const &v) {
                if (auto *m = dyn_cast<memref_data_type>(v->ty().get()); m) {
                    rw.emplace_write(m->addrspace(), v.get());
                }
            };
            visit(overloaded{[&](blas_a2_inst &in) {
                                 emplace_read(in.A());
                                 emplace_write(in.B());
                             },
                             [&](blas_a3_inst &in) {
                                 emplace_read(in.A());
                                 emplace_read(in.B());
                                 emplace_write(in.C());
                             },
                             [&](load_inst &in) { emplace_read(in.operand()); },
                             [&](store_inst &in) { emplace_write(in.operand()); },
                             [](inst_node &) {}},
                  **it);

            if (insert_barriers) {
                std::int32_t fence_flags = 0;
                for (auto &as : reads_writes::address_spaces) {
                    if (invisible_rw.raw_war_or_waw(as, rw, aa)) {
                        fence_flags |= static_cast<std::int32_t>(as);
                        invisible_rw.clear(as);
                    }
                }
                if (fence_flags != 0) {
                    it =
                        reg.insert(it, inst{std::make_unique<barrier_inst>(fence_flags).release()});
                    ++it; // skip over barrier
                }
            }

            invisible_rw.merge(std::move(rw));
        }
    }

    return invisible_rw;
}

/* Function nodes */
void insert_barrier_pass::run_on_function(function &fn) {
    auto aa = alias_analysis{}.run_on_function(fn);
    run_on_region(*fn.body(), aa);
}

} // namespace tinytc
