// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "node/function_node.hpp"
    #include "tinytc/tinytc.hpp"
    #include "tinytc/types.hpp"
    #include <algorithm>
    #include <cstdint>
    #include <functional>
    #include <new>
    #include <utility>

    namespace tinytc {
        class parse_context;
        class lexer;

        using int_or_val = std::variant<std::int64_t, ::tinytc::value>;
    }
}

%code {
    #include "error.hpp"
    #include "node/data_type_node.hpp"
    #include "node/inst_node.hpp"
    #include "node/program_node.hpp"
    #include "node/region_node.hpp"
    #include "node/value_node.hpp"
    #include "parser/lexer.hpp"
    #include "parser/parse_context.hpp"
    #include "support/util.hpp"
    #include "support/visit.hpp"

    #include <array>
    #include <cstdint>
    #include <cstdlib>
    #include <memory>
    #include <utility>
    #include <variant>
    #include <vector>

    namespace tinytc {
    void check_scalar_type(compiler_context const &ctx, value &val, scalar_type const &sty,
                           location &loc1, location &loc2) {
         if (val->ty() != get_scalar(ctx, sty)) {
             auto loc = loc1;
             loc.end = loc2.end;
             throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
         }
    }
    void check_type(value &val, tinytc_data_type_t ty, location &loc1, location &loc2) {
        if (val->ty() != ty) {
            auto loc = loc1;
            loc.end = loc2.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
    };
    } // namespace tinytc
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
    LOCAL           "local"
    GLOBAL          "global"
    LOCAL_ATTR      ".local"
    GLOBAL_ATTR     ".global"
    MEMREF          "memref"
    GROUP           "group"
    OFFSET          "offset"
    STRIDED         "strided"
    AXPBY           "axpby"
    ARITH           "arith"
    BARRIER         "barrier"
    GEMM            "gemm"
    GEMV            "gemv"
    GER             "ger"
    HADAMARD        "hadamard"
    ALLOCA          "alloca"
    CAST            "cast"
    CMP             "cmp"
    CONSTANT        "constant"
    EXPAND          "expand"
    FUSE            "fuse"
    LOAD            "load"
    FOR             "for"
    FOREACH         "foreach"
    GROUP_ID        "group_id"
    GROUP_SIZE      "group_size"
    IF              "if"
    ELSE            "else"
    NUM_SUBGROUPS   "num_subgroups"
    PARALLEL        "parallel"
    SIZE            "size"
    SUBGROUP_ID     "subgroup_id"
    SUBGROUP_LOCAL_ID "subgroup_local_id"
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
%token <arithmetic> ARITHMETIC
%token <arithmetic_unary> ARITHMETIC_UNARY
%token <cmp_condition> CMP_CONDITION

%nterm <prog> prog
%nterm <std::vector<func>> func_list
%nterm <func> func
%nterm <std::vector<::tinytc::value>> arguments
%nterm <::tinytc::value> argument
%nterm <std::vector<std::function<void(function_node&)>>> attributes
%nterm <std::function<void(function_node&)>> attribute
%nterm <tinytc_data_type_t> data_type
%nterm <scalar_type> scalar_type
%nterm <tinytc_data_type_t> memref_type
%nterm <address_space> optional_address_space
%nterm <std::vector<std::int64_t>> mode_list
%nterm <std::vector<std::int64_t>> optional_stride_list
%nterm <std::vector<std::int64_t>> stride_list
%nterm <std::int64_t> constant_or_dynamic
%nterm <tinytc_data_type_t> group_type
%nterm <std::int64_t> group_offset
%nterm <tinytc_data_type_t> memref_or_group_type
%nterm <region> region
%nterm <::tinytc::value> var
%nterm <std::vector<inst>> instructions
%nterm <inst> instruction
%nterm <inst> axpby_inst
%nterm <bool> atomic
%nterm <std::vector<::tinytc::value>> optional_value_list
%nterm <std::vector<::tinytc::value>> value_list
%nterm <inst> barrier_inst
%nterm <std::int32_t> optional_global_attr
%nterm <std::int32_t> optional_local_attr
%nterm <inst> gemm_inst
%nterm <inst> gemv_inst
%nterm <inst> ger_inst
%nterm <transpose> transpose
%nterm <inst> for_inst
%nterm <::tinytc::value> optional_step
%nterm <inst> foreach_inst
%nterm <inst> hadamard_inst
%nterm <inst> if_inst
%nterm <std::vector<tinytc_data_type_t>> optional_returned_values
%nterm <std::vector<tinytc_data_type_t>> optional_scalar_type_list
%nterm <std::vector<tinytc_data_type_t>> scalar_type_list
%nterm <region> else_region
%nterm <inst> sum_inst
%nterm <inst> yield_inst
%nterm <scalar_type> for_loop_var_type
%nterm <inst> var_definition
%nterm <std::vector<std::string>> identifier_list
%nterm <inst> valued_inst
%nterm <inst> alloca_inst
%nterm <inst> arith_inst
%nterm <inst> arith_unary_inst
%nterm <inst> cast_inst
%nterm <inst> compare_inst
%nterm <inst> constant_inst
%nterm <inst> expand_inst
%nterm <int_or_val> integer_constant_or_identifier
%nterm <std::vector<int_or_val>> expand_shape
%nterm <inst> fuse_inst
%nterm <inst> load_inst
%nterm <inst> group_id_inst
%nterm <inst> group_size_inst
%nterm <inst> num_subgroups_inst
%nterm <inst> parallel_inst
%nterm <inst> size_inst
%nterm <inst> subgroup_id_inst
%nterm <inst> subgroup_local_id_inst
%nterm <inst> subgroup_size_inst
%nterm <inst> store_inst
%nterm <inst> subview_inst
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> optional_slice_list
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> slice_list
%nterm <std::pair<int_or_val,int_or_val>> slice
%nterm <int_or_val> slice_size

%%
prog:
    func_list {
        auto p = prog { std::make_unique<program_node>(ctx.cctx(), @prog).release() };
        ctx.program(p);
        $$ = std::move(p);
        for (auto& f : $func_list) {
            $$.add_function(std::move(f));
        }
    }
;

func_list:
    func { $$.emplace_back(std::move($func)); }
  | func_list func { $$ = std::move($1); $$.emplace_back(std::move($func)); }

func:
    FUNC {
        ctx.push_scope();
    } GLOBAL_IDENTIFIER LPAREN arguments RPAREN attributes region {
        auto loc = @FUNC;
        loc.end = @RPAREN.end;
        auto func_node = std::make_unique<function_node>($GLOBAL_IDENTIFIER, std::move($arguments),
                                                         $region.release(), loc)
                             .release();
        for (auto &attr : $attributes) {
            attr(*func_node);
        }
        $func = func{func_node};
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
        auto v = make_value(std::move($data_type));
        v.name($LOCAL_IDENTIFIER);
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
        auto const wgs = std::array<std::int32_t, 2>{static_cast<std::int32_t>($m),
                                                     static_cast<std::int32_t>($n)};
        $$ = [=](function_node &f) { f.work_group_size(wgs); };
    }
  | SUBGROUP_SIZE LPAREN INTEGER_CONSTANT RPAREN {
        if ($INTEGER_CONSTANT <= 0) {
            throw parser::syntax_error(@INTEGER_CONSTANT, "Must be a non-negative number");
        }
        auto const sgs = static_cast<std::int32_t>($INTEGER_CONSTANT);
        $$ = [=](function_node &f) { f.subgroup_size(sgs); };
    }
;


data_type:
    scalar_type { $$ = get_scalar(ctx.cctx(), $scalar_type); }
  | memref_type
  | group_type
;

scalar_type:
    INTEGER_TYPE
  | FLOATING_TYPE
;

memref_type:
    MEMREF LCHEV scalar_type mode_list optional_address_space RCHEV {
        try {
            $$ =
                get_memref(ctx.cctx(), $scalar_type, $mode_list, {}, $optional_address_space, @memref_type);
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
  | MEMREF LCHEV scalar_type mode_list COMMA STRIDED LCHEV optional_stride_list RCHEV optional_address_space RCHEV {
        if ($mode_list.size() != $optional_stride_list.size()) {
            auto loc = @scalar_type;
            loc.end = @optional_stride_list.end;
            throw syntax_error(loc, "Shape and stride list must have the same length");
        }
        try {
            $$ = get_memref(ctx.cctx(), $scalar_type, $mode_list, $optional_stride_list,
                            $optional_address_space, @memref_type);
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

mode_list:
    %empty {}
  | mode_list TIMES constant_or_dynamic { $$ = std::move($1); $$.push_back($constant_or_dynamic); }
;

optional_address_space:
    %empty { $$ = address_space::global; }
  | COMMA GLOBAL { $$ = address_space::global; }
  | COMMA LOCAL { $$ = address_space::local; }
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
    GROUP LCHEV memref_type group_offset RCHEV {
        $$ = get_group(ctx.cctx(), std::move($memref_type), $group_offset, @group_type);
    }
;

group_offset:
    %empty { $$ = std::int64_t(0); }
  | COMMA OFFSET COLON constant_or_dynamic { $$ = $constant_or_dynamic; }
;

memref_or_group_type:
    memref_type
  | group_type
;

region:
    LBRACE {
        ctx.push_scope();
    } instructions RBRACE {
        $$ = region{std::make_unique<region_node>(@region).release()};
        for (auto& i : $instructions) {
            $$.add_instruction(std::move(i));
        }
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
    axpby_inst      { $$ = std::move($1); }
  | barrier_inst    { $$ = std::move($1); }
  | gemm_inst       { $$ = std::move($1); }
  | gemv_inst       { $$ = std::move($1); }
  | ger_inst        { $$ = std::move($1); }
  | for_inst        { $$ = std::move($1); }
  | foreach_inst    { $$ = std::move($1); }
  | hadamard_inst   { $$ = std::move($1); }
  | if_inst         { $$ = std::move($1); }
  | parallel_inst   { $$ = std::move($1); }
  | var_definition  { $$ = std::move($1); }
  | store_inst      { $$ = std::move($1); }
  | sum_inst        { $$ = std::move($1); }
  | yield_inst      { $$ = std::move($1); }
;

axpby_inst:
    AXPBY transpose[ta] atomic
          var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b]
          COLON scalar_type[falpha] COMMA memref_type[ma] COMMA scalar_type[fbeta] COMMA memref_type[mb] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($b, $mb, @b, @mb);
        try {
            $$ = inst {
                std::make_unique<axpby_inst>($ta, std::move($alpha), std::move($a),
                                             std::move($beta), std::move($b), $atomic, @axpby_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

atomic:
    %empty { $$ = false; }
  | ATOMIC { $$ = true; }
;

optional_value_list:
    %empty {}
  | value_list { $$ = std::move($1); }
;

value_list:
    var { $$.push_back(std::move($var)); }
  | value_list COMMA var {
        $$ = std::move($1);
        $$.push_back(std::move($var));
    }
;

barrier_inst:
    BARRIER optional_global_attr optional_local_attr {
        int32_t fence_flags = 0;
        fence_flags |= $optional_global_attr;
        fence_flags |= $optional_local_attr;
        try {
            $$ = inst { std::make_unique<barrier_inst>(fence_flags, @barrier_inst).release() };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

optional_global_attr:
    %empty { $$ = 0; }
  | GLOBAL_ATTR { $$ = tinytc_address_space_global; }
;

optional_local_attr:
    %empty { $$ = 0; }
  | LOCAL_ATTR { $$ = tinytc_address_space_local; }
;

gemm_inst:
    GEMM transpose[ta] transpose[tb] atomic
         var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_unique<gemm_inst>($ta, $tb, std::move($alpha), std::move($a),
                                            std::move($b), std::move($beta), std::move($c), $atomic,
                                            @gemm_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

gemv_inst:
    GEMV transpose[ta] atomic
         var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_unique<gemv_inst>($ta, std::move($alpha), std::move($a), std::move($b),
                                            std::move($beta), std::move($c), $atomic, @gemv_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

transpose:
    NOTRANS { $$ = transpose::N; }
  | TRANS { $$ = transpose::T; }
;

ger_inst:
    GER atomic
         var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_unique<ger_inst>(std::move($alpha), std::move($a), std::move($b),
                                           std::move($beta), std::move($c), $atomic, @ger_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

for_inst:
    FOR LOCAL_IDENTIFIER[loop_var]
        EQUALS var[from] COMMA var[to] optional_step
        for_loop_var_type {
        check_scalar_type(ctx.cctx(), $from, $for_loop_var_type, @from, @for_loop_var_type);
        check_scalar_type(ctx.cctx(), $to, $for_loop_var_type, @to, @for_loop_var_type);
        if ($optional_step) {
            check_scalar_type(ctx.cctx(), $optional_step, $for_loop_var_type, @optional_step, @for_loop_var_type);
        }
        auto v = make_value(get_scalar(ctx.cctx(), $for_loop_var_type));
        v.name($loop_var);
        ctx.val($loop_var, std::move(v), @loop_var);
    } region {
        try {
            $$ = inst {
                std::make_unique<for_inst>(ctx.val($loop_var, @loop_var), $from, $to,
                                           $optional_step, std::move($region), @for_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

optional_step:
    %empty { $$ = {}; }
  | COMMA var { $$ = $var; }

foreach_inst:
    FOREACH LOCAL_IDENTIFIER[loop_var]
        EQUALS var[from] COMMA var[to] for_loop_var_type {
        check_scalar_type(ctx.cctx(), $from, $for_loop_var_type, @from, @for_loop_var_type);
        check_scalar_type(ctx.cctx(), $to, $for_loop_var_type, @to, @for_loop_var_type);
        auto v = make_value(get_scalar(ctx.cctx(), $for_loop_var_type));
        v.name($loop_var);
        ctx.val($loop_var, std::move(v), @loop_var);
    } region {
        try {
            $$ = inst {
                std::make_unique<foreach_inst>(ctx.val($loop_var, @loop_var), $from, $to,
                                               std::move($region), @foreach_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
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
        if (static_cast<std::int64_t>($identifier_list.size()) != $$->num_results()) {
            throw syntax_error(
                @identifier_list,
                "Number of identifiers does not equal number of returned values");
        }
        auto results = $$->result_begin();
        for (std::int64_t i = 0; i < $$->num_results(); ++i) {
            results[i]->name($identifier_list[i]);
            ctx.val($identifier_list[i], results[i], @identifier_list);
        }
    }
;

identifier_list:
    LOCAL_IDENTIFIER { $$.push_back($LOCAL_IDENTIFIER); }
  | identifier_list COMMA LOCAL_IDENTIFIER { $$ = std::move($1); $$.push_back($LOCAL_IDENTIFIER); }
;


hadamard_inst:
    HADAMARD atomic
         var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c]
         COLON scalar_type[falpha] COMMA memref_type[ma] COMMA memref_type[mb] COMMA scalar_type[fbeta]
         COMMA memref_type[mc] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_type($b, $mb, @b, @mb);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($c, $mc, @c, @mc);
        try {
            $$ = inst {
                std::make_unique<hadamard_inst>(std::move($alpha), std::move($a), std::move($b),
                                                std::move($beta), std::move($c), $atomic,
                                                @hadamard_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

sum_inst:
    SUM transpose[ta] atomic
          var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b]
          COLON scalar_type[falpha] COMMA memref_type[ma] COMMA scalar_type[fbeta] COMMA memref_type[mb] {
        check_scalar_type(ctx.cctx(), $alpha, $falpha, @alpha, @falpha);
        check_type($a, $ma, @a, @ma);
        check_scalar_type(ctx.cctx(), $beta, $fbeta, @beta, @fbeta);
        check_type($b, $mb, @b, @mb);
        try {
            $$ = inst {
                std::make_unique<sum_inst>($ta, std::move($alpha), std::move($a), std::move($beta),
                                           std::move($b), $atomic, @sum_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

yield_inst:
    YIELD optional_value_list[vals] COLON optional_scalar_type_list[tys] {
        if ($vals.size() != $tys.size()) {
            location loc = @vals;
            loc.end = @tys.end;
            throw syntax_error(loc, "Identifier and scalar type list must have the same length");
        }
        for (std::size_t i = 0; i < $vals.size(); ++i) {
            if (auto ty = dyn_cast<scalar_data_type>($tys[i]); ty) {
                check_scalar_type(ctx.cctx(), $vals[i], ty->ty(), @vals, @tys);
            } else {
                throw syntax_error(@tys, "Yield only accepts scalar types");
            }
        }
        $$ = inst{std::make_unique<yield_inst>(std::move($vals)).release()};
    }
;

valued_inst:
    alloca_inst             { $$ = std::move($1); }
  | arith_inst              { $$ = std::move($1); }               
  | arith_unary_inst        { $$ = std::move($1); }
  | cast_inst               { $$ = std::move($1); }
  | compare_inst            { $$ = std::move($1); }
  | constant_inst           { $$ = std::move($1); }
  | expand_inst             { $$ = std::move($1); }
  | fuse_inst               { $$ = std::move($1); }
  | group_id_inst           { $$ = std::move($1); }
  | group_size_inst         { $$ = std::move($1); }
  | if_inst                 { $$ = std::move($1); }
  | load_inst               { $$ = std::move($1); }
  | num_subgroups_inst      { $$ = std::move($1); }
  | size_inst               { $$ = std::move($1); }
  | subgroup_id_inst        { $$ = std::move($1); }
  | subgroup_local_id_inst  { $$ = std::move($1); }
  | subgroup_size_inst      { $$ = std::move($1); }
  | subview_inst            { $$ = std::move($1); }
;

alloca_inst:
    ALLOCA RETURNS memref_type {
        try {
            $$ = inst {
                std::make_unique<alloca_inst>(std::move($memref_type), @alloca_inst).release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

arith_inst:
    ARITH ARITHMETIC var[a] COMMA var[b] COLON scalar_type[ty] {
        check_scalar_type(ctx.cctx(), $a, $ty, @a, @ty);
        check_scalar_type(ctx.cctx(), $b, $ty, @b, @ty);
        try {
            $$ = inst {
                std::make_unique<arith_inst>($ARITHMETIC, std::move($a), std::move($b), @arith_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

arith_unary_inst:
    ARITH ARITHMETIC_UNARY var[a] COLON scalar_type[ty] {
        check_scalar_type(ctx.cctx(), $a, $ty, @a, @ty);
        try {
            $$ = inst {
                std::make_unique<arith_unary_inst>($ARITHMETIC_UNARY, std::move($a),
                                                   @arith_unary_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;


cast_inst:
    CAST var[a] COLON scalar_type[from] RETURNS scalar_type[to] {
        check_scalar_type(ctx.cctx(), $a, $from, @a, @from);
        try {
            $$ = inst { std::make_unique<cast_inst>(std::move($a), $to, @cast_inst).release() };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

compare_inst:
    CMP CMP_CONDITION var[a] COMMA var[b] COLON scalar_type[ty] {
        check_scalar_type(ctx.cctx(), $a, $ty, @a, @ty);
        check_scalar_type(ctx.cctx(), $b, $ty, @b, @ty);
        try {
            $$ = inst {
                std::make_unique<compare_inst>($CMP_CONDITION, std::move($a), std::move($b),
                                               @compare_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

constant_inst:
    CONSTANT LSQBR FLOATING_CONSTANT[re] COMMA FLOATING_CONSTANT[im] RSQBR RETURNS data_type {
        try {
            $$ = inst {
                std::make_unique<constant_inst>(std::complex<double>{$re, $im}, $data_type, @constant_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
  | CONSTANT FLOATING_CONSTANT RETURNS data_type {
        try {
            $$ = inst {
                std::make_unique<constant_inst>($FLOATING_CONSTANT, $data_type, @constant_inst).release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
  | CONSTANT INTEGER_CONSTANT RETURNS data_type {
        try {
            $$ = inst {
                std::make_unique<constant_inst>($INTEGER_CONSTANT, $data_type, @constant_inst).release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

expand_inst:
    EXPAND var LSQBR INTEGER_CONSTANT[expanded_mode] RETURNS expand_shape RSQBR COLON memref_type {
        if ($var->ty() != $memref_type) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            auto static_shape = std::vector<std::int64_t>{};
            static_shape.reserve($expand_shape.size());
            auto dynamic_shape = std::vector<value>{};
            dynamic_shape.reserve($expand_shape.size());
            for (auto &s : $expand_shape) {
                std::visit(overloaded{
                    [&](std::int64_t i) { static_shape.push_back(i); },
                    [&](value const &v) {
                        static_shape.push_back(dynamic);
                        dynamic_shape.push_back(v);
                    },
                }, s);
            }
            $$ = inst {
                std::make_unique<expand_inst>(std::move($var), $expanded_mode, std::move(static_shape),
                                              std::move(dynamic_shape), @expand_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        } catch (std::exception const& e) {
            error(@expand_inst, e.what());
        }
    }
;

expand_shape:
    integer_constant_or_identifier[a] TIMES integer_constant_or_identifier[b] {
        $$ = std::vector<std::variant<std::int64_t,value>>{$a, $b};
    }
  | expand_shape TIMES integer_constant_or_identifier[a] { $$ = std::move($1); $$.push_back($a); }
;

integer_constant_or_identifier:
    var {
        check_scalar_type(ctx.cctx(), $var, scalar_type::index, @var, @var);
        $$ = $var;
    }
  | INTEGER_CONSTANT {
        $$ = $INTEGER_CONSTANT;
    }
;

fuse_inst:
    FUSE var LSQBR INTEGER_CONSTANT[from] COMMA INTEGER_CONSTANT[to] RSQBR COLON memref_type {
        if ($var->ty() != $memref_type) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_unique<fuse_inst>(std::move($var), $from, $to, @fuse_inst).release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

load_inst:
    LOAD var LSQBR optional_value_list RSQBR COLON memref_or_group_type {
        if ($var->ty() != $memref_or_group_type) {
            auto loc = @var;
            loc.end = @memref_or_group_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_unique<load_inst>(std::move($var), std::move($optional_value_list),
                                            @load_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

store_inst:
    STORE var[a] COMMA var[b] LSQBR optional_value_list RSQBR COLON memref_type {
        if ($b->ty() != $memref_type) {
            auto loc = @b;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst {
                std::make_unique<store_inst>(std::move($a), std::move($b),
                                             std::move($optional_value_list), @store_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

group_id_inst:
    GROUP_ID { $$ = inst{std::make_unique<group_id_inst>(ctx.cctx().get(), @GROUP_ID).release()}; }
;

group_size_inst:
    GROUP_SIZE { $$ = inst{std::make_unique<group_size_inst>(ctx.cctx().get(), @GROUP_SIZE).release()}; }
;

if_inst:
    IF var[condition] optional_returned_values region else_region {
        check_scalar_type(ctx.cctx(), $condition, scalar_type::i1, @condition, @condition);
        $$ = inst{std::make_unique<if_inst>(std::move($condition), std::move($region),
                                            std::move($else_region),
                                            std::move($optional_returned_values))
                      .release()};
        $$->loc(@if_inst);
    }
;

else_region:
    %empty { $$ = {}; }
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
    scalar_type { $$.push_back(get_scalar(ctx.cctx(), $scalar_type)); }
  | scalar_type_list COMMA scalar_type {
        $$ = std::move($1); $$.push_back(get_scalar(ctx.cctx(), $scalar_type));
    }
;

num_subgroups_inst:
    NUM_SUBGROUPS { $$ = inst{std::make_unique<num_subgroups_inst>(ctx.cctx().get(), @NUM_SUBGROUPS).release()}; }
;

parallel_inst:
    PARALLEL region {
        $$ = inst{std::make_unique<parallel_inst>(std::move($region), @parallel_inst) .release()};
    }
;

size_inst:
    SIZE var LSQBR INTEGER_CONSTANT[mode] RSQBR COLON memref_type {
        if ($var->ty() != $memref_type) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            $$ = inst { std::make_unique<size_inst>(std::move($var), $mode, @size_inst).release() };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        }
    }
;

subgroup_id_inst:
    SUBGROUP_ID { $$ = inst{std::make_unique<subgroup_id_inst>(ctx.cctx().get(), @SUBGROUP_ID).release()}; }
;

subgroup_local_id_inst:
    SUBGROUP_LOCAL_ID {
        $$ = inst{std::make_unique<subgroup_local_id_inst>(ctx.cctx().get(), @SUBGROUP_LOCAL_ID).release()};
    }
;

subgroup_size_inst:
    SUBGROUP_SIZE { $$ = inst{std::make_unique<subgroup_size_inst>(ctx.cctx().get(), @SUBGROUP_SIZE).release()}; }
;

subview_inst:
    SUBVIEW var LSQBR optional_slice_list RSQBR COLON memref_type {
        if ($var->ty() != $memref_type) {
            auto loc = @var;
            loc.end = @memref_type.end;
            throw parser::syntax_error(loc, "Type of SSA value does not match operand type");
        }
        try {
            auto static_offsets = std::vector<std::int64_t>{};
            auto static_sizes = std::vector<std::int64_t>{};
            auto offsets = std::vector<value>{};
            auto sizes = std::vector<value>{};
            static_offsets.reserve($optional_slice_list.size());
            static_sizes.reserve($optional_slice_list.size());
            offsets.reserve($optional_slice_list.size());
            sizes.reserve($optional_slice_list.size());
            for (auto &s : $optional_slice_list) {
                std::visit(overloaded{
                    [&](std::int64_t i) { static_offsets.push_back(i); },
                    [&](value const &v) {
                        static_offsets.push_back(dynamic);
                        offsets.push_back(v);
                    },
                }, s.first);
                std::visit(overloaded{
                    [&](std::int64_t i) { static_sizes.push_back(i); },
                    [&](value const &v) {
                        static_sizes.push_back(dynamic);
                        sizes.push_back(v);
                    },
                }, s.second);
            }
            $$ = inst {
                std::make_unique<subview_inst>(std::move($var), std::move(static_offsets), std::move(static_sizes),
                                               std::move(offsets), std::move(sizes), @subview_inst)
                    .release()
            };
        } catch (compilation_error const &e) {
            error(e.loc(), e.what());
            YYERROR;
        } catch (std::exception const& e) {
            error(@subview_inst, e.what());
        }

    }
;

optional_slice_list:
    %empty {}
  | slice_list { $$ = std::move($1); }
;

slice_list:
    slice {
        $$.emplace_back(std::move($slice));
    }
  | slice_list COMMA slice {
        $$ = std::move($1);
        $$.emplace_back(std::move($slice));
    }
;

slice:
    integer_constant_or_identifier slice_size { $$ = std::make_pair(std::move($1), std::move($2)); }
;

slice_size:
    %empty { $$ = {}; }
  | COLON integer_constant_or_identifier { $$ = $2; }
;

%%

namespace tinytc {
void parser::error(location_type const& l, std::string const& m) {
    ctx.report_error(l, m);
}
}
