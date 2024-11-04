// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_ASM_20241029_HPP
#define DUMP_ASM_20241029_HPP

#include "spv/instructions.hpp"
#include "spv/names.hpp"
#include "spv/visit.hpp"

#include <ostream>
#include <unordered_map>
#include <utility>

namespace tinytc::spv {

class mod;

class dump_asm_pass : public default_visitor<dump_asm_pass> {
  public:
    using default_visitor<dump_asm_pass>::operator();
    constexpr static int rhs_indent = 15;

    dump_asm_pass(std::ostream &os);

    void pre_visit(spv_inst const &in);

    template <typename T>
    requires requires(T const &e) { to_string(e); }
    void operator()(T const &e) {
        *os_ << " " << to_string(e);
    }
    void operator()(DecorationAttr const &da);
    void operator()(ExecutionModeAttr const &ea);
    void operator()(LiteralContextDependentNumber const &l);
    void operator()(LiteralInteger const &l);
    void operator()(LiteralString const &l);

    void operator()(spv_inst *const &in);
    void operator()(PairIdRefIdRef const &p);
    void operator()(PairIdRefLiteralInteger const &p);
    void operator()(PairLiteralIntegerIdRef const &p);

    void run_on_module(mod const &m);

  private:
    auto declare(spv_inst const *in) -> std::int64_t;

    std::ostream *os_;

    std::int64_t slot_ = 0;
    std::unordered_map<spv_inst const *, std::int64_t> slot_map_;
};

} // namespace tinytc::spv

#endif // DUMP_ASM_20241029_HPP
