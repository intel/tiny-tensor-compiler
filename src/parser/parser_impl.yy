// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "ir/node/function_node.hpp"
    #include "location.hpp"
    #include "tinytc/ir/data_type.hpp"
    #include "tinytc/ir/inst.hpp"
    #include "tinytc/ir/func.hpp"
    #include "tinytc/ir/prog.hpp"
    #include "tinytc/ir/region.hpp"
    #include "tinytc/ir/scalar_type.hpp"
    #include "tinytc/ir/slice.hpp"
    #include "tinytc/ir/value.hpp"
    #include <algorithm>
    #include <cstdint>
    #include <functional>
    #include <new>
    #include <string>
    #include <utility>

    namespace tinytc {
        class parse_context;
        class lexer;
    }
}

%code {
    #include "ir/visitor/util.hpp"
    #include "ir/node/data_type_node.hpp"
    #include "ir/node/inst_node.hpp"
    #include "ir/node/program_node.hpp"
    #include "ir/node/region_node.hpp"
    #include "ir/node/value_node.hpp"
    #include "parser/parse_context.hpp"
    #include "parser/lexer.hpp"
    #include "error.hpp"
    #include "tinytc/ir/passes.hpp"
    
    #include <clir/visit.hpp>
    #include <clir/handle.hpp>

    #include <array>
    #include <cstdint>
    #include <memory>
    #include <vector>
    #include <utility>

    namespace tinytc {
    void check_scalar_type(value & val, scalar_type const& sty, location & loc1,
                           location & loc2) {
        clir::visit(
            overloaded{[&](int_imm &i) { i.ty(sty); },
                           [&](float_imm &i) { i.ty(sty); },
                           [&](auto &) {
                               if (!val->ty() || !is_equal(val->ty(), data_type(sty))) {
                                   auto loc = loc1;
                                   loc.end = loc2.end;
                                   throw parser::syntax_error(
                                       loc, "Type of SSA value does not match operand type");
                               }
                           }},
            *val);
    }
    void check_type(value & val, data_type & ty, location & loc1,
                    location & loc2) {
        if (!val->ty() || !is_equal(val->ty(), ty)) {
            auto loc = loc1;
            loc.end = loc2.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
    };
    }
}

%header
%parse-param { lexer& lex }
%parse-param { parse_context& ctx }
%lex-param { lexer& lex }
%locations
%define api.location.type { location }

%define parse.assert
%define parse.error detailed
%define parse.lac full
%define api.namespace {tinytc}
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {TOK_}
%token
    EQUALS          "="
    COMMA           ","
    TIMES           "x"
    COLON           ":"
    LPAREN          "("
    RPAREN          ")"
    LBRACE          "{"
    RBRACE          "}"
    LCHEV           "<"
    RCHEV           ">"
    LSQBR           "["
    RSQBR           "]"
    FUNC            "func"
    WORK_GROUP_SIZE "work_group_size"
    SUBGROUP_SIZE   "subgroup_size"
    RETURNS         "->"
    DYNAMIC         "?"
    NOTRANS         ".n"
    TRANS           ".t"
    ATOMIC          ".atomic"
    MEMREF          "memref"
    GROUP           "group"
    STRIDED         "strided"
    AXPBY           "axpby"
    GEMM            "gemm"
    GEMV            "gemv"
    GER             "ger"
    HADAMARD        "hadamard"
    ALLOCA          "alloca"
    CAST            "cast"
    CMP             "cmp"
    EXPAND          "expand"
    FUSE            "fuse"
    LOAD            "load"
    FOR             "for"
    FOREACH         "foreach"
    IF              "if"
    ELSE            "else"
    GROUP_ID        "group_id"
    GROUP_SIZE      "group_size"
    NEG             "neg"
    SIZE            "size"
    SUBVIEW         "subview"
    STORE           "store"
    SUM             "sum"
    YIELD           "yield"
;
%token <std::string> LOCAL_IDENTIFIER
%token <std::string> GLOBAL_IDENTIFIER
%token <std::int64_t> INTEGER_CONSTANT
%token <double> FLOATING_CONSTANT
%token <scalar_type> INTEGER_TYPE
%token <scalar_type> FLOATING_TYPE
%token <binary_op> BINARY_OP
%token <cmp_condition> CMP_CONDITION

%nterm <prog> prog
%nterm <std::vector<func>> func_list
%nterm <func> func
%nterm <std::vector<::tinytc::value>> arguments
%nterm <::tinytc::value> argument
%nterm <std::vector<std::function<void(function&)>>> attributes
%nterm <std::function<void(function&)>> attribute
%nterm <data_type> data_type
%nterm <scalar_type> scalar_type
%nterm <data_type> memref_type
%nterm <std::vector<std::int64_t>> mode_list
%nterm <std::vector<std::int64_t>> optional_stride_list
%nterm <std::vector<std::int64_t>> stride_list
%nterm <std::int64_t> constant_or_dynamic
%nterm <data_type> group_type
%nterm <data_type> memref_or_group_type
%nterm <region> region
%nterm <::tinytc::value> var
%nterm <std::vector<inst>> instructions
%nterm <inst> instruction
%nterm <inst> axpby_inst
%nterm <bool> atomic
%nterm <::tinytc::value> identifier_or_constant
%nterm <std::vector<::tinytc::value>> optional_identifier_or_constant_list
%nterm <std::vector<::tinytc::value>> identifier_or_constant_list
%nterm <inst> gemm_inst
%nterm <inst> gemv_inst
%nterm <inst> ger_inst
%nterm <transpose> transpose
%nterm <inst> for_inst
%nterm <::tinytc::value> optional_step
%nterm <inst> foreach_inst
%nterm <inst> hadamard_inst
%nterm <inst> if_inst
%nterm <std::vector<scalar_type>> optional_returned_values
%nterm <std::vector<scalar_type>> optional_scalar_type_list
%nterm <std::vector<scalar_type>> scalar_type_list
%nterm <region> else_region
%nterm <inst> sum_inst
%nterm <inst> yield_inst
%nterm <scalar_type> for_loop_var_type
%nterm <inst> var_definition
%nterm <std::vector<std::string>> identifier_list
%nterm <inst> valued_inst
%nterm <inst> alloca_inst
%nterm <inst> binary_op_inst
%nterm <inst> cast_inst
%nterm <inst> compare_inst
%nterm <inst> expand_inst
%nterm <::tinytc::value> constant_or_dynamic_or_identifier
%nterm <std::vector<::tinytc::value>> expand_shape
%nterm <inst> fuse_inst
%nterm <inst> load_inst
%nterm <std::vector<::tinytc::value>> optional_index_list
%nterm <std::vector<::tinytc::value>> index_list
%nterm <::tinytc::value> index_identifier_or_const
%nterm <inst> group_id_inst
%nterm <inst> group_size_inst
%nterm <inst> neg_inst
%nterm <inst> size_inst
%nterm <inst> store_inst
%nterm <inst> subview_inst
%nterm <std::vector<slice>> optional_slice_list
%nterm <std::vector<slice>> slice_list
%nterm <slice> slice
%nterm <::tinytc::value> slice_size

%%
prog:
    func_list { 
        auto p = prog{std::make_shared<program>(std::move($func_list))};
        ctx.program(p);
        $$ = std::move(p);
    }
;

func_list:
    func { $$.emplace_back(std::move($func)); }
  | func_list func { $$ = std::move($1); $$.emplace_back(std::move($func)); }

func:
    FUNC {
        ctx.push_scope();
    } GLOBAL_IDENTIFIER LPAREN arguments RPAREN attributes region {
        auto proto = func{
            std::make_shared<prototype>($GLOBAL_IDENTIFIER, std::move($arguments))};
        auto loc = @FUNC;
        loc.end = @RPAREN.end;
        ctx.prototype($GLOBAL_IDENTIFIER, proto, loc);
        auto func_node =
            std::make_shared<function>(std::move(proto), std::move($region));
        for (auto &attr : $attributes) {
            attr(*func_node);
        }
        $func = func{std::move(func_node)};
        $func->loc(@func);
        ctx.pop_scope();
    }
;

arguments:
    %empty {}
  | argument { $$.emplace_back(std::move($argument)); }
  | arguments COMMA argument { $$ = std::move($1); $$.emplace_back(std::move($argument)); }
;

argument:
    LOCAL_IDENTIFIER COLON data_type {
        auto v = value(std::move($data_type), $LOCAL_IDENTIFIER);
        ctx.val($LOCAL_IDENTIFIER, v, @LOCAL_IDENTIFIER);
        $$ = std::move(v);
    }
;

attributes:
    %empty {}
  | attributes attribute { $$ = std::move($1); $$.emplace_back(std::move($attribute)); }
;

attribute:
    WORK_GROUP_SIZE LPAREN INTEGER_CONSTANT[m] COMMA INTEGER_CONSTANT[n] RPAREN {
        if ($m <= 0) {
            throw parser::syntax_error(@m, "Must be a non-negative number");
        }
        if ($n <= 0) {
            throw parser::syntax_error(@n, "Must be a non-negative number");
        }
        auto const wgs = std::array<std::uint32_t, 2>{static_cast<std::uint32_t>($m),
                                                      static_cast<std::uint32_t>($n)};
        $$ = [=](function &f) { f.work_group_size(wgs); };
    }
  | SUBGROUP_SIZE LPAREN INTEGER_CONSTANT RPAREN {
        if ($INTEGER_CONSTANT <= 0) {
            throw parser::syntax_error(@INTEGER_CONSTANT, "Must be a non-negative number");
        }
        auto const sgs = static_cast<std::uint32_t>($INTEGER_CONSTANT);
        $$ = [=](function &f) { f.subgroup_size(sgs); };
    }
;


data_type:
    scalar_type { $$ = data_type($scalar_type); $$->loc(@scalar_type); }
  | memref_type
  | group_type
;

scalar_type:
    INTEGER_TYPE
  | FLOATING_TYPE
;

memref_type:
    MEMREF LCHEV scalar_type mode_list RCHEV {
        try {
            $$ = memref_type($scalar_type, std::move($mode_list), std::vector<std::int64_t>{}, @memref_type);
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
  | MEMREF LCHEV scalar_type mode_list COMMA STRIDED LCHEV optional_stride_list RCHEV RCHEV {
        if ($mode_list.size() != $optional_stride_list.size()) {
            auto loc = @scalar_type;
            loc.end = @optional_stride_list.end;
            throw syntax_error(loc, "Shape and stride list must have the same length");
        }
        try {
            $$ = memref_type($scalar_type, std::move($mode_list),
                                 std::move($optional_stride_list), @memref_type);
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

mode_list:
    %empty {}
  | mode_list TIMES constant_or_dynamic { $$ = std::move($1); $$.push_back($constant_or_dynamic); }
;

optional_stride_list:
    %empty {}
  | stride_list { $$ = std::move($1); }
;

stride_list:
    constant_or_dynamic { $$.push_back($constant_or_dynamic); }
  | stride_list COMMA constant_or_dynamic { $$ = std::move($1); $$.push_back($constant_or_dynamic); }
;

constant_or_dynamic:
    INTEGER_CONSTANT { $$ = $INTEGER_CONSTANT; }
  | DYNAMIC { $$ = dynamic; }
;

group_type:
    GROUP LCHEV memref_type RCHEV {
        $$ = group_type(std::move($memref_type));
        $$->loc(@group_type);
    }
;

memref_or_group_type:
    memref_type
  | group_type
;

region:
    LBRACE {
        ctx.push_scope();
    } instructions RBRACE {
        $$ = region{std::make_shared<rgn>(std::move($instructions))};
        ctx.pop_scope();
    }
;

var:
    LOCAL_IDENTIFIER { $$ = ctx.val($LOCAL_IDENTIFIER, @LOCAL_IDENTIFIER); }
;

instructions:
    %empty {}
  | instructions instruction {
        $$ = std::move($1);
        $$.emplace_back(std::move($instruction));
    }
;

instruction:
    axpby_inst
  | gemm_inst
  | gemv_inst
  | ger_inst
  | for_inst
  | foreach_inst
  | hadamard_inst
  | if_inst
  | var_definition
  | store_inst
  | sum_inst
  | yield_inst
;

axpby_inst:
    AXPBY transpose[ta] atomic
          identifier_or_constant[alpha] COMMA var[a] COMMA identifier_or_constant[beta] COMMA var[b]
          COLON scalar_type[falpha] COMMA memref_type[ma] COMMA scalar_type[fbeta] COMMA memref_type[mb] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($b, $mb, @b, @mb);
        try {
            $$ = inst {
                std::make_shared<axpby_inst>($ta, std::move($alpha), std::move($a),
                                                           std::move($beta), std::move($b), $atomic,
                                                           @axpby_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

atomic:
    %empty { $$ = false; }
  | ATOMIC { $$ = true; }
;

identifier_or_constant:
    var { $$ = $var; }
  | INTEGER_CONSTANT { $$ = value($INTEGER_CONSTANT); $$->loc(@INTEGER_CONSTANT); }
  | FLOATING_CONSTANT { $$ = value($FLOATING_CONSTANT); $$->loc(@FLOATING_CONSTANT); }
;

optional_identifier_or_constant_list:
    %empty {}
  | identifier_or_constant_list { $$ = std::move($1); }

identifier_or_constant_list:
    identifier_or_constant { $$.push_back(std::move($identifier_or_constant)); }
  | identifier_or_constant_list COMMA identifier_or_constant {
        $$ = std::move($1);
        $$.push_back(std::move($identifier_or_constant));
    }
;

gemm_inst:
    GEMM transpose[ta] transpose[tb] atomic
         identifier_or_constant[alpha] COMMA var[a] COMMA var[b] COMMA identifier_or_constant[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst{
                std::make_shared<gemm_inst>(
                    $ta, $tb, std::move($alpha), std::move($a), std::move($b), std::move($beta),
                    std::move($c), $atomic, @gemm_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

gemv_inst:
    GEMV transpose[ta] atomic
         identifier_or_constant[alpha] COMMA var[a] COMMA var[b] COMMA identifier_or_constant[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_shared<gemv_inst>($ta, std::move($alpha), std::move($a),
                                                          std::move($b), std::move($beta),
                                                          std::move($c), $atomic, @gemv_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

transpose:
    NOTRANS { $$ = transpose::N; }
  | TRANS { $$ = transpose::T; }
;

ger_inst:
    GER atomic
         identifier_or_constant[alpha] COMMA var[a] COMMA var[b] COMMA identifier_or_constant[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_shared<ger_inst>(std::move($alpha), std::move($a),
                                                         std::move($b), std::move($beta),
                                                         std::move($c), $atomic, @ger_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

for_inst:
    FOR LOCAL_IDENTIFIER[loop_var]
        EQUALS identifier_or_constant[from] COMMA identifier_or_constant[to] optional_step
        for_loop_var_type {
        check_scalar_type($from, $for_loop_var_type, @from, @for_loop_var_type);
        check_scalar_type($to, $for_loop_var_type, @to, @for_loop_var_type);
        if ($optional_step) {
            check_scalar_type($optional_step, $for_loop_var_type, @optional_step, @for_loop_var_type);
        }
        auto v = value(data_type($for_loop_var_type), $loop_var);
        ctx.val($loop_var, std::move(v), @loop_var);
    } region {
        try {
            $$ = inst {
                std::make_shared<for_inst>(ctx.val($loop_var, @loop_var), $from, $to,
                                                         $optional_step, std::move($region),
                                                         @for_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

optional_step:
    %empty { $$ = nullptr; }
  | COMMA identifier_or_constant { $$ = $identifier_or_constant; }

foreach_inst:
    FOREACH LOCAL_IDENTIFIER[loop_var]
        EQUALS identifier_or_constant[from] COMMA identifier_or_constant[to] for_loop_var_type {
        check_scalar_type($from, $for_loop_var_type, @from, @for_loop_var_type);
        check_scalar_type($to, $for_loop_var_type, @to, @for_loop_var_type);
        auto v = value(data_type($for_loop_var_type), $loop_var);
        ctx.val($loop_var, std::move(v), @loop_var);
    } region {
        try {
            $$ = inst {
                std::make_shared<foreach_inst>(ctx.val($loop_var, @loop_var), $from,
                                                             $to, std::move($region), @foreach_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

for_loop_var_type:
    %empty { $$ = scalar_type::index; }
  | COLON INTEGER_TYPE { $$ = $INTEGER_TYPE; }
;

var_definition:
    identifier_list EQUALS valued_inst {
        $$ = std::move($valued_inst);
        if ($identifier_list.size() == 1) {
            if (!$$->result()) {
                throw syntax_error(@identifier_list, "Instruction does not return value");
            }
            $$->result()->name($identifier_list[0]);
            ctx.val($identifier_list[0], $$->result(), @identifier_list);
        } else {
            auto results = $$->results();
            if (results.size() != $identifier_list.size()) {
                throw syntax_error(
                    @identifier_list,
                    "Number of identifiers does not equal number of returned values");
            }
            for (std::size_t i = 0; i < results.size(); ++i) {
                results[i]->name($identifier_list[i]);
                ctx.val($identifier_list[i], results[i], @identifier_list);
            }
        }
    }
;

identifier_list:
    LOCAL_IDENTIFIER { $$.push_back($LOCAL_IDENTIFIER); }
  | identifier_list COMMA LOCAL_IDENTIFIER { $$ = std::move($1); $$.push_back($LOCAL_IDENTIFIER); }
;


hadamard_inst:
    HADAMARD atomic
         identifier_or_constant[alpha] COMMA var[a] COMMA var[b] COMMA identifier_or_constant[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_shared<hadamard_inst>(
                    std::move($alpha), std::move($a), std::move($b), std::move($beta),
                    std::move($c), $atomic, @hadamard_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

sum_inst:
    SUM transpose[ta] atomic
          identifier_or_constant[alpha] COMMA var[a] COMMA identifier_or_constant[beta] COMMA var[b]
          COLON scalar_type[falpha] COMMA memref_type[ma] COMMA scalar_type[fbeta] COMMA memref_type[mb] {
        check_scalar_type($alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_scalar_type($beta, $fbeta, @beta, @fbeta);
        check_type($b, $mb, @b, @mb);
        try {
            $$ = inst {
                std::make_shared<sum_inst>($ta, std::move($alpha), std::move($a),
                                                         std::move($beta), std::move($b), $atomic,
                                                         @sum_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

yield_inst:
    YIELD optional_identifier_or_constant_list[vals] COLON optional_scalar_type_list[tys] {
        if ($vals.size() != $tys.size()) {
            location loc = @vals;
            loc.end = @tys.end;
            throw syntax_error(loc, "Identifier and scalar type list must have the same length");
        }
        for (std::size_t i = 0; i < $vals.size(); ++i) {
            check_scalar_type($vals[i], $tys[i], @vals, @tys);
        }
        $$ = inst{std::make_shared<yield_inst>(std::move($vals))};
    }
;

valued_inst:
    alloca_inst
  | binary_op_inst
  | cast_inst
  | compare_inst
  | expand_inst
  | fuse_inst
  | if_inst
  | load_inst
  | group_id_inst
  | group_size_inst
  | neg_inst
  | size_inst
  | subview_inst
;

alloca_inst:
    ALLOCA RETURNS memref_type {
        try {
            $$ = inst {
                std::make_shared<alloca_inst>(std::move($memref_type), @alloca_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

binary_op_inst:
    BINARY_OP identifier_or_constant[a] COMMA identifier_or_constant[b] COLON scalar_type[ty] {
        check_scalar_type($a, $ty, @a, @ty);
        check_scalar_type($b, $ty, @b, @ty);
        try {
            $$ = inst {
                std::make_shared<binary_op_inst>($BINARY_OP, std::move($a),
                                                               std::move($b), @binary_op_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

cast_inst:
    CAST identifier_or_constant[a] COLON scalar_type[from] RETURNS scalar_type[to] {
        check_scalar_type($a, $from, @a, @from);
        try {
            $$ = inst {
                std::make_shared<cast_inst>(std::move($a), $to, @cast_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

compare_inst:
    CMP CMP_CONDITION identifier_or_constant[a] COMMA identifier_or_constant[b] COLON scalar_type[ty] {
        check_scalar_type($a, $ty, @a, @ty);
        check_scalar_type($b, $ty, @b, @ty);
        try {
            $$ = inst {
                std::make_shared<compare_inst>($CMP_CONDITION, std::move($a),
                                                             std::move($b), @compare_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

expand_inst:
    EXPAND var LSQBR INTEGER_CONSTANT[mode] RETURNS expand_shape RSQBR COLON memref_type {
        if (!$var->ty() || !is_equal($var->ty(), $memref_type)) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<expand_inst>(std::move($var), $mode,
                                                            std::move($expand_shape), @expand_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

expand_shape:
    constant_or_dynamic_or_identifier[a] TIMES constant_or_dynamic_or_identifier[b] {
        $$ = std::vector<value>{$a, $b};
    }
  | expand_shape TIMES constant_or_dynamic_or_identifier[a] { $$ = std::move($1); $$.push_back($a); }
;

constant_or_dynamic_or_identifier:
    var {
        check_scalar_type($var, scalar_type::index, @var, @var);
        $$ = $var;
    }
  | INTEGER_CONSTANT { $$ = value($INTEGER_CONSTANT, scalar_type::index); $$->loc(@INTEGER_CONSTANT); }
  | DYNAMIC { $$ = value(dynamic); $$->loc(@DYNAMIC); }
;

fuse_inst:
    FUSE var LSQBR INTEGER_CONSTANT[from] COMMA INTEGER_CONSTANT[to] RSQBR COLON memref_type {
        if (!$var->ty() || !is_equal($var->ty(), $memref_type)) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<fuse_inst>(std::move($var), $from, $to, @fuse_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

load_inst:
    LOAD var LSQBR optional_index_list RSQBR COLON memref_or_group_type {
        if (!$var->ty() || !is_equal($var->ty(), $memref_or_group_type)) {
            auto loc = @var;
            loc.end = @memref_or_group_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<load_inst>(
                    std::move($var), std::move($optional_index_list), @load_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

optional_index_list:
    %empty {}
  | index_list { $$ = std::move($1); }
;

index_list:
    index_identifier_or_const { $$.push_back($index_identifier_or_const); }
  | index_list COMMA index_identifier_or_const { $$ = std::move($1); $$.push_back($index_identifier_or_const); }
;

index_identifier_or_const:
    var {
        check_scalar_type($var, scalar_type::index, @var, @var);
        $$ = $var;
    }
  | INTEGER_CONSTANT {
        $$ = value($INTEGER_CONSTANT);
        $$->ty(scalar_type::index);
        $$->loc(@INTEGER_CONSTANT);
    }
;

store_inst:
    STORE var[a] COMMA var[b] LSQBR optional_index_list RSQBR COLON memref_type {
        if (!$b->ty() || !is_equal($b->ty(), $memref_type)) {
            auto loc = @b;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<store_inst>(
                    std::move($a), std::move($b), std::move($optional_index_list), @store_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

group_id_inst:
    GROUP_ID { $$ = inst{std::make_shared<group_id_inst>()}; }
;

group_size_inst:
    GROUP_SIZE { $$ = inst{std::make_shared<group_size_inst>()}; }
;

if_inst:
    IF identifier_or_constant[condition] optional_returned_values region else_region {
        check_scalar_type($condition, scalar_type::bool_, @condition, @condition);
        $$ = inst{std::make_shared<if_inst>(
            std::move($condition), std::move($region), std::move($else_region),
            std::move($optional_returned_values))};
        $$->loc(@if_inst);
    }
;

else_region:
    %empty { $$ = nullptr; }
  | ELSE region{ $$ = std::move($region); }
;

optional_returned_values:
    %empty { $$ = {}; }
  | RETURNS LPAREN optional_scalar_type_list[tys] RPAREN { $$ = std::move($tys); }
;

optional_scalar_type_list:
    %empty {}
  | scalar_type_list { $$ = std::move($1); }
;

scalar_type_list:
    scalar_type { $$.push_back($scalar_type); }
  | scalar_type_list COMMA scalar_type { $$ = std::move($1); $$.push_back($scalar_type); }
;


neg_inst:
    NEG identifier_or_constant[a] COLON scalar_type[ty] {
        check_scalar_type($a, $ty, @a, @ty);
        try {
            $$ = inst { std::make_shared<neg_inst>(std::move($a), @neg_inst) };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

size_inst:
    SIZE var LSQBR INTEGER_CONSTANT[mode] RSQBR COLON memref_type {
        if (!$var->ty() || !is_equal($var->ty(), $memref_type)) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<size_inst>(std::move($var), $mode, @size_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

subview_inst:
    SUBVIEW var LSQBR optional_slice_list RSQBR COLON memref_type {
        if (!$var->ty() || !is_equal($var->ty(), $memref_type)) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_shared<subview_inst>(
                    std::move($var), std::move($optional_slice_list), @subview_inst)
            };
        } catch (compilation_error const &e) {
            throw syntax_error(e.loc(), e.what());
        }
    }
;

optional_slice_list:
    %empty {}
  | slice_list { $$ = std::move($1); }
;

slice_list:
    slice { $$.push_back($slice); }
  | slice_list COMMA slice { $$ = std::move($1); $$.push_back($slice); }
;

slice:
    COLON { $$ = slice(static_cast<std::int64_t>(0), dynamic); }
  | index_identifier_or_const slice_size { $$ = slice(std::move($1), std::move($2)); }
;

slice_size:
    %empty { $$ = nullptr; }
  | COLON index_identifier_or_const { $$ = $2; }
  | COLON DYNAMIC { $$ = dynamic; }
;

%%

namespace tinytc {
void parser::error(location_type const& l, std::string const& m) {
    lex.error(l, m);
}
}
