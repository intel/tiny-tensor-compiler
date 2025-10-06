// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/inst.hpp"
#include "error.hpp"
#include "node/region.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "tinytc/builder.h"
#include "tinytc/types.hpp"
#include "util/overloaded.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>

using namespace tinytc;

static_assert(alignof(tinytc_value) == alignof(tinytc_inst));
static_assert(alignof(use) == alignof(tinytc_inst));
static_assert(alignof(tinytc_region) == alignof(tinytc_inst));
static_assert(alignof(tinytc_inst) <= alignof(std::max_align_t));

auto tinytc_inst::create(IK tid, inst_layout layout, tinytc_location const &lc) -> tinytc_inst_t {
    std::size_t size = 0;
    size += sizeof(tinytc_value) * layout.num_results;
    size += sizeof(tinytc_inst);
    size += sizeof(use) * layout.num_operands;
    size += layout.sizeof_properties;
    size += sizeof(tinytc_region) * layout.num_child_regions;

    struct del {
        void operator()(std::uint8_t *p) const { std::free(p); }
    };
    auto raw_mem =
        std::unique_ptr<std::uint8_t, del>(static_cast<std::uint8_t *>(std::malloc(size)));
    if (raw_mem.get() == nullptr) {
        throw status::bad_alloc;
    }

    // initialize results
    tinytc_value_t first_result = reinterpret_cast<tinytc_value_t>(raw_mem.get());
    tinytc_value_t last_result = first_result + layout.num_results;
    for (; first_result != last_result; ++first_result) {
        new (first_result) tinytc_value();
    }

    // initialize inst
    tinytc_inst_t in = reinterpret_cast<tinytc_inst_t>(last_result);
    new (in) tinytc_inst(tid, layout, lc);

    // initialize uses
    use *first_use = reinterpret_cast<use *>(in + 1);
    use *last_use = first_use + layout.num_operands;
    for (; first_use != last_use; ++first_use) {
        new (first_use) use(in);
    }

    // properties
    std::uint8_t *first_prop = reinterpret_cast<std::uint8_t *>(last_use);
    std::uint8_t *last_prop = first_prop + layout.sizeof_properties;
    if (layout.sizeof_properties > 0) {
        visit(overloaded{[&](auto view) {
                  std::construct_at(reinterpret_cast<decltype(view)::properties *>(first_prop));
              }},
              *in);
    }

    // child regions
    tinytc_region_t first_region = reinterpret_cast<tinytc_region_t>(last_prop);
    tinytc_region_t last_region = first_region + layout.num_child_regions;
    for (; first_region != last_region; ++first_region) {
        new (first_region) tinytc_region(in);
    }

    raw_mem.release();
    return in;
}

void tinytc_inst::destroy(tinytc_inst_t in) {
    void *raw_mem = reinterpret_cast<tinytc_value_t>(in) - in->layout_.num_results;
    in->~tinytc_inst();
    std::free(raw_mem);
}

tinytc_inst::~tinytc_inst() {
    // child regions
    for (tinytc_region_t r = child_region_ptr(0); r != child_region_ptr(layout_.num_child_regions);
         ++r) {
        std::destroy_at(r);
    }

    // properties
    if (layout_.sizeof_properties > 0) {
        visit_noexcept(overloaded{[&](auto view) {
                           std::destroy_at(static_cast<decltype(view)::properties *>(props()));
                       }},
                       *this);
    }

    // uses
    for (use *u = use_ptr(0); u != use_ptr(layout_.num_operands); ++u) {
        std::destroy_at(u);
    }

    // results
    for (tinytc_value_t r = result_ptr(0); r != result_ptr(layout_.num_results); --r) {
        std::destroy_at(r);
    }
}

auto tinytc_inst::context() -> tinytc_compiler_context_t {
    if (num_results() > 0) {
        return result(0).context();
    } else if (num_operands() > 0) {
        return op(0).context();
    }
    return nullptr;
}

void tinytc_inst::subs(tinytc_value_t old_value, tinytc_value_t new_value, bool recursive) {
    for (auto u = use_ptr(0); u != use_ptr(layout_.num_operands); ++u) {
        if (u->get() == old_value) {
            u->set(new_value);
        }
    }
    if (recursive) {
        for (auto &reg : child_regions()) {
            for (auto &in : reg) {
                in.subs(old_value, new_value, true);
            }
        }
    }
}

void tinytc_inst::op(std::size_t pos, tinytc_value_t val) {
    if (val == nullptr) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    *use_ptr(pos) = val;
}

void tinytc_inst::result(std::size_t pos, tinytc_type_t ty) {
    if (ty == nullptr) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    *result_ptr(pos) = tinytc_value{ty, this, loc()};
}

auto tinytc_inst::child_region_ptr(std::int32_t no) -> tinytc_region_t {
    std::uint8_t *props_end = static_cast<std::uint8_t *>(props()) + layout_.sizeof_properties;
    return reinterpret_cast<tinytc_region_t>(props_end) + no;
}

extern "C" {

void tinytc_inst_destroy(tinytc_inst_t obj) { tinytc_inst::destroy(obj); }

tinytc_status_t tinytc_inst_get_parent_region(tinytc_inst_t instr, tinytc_region_t *parent) {
    if (instr == nullptr || parent == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *parent = instr->parent(); });
}

tinytc_status_t tinytc_inst_get_values(tinytc_inst_t instr, size_t *result_list_size,
                                       tinytc_value_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_results();
        if (num_results < 0) {
            throw std::out_of_range("Number of results must not be negative");
        }
        auto num = static_cast<std::size_t>(num_results);
        if (*result_list_size > 0) {
            num = std::min(num, *result_list_size);
            auto results = instr->result_begin();
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}

tinytc_status_t tinytc_inst_get_regions(tinytc_inst_t instr, size_t *result_list_size,
                                        tinytc_region_t *result_list) {
    if (instr == nullptr || result_list_size == nullptr ||
        (*result_list_size > 0 && result_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const num_results = instr->num_child_regions();
        if (num_results < 0) {
            throw std::out_of_range("Number of results must not be negative");
        }
        auto num = static_cast<std::size_t>(num_results);
        if (*result_list_size > 0) {
            auto results = instr->child_regions_begin();
            num = std::min(num, *result_list_size);
            for (uint32_t i = 0; i < num; ++i) {
                result_list[i] = &results[i];
            }
        }
        *result_list_size = num;
    });
}

tinytc_status_t tinytc_inst_set_attr(tinytc_inst_t instr, tinytc_attr_t a) {
    if (instr == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { instr->attr(a); });
}
}
