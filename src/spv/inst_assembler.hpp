// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INST_ASSEMBLER_20241111_HPP
#define INST_ASSEMBLER_20241111_HPP

#include "spv/defs.hpp"
#include "spv/visit.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace tinytc::spv {

template <typename WordT> class word_stream {
  public:
    word_stream(std::vector<std::uint8_t> &vec) : vec_{&vec} {}

    template <typename T> auto operator<<(T const &t) -> word_stream & {
        const std::size_t insert_pos = vec_->size() / sizeof(WordT);
        vec_->resize(vec_->size() + word_count(t) * sizeof(WordT));
        update(insert_pos, t);
        return *this;
    }

    template <typename T> static auto word_count(T const &) -> std::size_t {
        return 1 + (sizeof(T) - 1) / sizeof(WordT); // ceil(sizeof(T)/sizeof(WordT))
    }

    static auto word_count(std::string const &s) -> std::size_t {
        return 1 + s.size() / sizeof(WordT); // ceil((s.size()+1)/sizeof(WordT))
    }

    template <typename T> auto update(std::size_t word, T const &t) -> word_stream & {
        const std::size_t addr = word * sizeof(WordT);
        std::memcpy(vec_->data() + addr, &t, sizeof(T));
        return *this;
    }

    auto update(std::size_t word, std::string const &s) -> word_stream & {
        const std::size_t addr = word * sizeof(WordT);
        std::memcpy(vec_->data() + addr, s.c_str(), s.size() + 1);
        return *this;
    }

    //! Returns last word position
    auto tell() const -> std::size_t {
        const auto size = vec_->size();
        return size > 0 ? size / sizeof(WordT) - 1 : 0;
    }

  private:
    std::vector<std::uint8_t> *vec_;
};

class inst_assembler : public default_visitor<inst_assembler> {
  public:
    using default_visitor<inst_assembler>::operator();

    inst_assembler(word_stream<std::int32_t> &stream);

    void pre_visit(spv_inst const &in);
    void visit_result(spv_inst const &in);
    void post_visit(spv_inst const &in);

    template <typename T>
    requires std::is_enum_v<T>
    void operator()(T const &t) {
        *stream_ << static_cast<std::int32_t>(t);
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

  private:
    word_stream<std::int32_t> *stream_;
    std::size_t last_opcode_pos_ = 0;
};

} // namespace tinytc::spv

#endif // INST_ASSEMBLER_20241111_HPP
