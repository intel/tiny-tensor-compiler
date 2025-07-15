// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "location.hpp"
#include "parser/lexer.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/types.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <utility>

namespace tinytc {

lexer::lexer(std::uint64_t input_size, char const *input, location const &initial_loc)
    : input_{input}, len_(input_size), YYCURSOR{input_}, YYLIMIT{input_ + len_}, loc_{initial_loc} {
}

parser::symbol_type lexer::operator()() {
    char const *YYMARKER;
lex:
    char const *b = YYCURSOR;
    step(loc_);
    auto const adv_loc = [&]() { columns(loc_, YYCURSOR - b); };
    // clang-format off
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:eof = 0;

        // regexp
        comment               = ";" [^\n]*;
        newline               = "\r"? "\n";
        whitespace            = [ \t\v\r]+;

        unnamed_identifier    = [0-9]+;
        named_identifier      = [a-zA-Z] [a-zA-Z0-9_]*;
        local_unnamed_identifier = "%" unnamed_identifier;
        local_named_identifier   = "%" named_identifier;
        global_identifier     = "@" (unnamed_identifier | named_identifier);
        def_identifier        = "$" (unnamed_identifier | named_identifier);
        string                = "\"" [^\"]* "\"";
        digit                 = [0-9];
        hexdigit              = [0-9a-fA-F];
        integer_constant      = [-+]? digit+;
        mantissa              = digit* "." digit+ | digit+ ".";
        mantissa_hex          = (hexdigit* "." hexdigit+ | hexdigit+ ".");
        exponent              = [eE] [-+]? digit+;
        exponent_hex          = [pP] [-+]? digit+;
        floating_constant     = [-+]? (mantissa exponent? | digit+ exponent | "inf" | "nan");
        floating_constant_hex = [-+]? "0x" (mantissa_hex exponent_hex? | hexdigit+ exponent_hex);


        // identifier
        local_unnamed_identifier    {
            adv_loc();
            std::int64_t id = lex_integer_constant(b + 1, YYCURSOR);
            return parser::make_LOCAL_IDENTIFIER(std::move(id), loc_);
        }
        local_named_identifier    {
            adv_loc();
            auto id = std::string(b + 1, YYCURSOR);
            return parser::make_LOCAL_IDENTIFIER(std::move(id), loc_);
        }
        global_identifier   {
            adv_loc();
            auto id = std::string(b + 1, YYCURSOR);
            return parser::make_GLOBAL_IDENTIFIER(std::move(id), loc_);
        }
        def_identifier   {
            adv_loc();
            auto id = std::string(b + 1, YYCURSOR);
            return parser::make_DEF_IDENTIFIER(std::move(id), loc_);
        }

        // symbols
        "="                 { adv_loc(); return parser::make_EQUALS(loc_); }
        "<="                { adv_loc(); return parser::make_LESS_EQUAL(loc_); }
        ","                 { adv_loc(); return parser::make_COMMA(loc_); }
        "x"                 { adv_loc(); return parser::make_TIMES(loc_); }
        ":"                 { adv_loc(); return parser::make_COLON(loc_); }
        "("                 { adv_loc(); return parser::make_LPAREN(loc_); }
        ")"                 { adv_loc(); return parser::make_RPAREN(loc_); }
        "{"                 { adv_loc(); return parser::make_LBRACE(loc_); }
        "}"                 { adv_loc(); return parser::make_RBRACE(loc_); }
        "<"                 { adv_loc(); return parser::make_LCHEV(loc_); }
        ">"                 { adv_loc(); return parser::make_RCHEV(loc_); }
        "["                 { adv_loc(); return parser::make_LSQBR(loc_); }
        "]"                 { adv_loc(); return parser::make_RSQBR(loc_); }
        "^"                 { adv_loc(); return parser::make_CIRCUMFLEX(loc_); }
        "-"                 { adv_loc(); return parser::make_MINUS(loc_); }
        "+"                 { adv_loc(); return parser::make_PLUS(loc_); }
        "*"                 { adv_loc(); return parser::make_STAR(loc_); }
        "/"                 { adv_loc(); return parser::make_SLASH(loc_); }
        "%"                 { adv_loc(); return parser::make_PERCENT(loc_); }

        // keywords
        "as"                { adv_loc(); return parser::make_AS(loc_); }
        "func"              { adv_loc(); return parser::make_FUNC(loc_); }
        "->"                { adv_loc(); return parser::make_ARROW(loc_); }
        "?"                 { adv_loc(); return parser::make_DYNAMIC(loc_); }
        ".n"                { adv_loc(); return parser::make_TRANSPOSE(transpose::N, loc_); }
        ".t"                { adv_loc(); return parser::make_TRANSPOSE(transpose::T, loc_); }
        ".atomic"           { adv_loc(); return parser::make_ATOMIC(loc_); }
        "init"              { adv_loc(); return parser::make_INIT(loc_); }
        "local"             { adv_loc(); return parser::make_LOCAL(loc_); }
        "global"            { adv_loc(); return parser::make_GLOBAL(loc_); }
        ".local"            { adv_loc(); return parser::make_LOCAL_ATTR(loc_); }
        ".global"           { adv_loc(); return parser::make_GLOBAL_ATTR(loc_); }
        ".x"                { adv_loc(); return parser::make_COMP3(comp3::x, loc_); }
        ".y"                { adv_loc(); return parser::make_COMP3(comp3::y, loc_); }
        ".z"                { adv_loc(); return parser::make_COMP3(comp3::z, loc_); }
        ".row"              { adv_loc(); return parser::make_REDUCE_MODE(reduce_mode::row, loc_); }
        ".column"           { adv_loc(); return parser::make_REDUCE_MODE(reduce_mode::column, loc_); }

        // Memory scope
        ".cross_device" { adv_loc(); return parser::make_MEMORY_SCOPE(memory_scope::cross_device, loc_); }
        ".device"       { adv_loc(); return parser::make_MEMORY_SCOPE(memory_scope::device, loc_); }
        ".work_group"   { adv_loc(); return parser::make_MEMORY_SCOPE(memory_scope::work_group, loc_); }
        ".subgroup"     { adv_loc(); return parser::make_MEMORY_SCOPE(memory_scope::subgroup, loc_); }

        // Memory semantics
        ".relaxed"                 { adv_loc(); return parser::make_MEMORY_SEMANTICS(memory_semantics::relaxed, loc_); }
        ".acquire"                 { adv_loc(); return parser::make_MEMORY_SEMANTICS(memory_semantics::acquire, loc_); }
        ".release"                 { adv_loc(); return parser::make_MEMORY_SEMANTICS(memory_semantics::release, loc_); }
        ".acquire_release"         { adv_loc(); return parser::make_MEMORY_SEMANTICS(memory_semantics::acquire_release, loc_); }
        ".sequentially_consistent" { adv_loc(); return parser::make_MEMORY_SEMANTICS(memory_semantics::sequentially_consistent, loc_); }

        // constants
        "true"              { adv_loc(); return parser::make_BOOLEAN_CONSTANT(true, loc_); }
        "false"             { adv_loc(); return parser::make_BOOLEAN_CONSTANT(false, loc_); }
        integer_constant    {
            adv_loc();
            auto i = lex_integer_constant(b, YYCURSOR);
            return parser::make_INTEGER_CONSTANT(i, loc_);
        }
        floating_constant | floating_constant_hex {
            adv_loc();
            auto f = lex_floating_constant(b, YYCURSOR);
            return parser::make_FLOATING_CONSTANT(f, loc_);
        }

        // attributes
        "attributes"        { return parser::make_ATTRIBUTES(loc_); }
        "alignment" | "shape_gcd" | "stride_gcd" | "unroll" | "work_group_size" {
            adv_loc(); return parser::make_ATTR_NAME(std::string(b, YYCURSOR), loc_);
        }

        // types
        "bool"              { return parser::make_BOOLEAN(loc_); }
        "i8"                { adv_loc(); return parser::make_I8_TYPE(loc_); }
        "i16"               { adv_loc(); return parser::make_I16_TYPE(loc_); }
        "i32"               { adv_loc(); return parser::make_I32_TYPE(loc_); }
        "i64"               { adv_loc(); return parser::make_I64_TYPE(loc_); }
        "index"             { adv_loc(); return parser::make_INDEX_TYPE(loc_); }
        "bf16"              { adv_loc(); return parser::make_BF16_TYPE(loc_); }
        "f16"               { adv_loc(); return parser::make_F16_TYPE(loc_); }
        "f32"               { adv_loc(); return parser::make_F32_TYPE(loc_); }
        "f64"               { adv_loc(); return parser::make_F64_TYPE(loc_); }
        "c32"               { adv_loc(); return parser::make_C32_TYPE(loc_); }
        "c64"               { adv_loc(); return parser::make_C64_TYPE(loc_); }
        "coopmatrix"        { adv_loc(); return parser::make_COOPMATRIX(loc_); }
        "memref"            { adv_loc(); return parser::make_MEMREF(loc_); }
        "group"             { adv_loc(); return parser::make_GROUP(loc_); }

        // layouts
        "offset"            { adv_loc(); return parser::make_OFFSET(loc_); }
        "strided"           { adv_loc(); return parser::make_STRIDED(loc_); }

        // matrix use
        "matrix_a"          { adv_loc(); return parser::make_MATRIX_USE(matrix_use::a, loc_); }
        "matrix_b"          { adv_loc(); return parser::make_MATRIX_USE(matrix_use::b, loc_); }
        "matrix_acc"        { adv_loc(); return parser::make_MATRIX_USE(matrix_use::acc, loc_); }

        // checked flag
        ".rows_checked"     { adv_loc(); return parser::make_CHECKED(checked_flag::rows, loc_); }
        ".cols_checked"     { adv_loc(); return parser::make_CHECKED(checked_flag::cols, loc_); }
        ".both_checked"     { adv_loc(); return parser::make_CHECKED(checked_flag::both, loc_); }

        // instructions
        "alloca"             { adv_loc(); return parser::make_ALLOCA(loc_); }
        "atomic_load"        { adv_loc(); return parser::make_ATOMIC_LOAD(loc_); }
        "atomic_store"       { adv_loc(); return parser::make_ATOMIC_STORE(loc_); }
        "atomic_add"         { adv_loc(); return parser::make_ATOMIC_ADD(loc_); }
        "atomic_max"         { adv_loc(); return parser::make_ATOMIC_MAX(loc_); }
        "atomic_min"         { adv_loc(); return parser::make_ATOMIC_MIN(loc_); }
        "axpby"              { adv_loc(); return parser::make_AXPBY(loc_); }
        "barrier"            { adv_loc(); return parser::make_BARRIER(loc_); }
        "cumsum"             { adv_loc(); return parser::make_CUMSUM(loc_); }
        "gemm"               { adv_loc(); return parser::make_GEMM(loc_); }
        "gemv"               { adv_loc(); return parser::make_GEMV(loc_); }
        "ger"                { adv_loc(); return parser::make_GER(loc_); }
        "hadamard"           { adv_loc(); return parser::make_HADAMARD(loc_); }
        "cast"               { adv_loc(); return parser::make_CAST(loc_); }
        "constant"           { adv_loc(); return parser::make_CONSTANT(loc_); }
        "cooperative_matrix_apply"        { adv_loc(); return parser::make_COOPERATIVE_MATRIX_APPLY(loc_); }
        "cooperative_matrix_atomic_add"   { adv_loc(); return parser::make_COOPERATIVE_MATRIX_ATOMIC_ADD(loc_); }
        "cooperative_matrix_atomic_load"  { adv_loc(); return parser::make_COOPERATIVE_MATRIX_ATOMIC_LOAD(loc_); }
        "cooperative_matrix_atomic_max"   { adv_loc(); return parser::make_COOPERATIVE_MATRIX_ATOMIC_MAX(loc_); }
        "cooperative_matrix_atomic_min"   { adv_loc(); return parser::make_COOPERATIVE_MATRIX_ATOMIC_MIN(loc_); }
        "cooperative_matrix_atomic_store" { adv_loc(); return parser::make_COOPERATIVE_MATRIX_ATOMIC_STORE(loc_); }
        "cooperative_matrix_extract"      { adv_loc(); return parser::make_COOPERATIVE_MATRIX_EXTRACT(loc_); }
        "cooperative_matrix_insert"       { adv_loc(); return parser::make_COOPERATIVE_MATRIX_INSERT(loc_); }
        "cooperative_matrix_load"         { adv_loc(); return parser::make_COOPERATIVE_MATRIX_LOAD(loc_); }
        "cooperative_matrix_mul_add"      { adv_loc(); return parser::make_COOPERATIVE_MATRIX_MUL_ADD(loc_); }
        "cooperative_matrix_prefetch"     { adv_loc(); return parser::make_COOPERATIVE_MATRIX_PREFETCH(loc_); }
        "cooperative_matrix_scale"        { adv_loc(); return parser::make_COOPERATIVE_MATRIX_SCALE(loc_); }
        "cooperative_matrix_store"        { adv_loc(); return parser::make_COOPERATIVE_MATRIX_STORE(loc_); }
        "expand"             { adv_loc(); return parser::make_EXPAND(loc_); }
        "fuse"               { adv_loc(); return parser::make_FUSE(loc_); }
        "load"               { adv_loc(); return parser::make_LOAD(loc_); }
        "for"                { adv_loc(); return parser::make_FOR(loc_); }
        "foreach"            { adv_loc(); return parser::make_FOREACH(loc_); }
        "foreach_tile"       { adv_loc(); return parser::make_FOREACH_TILE(loc_); }
        "if"                 { adv_loc(); return parser::make_IF(loc_); }
        "parallel"           { adv_loc(); return parser::make_PARALLEL(loc_); }
        "else"               { adv_loc(); return parser::make_ELSE(loc_); }
        "size"               { adv_loc(); return parser::make_SIZE(loc_); }
        "subgroup_broadcast" { adv_loc(); return parser::make_SUBGROUP_BROADCAST(loc_); }
        "subview"            { adv_loc(); return parser::make_SUBVIEW(loc_); }
        "store"              { adv_loc(); return parser::make_STORE(loc_); }
        "sum"                { adv_loc(); return parser::make_SUM(loc_); }
        "yield"              { adv_loc(); return parser::make_YIELD(loc_); }

        // binary op
        "add"          { adv_loc(); return parser::make_ADD(loc_); }
        "sub"          { adv_loc(); return parser::make_SUB(loc_); }
        "mul"          { adv_loc(); return parser::make_MUL(loc_); }
        "div"          { adv_loc(); return parser::make_DIV(loc_); }
        "rem"          { adv_loc(); return parser::make_REM(loc_); }
        "shl"          { adv_loc(); return parser::make_SHL(loc_); }
        "shr"          { adv_loc(); return parser::make_SHR(loc_); }
        "and"          { adv_loc(); return parser::make_AND(loc_); }
        "or"           { adv_loc(); return parser::make_OR (loc_); }
        "xor"          { adv_loc(); return parser::make_XOR(loc_); }
        "min"          { adv_loc(); return parser::make_MIN(loc_); }
        "max"          { adv_loc(); return parser::make_MAX(loc_); }

        // unary op
        "abs"          { adv_loc(); return parser::make_ABS(loc_); }
        "neg"          { adv_loc(); return parser::make_NEG(loc_); }
        "not"          { adv_loc(); return parser::make_NOT(loc_); }
        "conj"         { adv_loc(); return parser::make_CONJ(loc_); }
        "im"           { adv_loc(); return parser::make_IM(loc_); }
        "re"           { adv_loc(); return parser::make_RE(loc_); }

        // builtin
        "group_id"           { adv_loc(); return parser::make_GROUP_ID(loc_); }
        "num_groups"         { adv_loc(); return parser::make_NUM_GROUPS(loc_); }
        "num_subgroups"      { adv_loc(); return parser::make_NUM_SUBGROUPS(loc_); }
        "subgroup_size"      { adv_loc(); return parser::make_SUBGROUP_SIZE(loc_); }
        "subgroup_id"        { adv_loc(); return parser::make_SUBGROUP_ID(loc_); }
        "subgroup_linear_id" { adv_loc(); return parser::make_SUBGROUP_LINEAR_ID(loc_); }
        "subgroup_local_id"  { adv_loc(); return parser::make_SUBGROUP_LOCAL_ID(loc_); }

        // comparison condition
        "equal"              { adv_loc(); return parser::make_EQUAL(loc_); }
        "not_equal"          { adv_loc(); return parser::make_NOT_EQUAL(loc_); }
        "greater_than"       { adv_loc(); return parser::make_GREATER_THAN(loc_); }
        "greater_than_equal" { adv_loc(); return parser::make_GREATER_THAN_EQUAL(loc_); }
        "less_than"          { adv_loc(); return parser::make_LESS_THAN(loc_); }
        "less_than_equal"    { adv_loc(); return parser::make_LESS_THAN_EQUAL(loc_); }

        // math op
        "cos"          { adv_loc(); return parser::make_COS(loc_); }
        "sin"          { adv_loc(); return parser::make_SIN(loc_); }
        "exp"          { adv_loc(); return parser::make_EXP(loc_); }
        "exp2"         { adv_loc(); return parser::make_EXP2(loc_); }
        "native_cos"   { adv_loc(); return parser::make_NATIVE_COS(loc_); }
        "native_sin"   { adv_loc(); return parser::make_NATIVE_SIN(loc_); }
        "native_exp"   { adv_loc(); return parser::make_NATIVE_EXP(loc_); }
        "native_exp2"  { adv_loc(); return parser::make_NATIVE_EXP2(loc_); }

        // coopmatrix reduce
        "cooperative_matrix_reduce_add" { adv_loc(); return parser::make_COOPERATIVE_MATRIX_REDUCE_ADD(loc_); }
        "cooperative_matrix_reduce_max" { adv_loc(); return parser::make_COOPERATIVE_MATRIX_REDUCE_MAX(loc_); }
        "cooperative_matrix_reduce_min" { adv_loc(); return parser::make_COOPERATIVE_MATRIX_REDUCE_MIN(loc_); }

        // subgroup op
        "subgroup_exclusive_scan_add" { adv_loc(); return parser::make_SUBGROUP_EXCLUSIVE_SCAN_ADD(loc_); }
        "subgroup_exclusive_scan_max" { adv_loc(); return parser::make_SUBGROUP_EXCLUSIVE_SCAN_MAX(loc_); }
        "subgroup_exclusive_scan_min" { adv_loc(); return parser::make_SUBGROUP_EXCLUSIVE_SCAN_MIN(loc_); }
        "subgroup_inclusive_scan_add" { adv_loc(); return parser::make_SUBGROUP_INCLUSIVE_SCAN_ADD(loc_); }
        "subgroup_inclusive_scan_max" { adv_loc(); return parser::make_SUBGROUP_INCLUSIVE_SCAN_MAX(loc_); }
        "subgroup_inclusive_scan_min" { adv_loc(); return parser::make_SUBGROUP_INCLUSIVE_SCAN_MIN(loc_); }
        "subgroup_reduce_add"         { adv_loc(); return parser::make_SUBGROUP_REDUCE_ADD(loc_); }
        "subgroup_reduce_max"         { adv_loc(); return parser::make_SUBGROUP_REDUCE_MAX(loc_); }
        "subgroup_reduce_min"         { adv_loc(); return parser::make_SUBGROUP_REDUCE_MIN(loc_); }

        // other strings
        string              { adv_loc(); return parser::make_STRING(std::string(b+1, YYCURSOR-1), loc_); }

        // macros
        "!calc"             { adv_loc(); return parser::make_M_CALC(loc_); }


        whitespace          { adv_loc(); goto lex; }
        comment             { adv_loc(); goto lex; }
        newline             { lines(loc_, 1); goto lex; }
        [\x00]              { adv_loc(); return parser::make_YYEOF(loc_); }
        $                   { adv_loc(); return parser::make_YYEOF(loc_); }
        *                   {
            adv_loc();
            throw parser::syntax_error(loc_, "Unknown token");
        }
     */
    // clang-format on
}

std::uint64_t lexer::lex_number(char const *s, char const *e) {
    std::uint64_t n = 0;
    for (; s < e; ++s) {
        auto d = *s - '0';
        /*
         * imax >= 10 * i + d
         * floor((imax - d) / 10) >= i
         */
        if ((std::numeric_limits<std::uint64_t>::max() - d) / 10 < n) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        n = 10 * n + d;
    }
    return n;
}

std::int64_t lexer::lex_integer_constant(char const *s, char const *e) {
    std::int64_t i = 0;
    if (*s == '-') {
        ++s;
        std::uint64_t n = lex_number(s, e);
        if (n > static_cast<std::uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        i = -static_cast<std::int64_t>(n);
    } else {
        if (*s == '+') {
            ++s;
        }
        std::uint64_t n = lex_number(s, e);
        if (n > static_cast<std::uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        i = static_cast<std::int64_t>(n);
    }
    return i;
}

double lexer::lex_floating_constant(char const *s, char const *e) {
    auto number = std::string(s, e);
    double d = strtod(number.c_str(), nullptr);
    if (errno == ERANGE) {
        throw parser::syntax_error(loc_, "Floating point value out of range: " + std::string(s, e));
    }
    return d;
}

} // namespace tinytc
