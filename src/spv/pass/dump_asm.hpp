// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_ASM_20241029_HPP
#define DUMP_ASM_20241029_HPP

#include "spv/defs.hpp"
#include "spv/instructions.hpp"
#include "spv/names.hpp"
#include "spv/visit.hpp"
#include "tinytc/types.h"

#include <ostream>

namespace tinytc::spv {

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

    void operator()(PairIdRefIdRef const &p);
    void operator()(PairIdRefLiteralInteger const &p);
    void operator()(PairLiteralIntegerIdRef const &p);

    void operator()(spv_inst *const &in);
    void operator()(OpExtInst const &in);

    void run_on_module(tinytc_spv_mod const &m);

  private:
    std::ostream *os_;
};

} // namespace tinytc::spv

#endif // DUMP_ASM_20241029_HPP
