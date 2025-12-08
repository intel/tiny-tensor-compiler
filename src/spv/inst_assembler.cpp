// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/inst_assembler.hpp"
#include "util/overloaded.hpp"

#include <array>
#include <utility>
#include <variant>

namespace tinytc::spv {

enum class LinkageType;

inst_assembler::inst_assembler(word_stream<std::int32_t> &stream) : stream_{&stream} {}

void inst_assembler::operator()(DecorationAttr const &da) {
    std::visit(overloaded{[&](auto const &a) { this->operator()(a); },
                          [&](std::pair<std::string, LinkageType> const &a) {
                              *stream_ << a.first;
                              this->operator()(a.second);
                          }},
               da);
}
void inst_assembler::operator()(ExecutionModeAttr const &ea) {
    std::visit(overloaded{[&](std::int32_t const &a) { *stream_ << a; },
                          [&](std::array<std::int32_t, 3u> const &a) {
                              for (auto const &s : a) {
                                  *stream_ << s;
                              }
                          }},
               ea);
}
void inst_assembler::operator()(LiteralContextDependentNumber const &l) {
    std::visit(overloaded{[&](auto const &l) { *stream_ << l; }}, l);
}
void inst_assembler::operator()(LiteralInteger const &l) { *stream_ << l; }
void inst_assembler::operator()(LiteralString const &l) { *stream_ << l; }

void inst_assembler::operator()(PairIdRefIdRef const &p) {
    this->operator()(p.first);
    this->operator()(p.second);
}
void inst_assembler::operator()(PairIdRefLiteralInteger const &p) {
    this->operator()(p.first);
    this->operator()(p.second);
}
void inst_assembler::operator()(PairLiteralIntegerIdRef const &p) {
    std::visit(overloaded{[&](auto const &l) { *stream_ << l; }}, p.first);
    this->operator()(p.second);
}

void inst_assembler::pre_visit(spv_inst const &) {
    *stream_ << std::int32_t{0};
    last_opcode_pos_ = stream_->tell();
}

void inst_assembler::visit_result(spv_inst const &in) { *stream_ << in.id(); }

void inst_assembler::post_visit(spv_inst const &in) {
    const std::int32_t word_count = stream_->tell() - last_opcode_pos_ + 1;
    const auto ophead = (word_count << 16) | static_cast<std::int32_t>(in.opcode());
    stream_->update(last_opcode_pos_, ophead);
}

void inst_assembler::operator()(spv_inst *const &in) { *stream_ << in->id(); }

} // namespace tinytc::spv

