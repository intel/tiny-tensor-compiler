// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/stack.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <list>

namespace tinytc {

void set_stack_ptr_pass::run_on_function(function_node &fn) {
    struct allocation {
        value_node *value;
        std::int64_t start, stop;
    };
    std::list<allocation> allocs;

    walk<walk_order::pre_order>(fn, [&allocs](inst_node &i) {
        visit(overloaded{
                  [&allocs](alloca_inst &a) {
                      auto t = dyn_cast<memref_data_type>(a.result()->ty());
                      if (t == nullptr) {
                          throw compilation_error(a.loc(), status::ir_expected_memref);
                      }
                      const auto align = t->alignment();
                      auto size = t->size_in_bytes();
                      std::int64_t stack_ptr = 0;
                      auto it = allocs.begin();
                      for (; it != allocs.end(); ++it) {
                          if (it->start - stack_ptr >= size) {
                              break;
                          }
                          stack_ptr = (1 + (it->stop - 1) / align) * align;
                      }
                      allocs.insert(it, allocation{a.result(), stack_ptr, stack_ptr + size});
                      a.stack_ptr(stack_ptr);
                  },
                  [&allocs](lifetime_stop_inst &s) {
                      int num = 0;
                      auto &v = s.object();
                      for (auto it = allocs.begin(); it != allocs.end();) {
                          if (it->value == &v) {
                              it = allocs.erase(it);
                              ++num;
                          } else {
                              ++it;
                          }
                      }
                      if (num != 1) {
                          throw compilation_error(
                              s.loc(), status::internal_compiler_error,
                              "Incorrect lifetime_stop: value not found in list of allocations");
                      }
                  },
                  [](inst_node &) {}},
              i);
    });
}

} // namespace tinytc
