// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "tinytc/types.h"
    #include "tinytc/types.hpp"
    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <memory>
    #include <new>
    #include <utility>
    #include <tuple>
    #include <variant>

    namespace tinytc {
        class parse_context;
        class lexer;

        using int_or_val = std::variant<std::int64_t, tinytc_value_t>;

        using identifier = std::variant<std::int64_t, std::string>;
        struct param_attrs {
            identifier id;
            location loc;
            tinytc_attr_t dict;
        };
    }
}

%code {
    #include "compiler_context.hpp"
    #include "error.hpp"
    #include "node/attr.hpp"
    #include "node/func.hpp"
    #include "node/inst.hpp"
    #include "node/inst_view.hpp"
    #include "node/region.hpp"
    #include "node/value.hpp"
    #include "parser/lexer.hpp"
    #include "parser/parse_context.hpp"
    #include "tinytc/builder.hpp"
    #include "tinytc/core.hpp"
    #include "util/ilist.hpp"
    #include "util/iterator.hpp"
    #include "util/overloaded.hpp"

    #include <complex>
    #include <cstdint>
    #include <initializer_list>
    #include <iterator>
    #include <sstream>
    #include <utility>
    #include <vector>

    namespace tinytc {
    void report_error(tinytc_compiler_context_t cctx, compilation_error const& e) {
        if (e.extra_info().size() > 0) {
            auto what = (std::ostringstream{} << e.what() << " (" << e.extra_info() << ')').str();
            cctx->report_error(e.loc(), e.ref_values(), what.c_str());
        } else {
            cctx->report_error(e.loc(), e.ref_values(), e.what());
        }
    }
    template <typename F>
    void yytry(parse_context& ctx, F&& f, location const& loc = {}) {
        try {
            f();
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            throw parser::syntax_error({}, "");
        } catch (status st) {
            throw parser::syntax_error(loc, to_string(st));
        } catch (std::exception const &e) {
            throw parser::syntax_error(loc, e.what());
        }
    }
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
    ATTRIBUTES      "attributes"
    ARROW           "->"
    DYNAMIC         "?"
    ATOMIC          ".atomic"
    INIT            "init"
    LOCAL           "local"
    GLOBAL          "global"
    LOCAL_ATTR      ".local"
    GLOBAL_ATTR     ".global"
    BOOLEAN         "bool"
    COOPMATRIX      "coopmatrix"
    MEMREF          "memref"
    GROUP           "group"
    OFFSET          "offset"
    STRIDED         "strided"
;
%token
    I8_TYPE    "i8"
    I16_TYPE   "i16"
    I32_TYPE   "i32"
    I64_TYPE   "i64"
    INDEX_TYPE "index"
    BF16_TYPE  "bf16"
    F16_TYPE   "f16"
    F32_TYPE   "f32"
    F64_TYPE   "f64"
    C32_TYPE   "c32"
    C64_TYPE   "64"
;

%token
    ALLOCA                        "alloca"
    ATOMIC_STORE                  "atomic_store"
    ATOMIC_ADD                    "atomic_add"
    ATOMIC_MAX                    "atomic_max"
    ATOMIC_MIN                    "atomic_min"
    BARRIER                       "barrier"
    CAST                          "cast"
    CONSTANT                      "constant"
    COOPERATIVE_MATRIX_APPLY      "cooperative_matrix_apply"
    COOPERATIVE_MATRIX_EXTRACT    "cooperative_matrix_extract"
    COOPERATIVE_MATRIX_INSERT     "cooperative_matrix_insert"
    COOPERATIVE_MATRIX_LOAD       "cooperative_matrix_load"
    COOPERATIVE_MATRIX_MUL_ADD    "cooperative_matrix_mul_add"
    COOPERATIVE_MATRIX_PREFETCH   "cooperative_matrix_prefetch"
    COOPERATIVE_MATRIX_REDUCE_ADD "cooperative_matrix_reduce_add"
    COOPERATIVE_MATRIX_REDUCE_MAX "cooperative_matrix_reduce_max"
    COOPERATIVE_MATRIX_REDUCE_MIN "cooperative_matrix_reduce_min"
    COOPERATIVE_MATRIX_SCALE      "cooperative_matrix_scale"
    COOPERATIVE_MATRIX_STORE      "cooperative_matrix_store"
    EXPAND                        "expand"
    FUSE                          "fuse"
    LOAD                          "load"
    IF                            "if"
    ELSE                          "else"
    PARALLEL                      "parallel"
    SIZE                          "size"
    SUBGROUP_BROADCAST            "subgroup_broadcast"
    SUBVIEW                       "subview"
    STORE                         "store"
    YIELD                         "yield"
    ADD                           "add"
    SUB                           "sub"
    MUL                           "mul"
    DIV                           "div"
    REM                           "rem"
    SHL                           "shl"
    SHR                           "shr"
    AND                           "and"
    OR                            "or"
    XOR                           "xor"
    MIN                           "min"
    MAX                           "max"
    ABS                           "abs"
    NEG                           "neg"
    NOT                           "not"
    CONJ                          "conj"
    IM                            "im"
    RE                            "re"
    AXPBY                         "axpby"
    CUMSUM                        "cumsum"
    SUM                           "sum"
    GEMM                          "gemm"
    GEMV                          "gemv"
    GER                           "ger"
    HADAMARD                      "hadamard"
    GROUP_ID                      "group_id"
    NUM_GROUPS                    "num_groups"
    NUM_SUBGROUPS                 "num_subgroups"
    SUBGROUP_SIZE                 "subgroup_size"
    SUBGROUP_ID                   "subgroup_id"
    SUBGROUP_LINEAR_ID            "subgroup_linear_id"
    SUBGROUP_LOCAL_ID             "subgroup_local_id"
    EQUAL                         "equal"
    NOT_EQUAL                     "not_equal"
    GREATER_THAN                  "greater_than"
    GREATER_THAN_EQUAL            "greater_than_equal"
    LESS_THAN                     "less_than"
    LESS_THAN_EQUAL               "less_than_equal"
    FOR                           "for"
    FOREACH                       "foreach"
    COS                           "cos"
    SIN                           "sin"
    EXP                           "exp"
    EXP2                          "exp2"
    NATIVE_COS                    "native_cos"
    NATIVE_SIN                    "native_sin"
    NATIVE_EXP                    "native_exp"
    NATIVE_EXP2                   "native_exp2"
    SUBGROUP_EXCLUSIVE_SCAN_ADD   "subgroup_exclusive_scan_add"
    SUBGROUP_EXCLUSIVE_SCAN_MAX   "subgroup_exclusive_scan_max"
    SUBGROUP_EXCLUSIVE_SCAN_MIN   "subgroup_exclusive_scan_min"
    SUBGROUP_INCLUSIVE_SCAN_ADD   "subgroup_inclusive_scan_add"
    SUBGROUP_INCLUSIVE_SCAN_MAX   "subgroup_inclusive_scan_max"
    SUBGROUP_INCLUSIVE_SCAN_MIN   "subgroup_inclusive_scan_min"
    SUBGROUP_REDUCE_ADD           "subgroup_reduce_add"
    SUBGROUP_REDUCE_MAX           "subgroup_reduce_max"
    SUBGROUP_REDUCE_MIN           "subgroup_reduce_min"
;
%token <identifier> LOCAL_IDENTIFIER
%token <std::string> GLOBAL_IDENTIFIER
%token <std::string> ATTR_NAME
%token <std::string> STRING
%token <bool> BOOLEAN_CONSTANT
%token <std::int64_t> INTEGER_CONSTANT
%token <double> FLOATING_CONSTANT
%token <comp3> COMP3
%token <reduce_mode> REDUCE_MODE
%token <matrix_use> MATRIX_USE
%token <checked_flag> CHECKED
%token <transpose> TRANSPOSE
%token <memory_scope> MEMORY_SCOPE
%token <memory_semantics> MEMORY_SEMANTICS

%nterm <std::vector<unique_handle<tinytc_func_t>>> func_list
%nterm <unique_handle<tinytc_func_t>> func
%nterm <std::pair<std::vector<param_attrs>,std::vector<tinytc_type_t>>> parameters
%nterm <std::pair<param_attrs,tinytc_type_t>> parameter
%nterm <tinytc_attr_t> function_attributes
%nterm <tinytc_attr_t> attribute
%nterm <tinytc_attr_t> array_attribute
%nterm <std::vector<tinytc_attr_t>> attribute_list
%nterm <tinytc_attr_t> dictionary_attribute
%nterm <std::vector<tinytc_named_attr_t>> named_attribute_list
%nterm <tinytc_named_attr_t> named_attribute
%nterm <tinytc_attr_t> attribute_name
%nterm <tinytc_attr_t> optional_dictionary_attribute
%nterm <tinytc_type_t> data_type
%nterm <tinytc_type_t> boolean_type
%nterm <tinytc_type_t> scalar_type
%nterm <tinytc_type_t> coopmatrix_type
%nterm <tinytc_type_t> memref_type
%nterm <address_space> optional_address_space
%nterm <std::vector<std::int64_t>> mode_list
%nterm <std::vector<std::int64_t>> optional_stride_list
%nterm <std::vector<std::int64_t>> stride_list
%nterm <std::int64_t> constant_or_dynamic
%nterm <tinytc_type_t> group_type
%nterm <std::int64_t> group_offset
%nterm <tinytc_value_t> var
%nterm <unique_handle<tinytc_inst_t>> instruction
%nterm <bool> atomic
%nterm <std::vector<tinytc_value_t>> optional_value_list
%nterm <std::vector<tinytc_value_t>> value_list
%nterm <std::int32_t> optional_global_attr
%nterm <std::int32_t> optional_local_attr
%nterm <transpose> transpose_opt
%nterm <std::pair<transpose,transpose>> transpose_opt2
%nterm <unique_handle<tinytc_inst_t>> for_inst
%nterm <std::tuple<std::vector<identifier>, std::vector<tinytc_value_t>, std::vector<tinytc_type_t>>> optional_loop_carried_values
%nterm <std::pair<std::vector<identifier>, std::vector<tinytc_value_t>>> init_value_list
%nterm <std::pair<identifier, tinytc_value_t>> init_value
%nterm <tinytc_value_t> optional_step
%nterm <unique_handle<tinytc_inst_t>> if_inst
%nterm <std::vector<tinytc_type_t>> optional_returned_values
%nterm <std::vector<tinytc_type_t>> optional_return_type_list
%nterm <std::vector<tinytc_type_t>> return_type_list
%nterm <std::vector<identifier>> identifier_list
%nterm <unique_handle<tinytc_inst_t>> valued_inst
%nterm <checked_flag> checked
%nterm <int_or_val> integer_constant_or_identifier
%nterm <std::vector<int_or_val>> expand_shape
%nterm <store_flag> store_flag
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> optional_slice_list
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> slice_list
%nterm <std::pair<int_or_val,int_or_val>> slice
%nterm <int_or_val> slice_size
%nterm <memory_scope> memory_scope
%nterm <memory_semantics> memory_semantics

%%
prog:
    func_list {
        auto p = create_prog(ctx.cctx(), @prog);
        for (auto& f : $func_list) {
            add_function(p.get(), std::move(f));
        }
        ctx.program(std::move(p));
    }
;

func_list:
    func { $$.emplace_back(std::move($func)); }
  | func_list func { $$ = std::move($1); $$.emplace_back(std::move($func)); }

func:
    FUNC GLOBAL_IDENTIFIER LPAREN parameters RPAREN
        function_attributes <unique_handle<tinytc_func_t>>{
        auto loc = @FUNC;
        loc.end = @RPAREN.end;
        yytry(
            ctx,
            [&] {
                ctx.add_global_name($GLOBAL_IDENTIFIER, loc);
                auto void_ty = get<void_type>(ctx.cctx());
                auto func_node = create_func($GLOBAL_IDENTIFIER, $parameters.second, void_ty, loc);
                func_node->attr($function_attributes);
                ctx.push_scope();
                auto name_it = $parameters.first.begin();
                for (auto &p : func_node->params()) {
                    ctx.val(name_it->id, p, name_it->loc);
                    if (name_it->dict != 0) {
                        func_node->param_attr(name_it - $parameters.first.begin(), name_it->dict);
                    }
                    ++name_it;
                }
                ctx.push_region(&func_node->body());
                $$ = std::move(func_node);
            },
            loc);
    }[prototype] region {
        ctx.pop_region();
        ctx.pop_scope();
        $$ = std::move($prototype);
    }
;

parameters:
    %empty {}
  | parameter {
        $$.first.emplace_back(std::move($parameter.first));
        $$.second.emplace_back(std::move($parameter.second));
    }
  | parameters COMMA parameter {
        $$.first = std::move($1.first);
        $$.second = std::move($1.second);
        $$.first.emplace_back(std::move($parameter.first));
        $$.second.emplace_back(std::move($parameter.second));
    }
;

parameter:
    LOCAL_IDENTIFIER COLON data_type optional_dictionary_attribute[dict] {
        $$ = std::make_pair(param_attrs{$LOCAL_IDENTIFIER, @LOCAL_IDENTIFIER, $dict}, $data_type);
    }
;

function_attributes:
    %empty {}
  | ATTRIBUTES dictionary_attribute { $$ = $dictionary_attribute; }
;

attribute:
    array_attribute { $$ = $array_attribute; }
  | BOOLEAN_CONSTANT { $$ = boolean_attr::get(ctx.cctx(), $BOOLEAN_CONSTANT); }
  | dictionary_attribute { $$ = $dictionary_attribute; }
  | INTEGER_CONSTANT { $$ = integer_attr::get(ctx.cctx(), $INTEGER_CONSTANT); }
  | STRING { $$ = string_attr::get(ctx.cctx(), $STRING); }
;

array_attribute:
    LSQBR RSQBR { $$ = array_attr::get(ctx.cctx(), {}); }
  | LSQBR attribute_list RSQBR { $$ = array_attr::get(ctx.cctx(), $attribute_list); }

attribute_list:
    attribute { $$.push_back($attribute); }
  | attribute_list COMMA attribute { $$ = std::move($1); $$.push_back($attribute); }
;

dictionary_attribute:
    LBRACE RBRACE { $$ = dictionary_attr::get(ctx.cctx(), {}); }
  | LBRACE named_attribute_list RBRACE {
        dictionary_attr::sort($named_attribute_list);
        $$ = dictionary_attr::get(ctx.cctx(), $named_attribute_list);
    }
;

named_attribute_list:
    named_attribute { $$.push_back($named_attribute); }
  | named_attribute_list COMMA named_attribute { $$ = std::move($1); $$.push_back($named_attribute); }
;

named_attribute:
    attribute_name EQUALS attribute {
        $$ = tinytc_named_attr_t{$attribute_name, $attribute};
    }
;

attribute_name:
    ATTR_NAME     { $$ = string_attr::get(ctx.cctx(), $ATTR_NAME); }
  | SUBGROUP_SIZE { $$ = string_attr::get(ctx.cctx(), "subgroup_size"); }
  | STRING        { $$ = string_attr::get(ctx.cctx(), $STRING); }

optional_dictionary_attribute:
    %empty { $$ = nullptr; }
  | dictionary_attribute { $$ = $dictionary_attribute; }
;


data_type:
    boolean_type
  | coopmatrix_type
  | group_type
  | memref_type
  | scalar_type
;

boolean_type:
    BOOLEAN { yytry(ctx, [&] { $$ = get<boolean_type>(ctx.cctx()); }, @boolean_type); }
;

scalar_type:
    I8_TYPE    { yytry(ctx, [&] { $$ = get<i8_type>(ctx.cctx());  }, @scalar_type); }
  | I16_TYPE   { yytry(ctx, [&] { $$ = get<i16_type>(ctx.cctx());  }, @scalar_type); }
  | I32_TYPE   { yytry(ctx, [&] { $$ = get<i32_type>(ctx.cctx());  }, @scalar_type); }
  | I64_TYPE   { yytry(ctx, [&] { $$ = get<i64_type>(ctx.cctx());  }, @scalar_type); }
  | INDEX_TYPE { yytry(ctx, [&] { $$ = get<index_type>(ctx.cctx());  }, @scalar_type); }
  | BF16_TYPE  { yytry(ctx, [&] { $$ = get<bf16_type>(ctx.cctx());  }, @scalar_type); }
  | F16_TYPE   { yytry(ctx, [&] { $$ = get<f16_type>(ctx.cctx());  }, @scalar_type); }
  | F32_TYPE   { yytry(ctx, [&] { $$ = get<f32_type>(ctx.cctx());  }, @scalar_type); }
  | F64_TYPE   { yytry(ctx, [&] { $$ = get<f64_type>(ctx.cctx());  }, @scalar_type); }
  | C32_TYPE   { yytry(ctx, [&] { $$ = get<c32_type>(ctx.cctx());  }, @scalar_type); }
  | C64_TYPE   { yytry(ctx, [&] { $$ = get<c64_type>(ctx.cctx());  }, @scalar_type); }
;

coopmatrix_type:
    COOPMATRIX LCHEV scalar_type TIMES INTEGER_CONSTANT[rows] TIMES INTEGER_CONSTANT[cols] COMMA MATRIX_USE RCHEV {
        yytry(
            ctx, [&] { $$ = get<coopmatrix_type>($scalar_type, $rows, $cols, $MATRIX_USE); },
            @coopmatrix_type);
    }
;

memref_type:
    MEMREF LCHEV scalar_type mode_list optional_address_space RCHEV {
        yytry(
            ctx,
            [&] {
                auto empty = array_view<std::int64_t>{};
                $$ = get<memref_type>($scalar_type, $mode_list, empty, $optional_address_space);
            },
            @memref_type);
    }
  | MEMREF LCHEV scalar_type mode_list COMMA STRIDED LCHEV optional_stride_list RCHEV optional_address_space RCHEV {
        if ($mode_list.size() != $optional_stride_list.size()) {
            auto loc = @scalar_type;
            loc.end = @optional_stride_list.end;
            throw syntax_error(loc, "Shape and stride list must have the same length");
        }
        yytry(
            ctx,
            [&] {
                $$ = get<memref_type>($scalar_type, $mode_list, $optional_stride_list,
                                      $optional_address_space);
            },
            @memref_type);
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
    GROUP LCHEV memref_type TIMES constant_or_dynamic[group_size] group_offset RCHEV {
        yytry(
            ctx, [&] { $$ = get<group_type>(std::move($memref_type), $group_size, $group_offset); },
            @group_type);
    }
;

group_offset:
    %empty { $$ = std::int64_t(0); }
  | COMMA OFFSET COLON constant_or_dynamic { $$ = $constant_or_dynamic; }
;

var:
    LOCAL_IDENTIFIER { $$ = ctx.val($LOCAL_IDENTIFIER, @LOCAL_IDENTIFIER); }
;

region:
    LBRACE { ctx.push_scope(); } instructions { ctx.pop_scope(); } RBRACE {}
;

instructions:
    %empty {}
  | instructions instruction {
        if (!ctx.has_regions()) {
            error(@instruction, "Internal error: missing region");
            YYERROR;
        }
        tinytc_region_t reg = ctx.top_region();
        reg->insts().push_back($instruction.release());
    }
;

instruction:
    AXPBY atomic transpose_opt[ta] var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b] {
        yytry(ctx, [&] {
            $$ = axpby_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($beta),
                                    std::move($b), @instruction);
        });
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

instruction:
    BARRIER optional_global_attr optional_local_attr {
        int32_t fence_flags = 0;
        fence_flags |= $optional_global_attr;
        fence_flags |= $optional_local_attr;
        yytry(ctx, [&] { $$ = barrier_inst::create(fence_flags, @instruction); });
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

instruction:
    CUMSUM atomic var[alpha] COMMA var[a] COMMA INTEGER_CONSTANT[mode] COMMA var[beta] COMMA var[b] {
        yytry(ctx, [&] {
            $$ = cumsum_inst::create($atomic, $mode, std::move($alpha), std::move($a), std::move($beta),
                                     std::move($b), @instruction);
        });
    }
;

instruction:
    GEMM atomic transpose_opt2[tr] var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        yytry(ctx, [&] {
            $$ = gemm_inst::create($atomic, $tr.first, $tr.second, std::move($alpha), std::move($a),
                                   std::move($b), std::move($beta), std::move($c), @instruction);
        });
    }
;

instruction:
    GEMV atomic transpose_opt[ta] var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        yytry(ctx, [&] {
            $$ = gemv_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($b),
                                   std::move($beta), std::move($c), @instruction);
        });
    }
;

transpose_opt:
    %empty { $$ = transpose::N; }
  | TRANSPOSE { $$ = $TRANSPOSE; }
;

transpose_opt2:
    %empty { $$ = std::make_pair(transpose::N, transpose::N); }
  | TRANSPOSE transpose_opt { $$ = std::make_pair($TRANSPOSE, $transpose_opt); }
;

instruction:
    GER atomic var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        yytry(ctx, [&] {
            $$ = ger_inst::create($atomic, std::move($alpha), std::move($a), std::move($b),
                                  std::move($beta), std::move($c), @instruction);
        });
    }
;

instruction: for_inst { $$ = std::move($1); } ;
valued_inst: for_inst { $$ = std::move($1); } ;
for_inst:
    FOR LOCAL_IDENTIFIER[loop_var] EQUALS var[from] COMMA var[to] optional_step optional_loop_carried_values[lcv] <unique_handle<tinytc_inst_t>> {
        yytry(ctx, [&] {
            auto &[lcv_id, lcv_init, lcv_type] = $lcv;
            location loc = @FOR;
            loc.end = @lcv.end;
            $$ = for_inst::create($from, $to, $optional_step, lcv_init, lcv_type, loc);
            auto inode = for_inst($$.get());
            ctx.push_scope();
            auto &loop_var = inode.loop_var();
            ctx.val($loop_var, loop_var, @loop_var);
            for (std::int32_t i = 0; i < inode.get().num_results(); ++i) {
                ctx.val(lcv_id[i], inode.iter_arg(i), @lcv);
            }
            ctx.push_region(&inode.body());
        });
    }[loop_header] region optional_dictionary_attribute {
        ctx.pop_region();
        ctx.pop_scope();
        $loop_header->attr($optional_dictionary_attribute);
        $$ = std::move($loop_header);
    }
;

optional_step:
    %empty { $$ = {}; }
  | COMMA var { $$ = $var; }
;

optional_loop_carried_values:
    %empty { $$ = {}; }
  | INIT LPAREN init_value_list RPAREN ARROW LPAREN return_type_list RPAREN {
        $$ = std::make_tuple(std::move($init_value_list.first), std::move($init_value_list.second),
                             std::move($return_type_list));
    }
;

init_value_list:
    init_value {
        $$.first.emplace_back($init_value.first);
        $$.second.emplace_back($init_value.second);
    }
  | init_value_list COMMA init_value {
        $$ = std::move($1);
        $$.first.emplace_back($init_value.first);
        $$.second.emplace_back($init_value.second);
    }
;

init_value:
    LOCAL_IDENTIFIER EQUALS var { $$ = std::make_pair($LOCAL_IDENTIFIER, $var); }
;

instruction:
    FOREACH LPAREN identifier_list[loop_var] RPAREN EQUALS
            LPAREN value_list[from] RPAREN COMMA LPAREN value_list[to] RPAREN[header_end] <unique_handle<tinytc_inst_t>>{
        yytry(ctx, [&] {
            location loc = @FOREACH;
            loc.end = @header_end.end;
            $$ = foreach_inst::create($from, $to, loc);
            auto inode = foreach_inst($$.get());
            ctx.push_scope();
            auto loop_vars = inode.loop_vars().begin();
            for (std::int64_t i = 0; i < inode.dim(); ++i) {
                ctx.val($loop_var[i], loop_vars[i], @loop_var);
            }
            ctx.push_region(&inode.body());
        });
    }[loop_header] region {
        ctx.pop_region();
        ctx.pop_scope();
        $$ = std::move($loop_header);
    }
;

instruction:
    identifier_list EQUALS valued_inst {
        $$ = std::move($valued_inst);
        if (static_cast<std::int64_t>($identifier_list.size()) != $$->num_results()) {
            throw syntax_error(
                @identifier_list,
                "Number of identifiers does not equal number of returned values");
        }
        auto results = $$->result_begin();
        for (std::int64_t i = 0; i < $$->num_results(); ++i) {
            ctx.val($identifier_list[i], results[i], @identifier_list);
        }
    }
;

identifier_list:
    LOCAL_IDENTIFIER { $$.push_back($LOCAL_IDENTIFIER); }
  | identifier_list COMMA LOCAL_IDENTIFIER { $$ = std::move($1); $$.push_back($LOCAL_IDENTIFIER); }
;


instruction:
    HADAMARD atomic var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        yytry(ctx, [&] {
            $$ = hadamard_inst::create($atomic, std::move($alpha), std::move($a), std::move($b),
                                       std::move($beta), std::move($c), @instruction);
        });
    }
;

instruction:
    SUM atomic transpose_opt[ta] var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b] {
        yytry(ctx, [&] {
            $$ = sum_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($beta),
                                  std::move($b), @instruction);
        });
    }
;

instruction:
    YIELD LPAREN optional_value_list[vals] RPAREN {
        yytry(ctx, [&] { $$ = yield_inst::create(std::move($vals), @instruction); });
    }
;

valued_inst:
    ALLOCA optional_dictionary_attribute[dict] COLON memref_type {
        yytry(ctx, [&] {
            $$ = alloca_inst::create(std::move($memref_type), @valued_inst);
            $$->attr($dict);
        });
    }
;

valued_inst: ADD var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = add_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: SUB var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = sub_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: MUL var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = mul_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: DIV var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = div_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: REM var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = rem_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: SHL var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = shl_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: SHR var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = shr_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: AND var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = and_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: OR  var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ =  or_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: XOR var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = xor_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: MIN var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = min_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: MAX var[a] COMMA var[b] COLON data_type[ty] { yytry(ctx, [&] { $$ = max_inst::create($a, $b, $ty, @valued_inst); }); };

valued_inst: ABS  var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ =  abs_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NEG  var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ =  neg_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NOT  var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ =  not_inst::create($a, $ty, @valued_inst); }); };
valued_inst: CONJ var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = conj_inst::create($a, $ty, @valued_inst); }); };
valued_inst: IM   var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ =   im_inst::create($a, $ty, @valued_inst); }); };
valued_inst: RE   var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ =   re_inst::create($a, $ty, @valued_inst); }); };

valued_inst: GROUP_ID COMP3      COLON data_type[ty] { yytry(ctx, [&] { $$ = group_id_inst::create($COMP3, $ty, @valued_inst); }); };
valued_inst: NUM_GROUPS COMP3    COLON data_type[ty] { yytry(ctx, [&] { $$ = num_groups_inst::create($COMP3, $ty, @valued_inst); }); };
valued_inst: NUM_SUBGROUPS COMP3 COLON data_type[ty] { yytry(ctx, [&] { $$ = num_subgroups_inst::create($COMP3, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_SIZE       COLON data_type[ty] { yytry(ctx, [&] { $$ = subgroup_size_inst::create($ty, @valued_inst); }); };
valued_inst: SUBGROUP_ID COMP3   COLON data_type[ty] { yytry(ctx, [&] { $$ = subgroup_id_inst::create($COMP3, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_LINEAR_ID  COLON data_type[ty] { yytry(ctx, [&] { $$ = subgroup_linear_id_inst::create($ty, @valued_inst); }); };
valued_inst: SUBGROUP_LOCAL_ID   COLON data_type[ty] { yytry(ctx, [&] { $$ = subgroup_local_id_inst::create($ty, @valued_inst); }); };

valued_inst:
    CAST var[a] COLON data_type[to] {
        yytry(ctx, [&] { $$ = cast_inst::create(std::move($a), $to, @valued_inst); });
    }
;

valued_inst: EQUAL              var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = equal_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: NOT_EQUAL          var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = not_equal_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: GREATER_THAN       var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = greater_than_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: GREATER_THAN_EQUAL var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = greater_than_equal_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: LESS_THAN          var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = less_than_inst::create($a, $b, $ty, @valued_inst); }); };
valued_inst: LESS_THAN_EQUAL    var[a] COMMA var[b] COLON boolean_type[ty] { yytry(ctx, [&] { $$ = less_than_equal_inst::create($a, $b, $ty, @valued_inst); }); };

valued_inst:
    CONSTANT LSQBR FLOATING_CONSTANT[re] COMMA FLOATING_CONSTANT[im] RSQBR COLON data_type {
        yytry(ctx, [&] {
            $$ = constant_inst::create(std::complex<double>{$re, $im}, $data_type, @valued_inst);
        });
    }
  | CONSTANT FLOATING_CONSTANT COLON data_type {
        yytry(ctx, [&] {
            $$ = constant_inst::create($FLOATING_CONSTANT, $data_type, @valued_inst);
        });
    }
  | CONSTANT INTEGER_CONSTANT COLON data_type {
        yytry(ctx, [&] {
            $$ = constant_inst::create($INTEGER_CONSTANT, $data_type, @valued_inst);
        });
    }
  | CONSTANT BOOLEAN_CONSTANT COLON data_type {
        yytry(ctx, [&] {
            $$ = constant_inst::create($BOOLEAN_CONSTANT, $data_type, @valued_inst);
        });
    }
;

valued_inst:
    COOPERATIVE_MATRIX_APPLY
    LPAREN LOCAL_IDENTIFIER[row] COMMA LOCAL_IDENTIFIER[col] COMMA LOCAL_IDENTIFIER[val] RPAREN
    EQUALS var ARROW data_type[result_ty] <unique_handle<tinytc_inst_t>> {
        yytry(ctx, [&] {
            location loc = @COOPERATIVE_MATRIX_APPLY;
            loc.end = @result_ty.end;
            $$ = cooperative_matrix_apply_inst::create($var, $result_ty, loc);
            auto inode = cooperative_matrix_apply_inst($$.get());
            ctx.push_scope();
            auto &row = inode.row();
            ctx.val($row, row, @row);
            auto &col = inode.col();
            ctx.val($col, col, @col);
            auto &val = inode.val();
            ctx.val($val, val, @val);
            ctx.push_region(&inode.body());
        });
    }[apply_header] region {
        ctx.pop_region();
        ctx.pop_scope();
        $$ = std::move($apply_header);
    }
;

valued_inst:
    COOPERATIVE_MATRIX_EXTRACT var[mat] LSQBR INTEGER_CONSTANT[index] RSQBR COLON data_type[ty]  {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_extract_inst::create($index, std::move($mat), std::move($ty),
                                                         @valued_inst);
        });
    }
;

valued_inst:
    COOPERATIVE_MATRIX_INSERT var[val] COMMA var[mat] LSQBR INTEGER_CONSTANT[index] RSQBR COLON data_type[ty]  {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_insert_inst::create($index, std::move($val), std::move($mat),
                                                        std::move($ty), @valued_inst);
        });
    }
;

valued_inst:
    COOPERATIVE_MATRIX_LOAD transpose_opt[ta] checked var[op] LSQBR var[p0] COMMA var[p1] RSQBR COLON data_type[result_ty]  {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_load_inst::create($ta, $checked, std::move($op), std::move($p0),
                                                      std::move($p1), std::move($result_ty), @valued_inst);
        });
    }
;

checked:
    %empty { $$ = checked_flag::none; }
  | CHECKED { $$ = $CHECKED; }
;

valued_inst:
    COOPERATIVE_MATRIX_MUL_ADD var[a] COMMA var[b] COMMA var[c] COLON data_type[to_ty] {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_mul_add_inst::create(std::move($a), std::move($b), std::move($c),
                                                         std::move($to_ty), @valued_inst);
        });
    }
;

instruction:
    COOPERATIVE_MATRIX_PREFETCH INTEGER_CONSTANT[cache_level] COMMA var[op] LSQBR var[p0] COMMA var[p1] RSQBR COMMA INTEGER_CONSTANT[rows] COMMA INTEGER_CONSTANT[cols] {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_prefetch_inst::create($cache_level, $rows, $cols, std::move($op),
                                                          std::move($p0), std::move($p1), @instruction);
        });
    }
;


valued_inst: COOPERATIVE_MATRIX_REDUCE_ADD REDUCE_MODE var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = cooperative_matrix_reduce_add_inst::create($REDUCE_MODE, $a, $ty, @valued_inst); }); };
valued_inst: COOPERATIVE_MATRIX_REDUCE_MAX REDUCE_MODE var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = cooperative_matrix_reduce_max_inst::create($REDUCE_MODE, $a, $ty, @valued_inst); }); };
valued_inst: COOPERATIVE_MATRIX_REDUCE_MIN REDUCE_MODE var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = cooperative_matrix_reduce_min_inst::create($REDUCE_MODE, $a, $ty, @valued_inst); }); };

valued_inst:
    COOPERATIVE_MATRIX_SCALE var[a] COMMA var[b] COLON data_type[ty] {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_scale_inst::create(std::move($a), std::move($b), std::move($ty),
                                                       @valued_inst);
        });
    }
;

instruction:
    COOPERATIVE_MATRIX_STORE transpose_opt[ta] checked store_flag var[val] COMMA var[op] LSQBR var[p0] COMMA var[p1] RSQBR {
        yytry(ctx, [&] {
            $$ = cooperative_matrix_store_inst::create($ta, $checked, $store_flag, std::move($val),
                                                       std::move($op), std::move($p0), std::move($p1),
                                                       @instruction);
        });
    }
;

valued_inst:
    EXPAND var LSQBR INTEGER_CONSTANT[expanded_mode] ARROW expand_shape RSQBR COLON memref_type[ty] {
        yytry(ctx, [&] {
            auto static_shape = std::vector<std::int64_t>{};
            static_shape.reserve($expand_shape.size());
            auto dynamic_shape = std::vector<tinytc_value_t>{};
            dynamic_shape.reserve($expand_shape.size());
            for (auto &s : $expand_shape) {
                std::visit(overloaded{
                               [&](std::int64_t i) { static_shape.push_back(i); },
                               [&](tinytc_value_t v) {
                                   static_shape.push_back(dynamic);
                                   dynamic_shape.push_back(v);
                               },
                           },
                           s);
            }
            $$ = expand_inst::create($expanded_mode, std::move(static_shape), std::move($var),
                                     std::move(dynamic_shape), $ty, @valued_inst);
        });
    }
;

expand_shape:
    integer_constant_or_identifier[a] TIMES integer_constant_or_identifier[b] {
        $$ = std::vector<int_or_val>{$a, $b};
    }
  | expand_shape TIMES integer_constant_or_identifier[a] { $$ = std::move($1); $$.push_back($a); }
;

integer_constant_or_identifier:
    var {
        $$ = $var;
    }
  | INTEGER_CONSTANT {
        $$ = $INTEGER_CONSTANT;
    }
;

valued_inst:
    FUSE var LSQBR INTEGER_CONSTANT[from] COMMA INTEGER_CONSTANT[to] RSQBR COLON memref_type[ty] {
        yytry(ctx, [&] {
            $$ = fuse_inst::create($from, $to, std::move($var), $ty, @valued_inst);
        });
    }
;

valued_inst:
    LOAD var LSQBR optional_value_list RSQBR COLON data_type {
        yytry(ctx, [&] {
            $$ = load_inst::create(std::move($var), std::move($optional_value_list), std::move($data_type),
                                   @valued_inst);
        });
    }
;

instruction:
    STORE var[a] COMMA var[b] LSQBR optional_value_list RSQBR {
        yytry(ctx, [&] {
            $$ = store_inst::create(std::move($a), std::move($b), std::move($optional_value_list),
                                    @instruction);
        });
    }
;

instruction:
    ATOMIC_STORE memory_scope memory_semantics var[a] COMMA var[b] LSQBR optional_value_list RSQBR {
        yytry(ctx, [&] {
            $$ = atomic_store_inst::create($memory_scope, $memory_semantics, std::move($a), std::move($b),
                                           std::move($optional_value_list), @instruction);
        });
    }
;

memory_scope:
    %empty { $$ = memory_scope::work_group; }
  | MEMORY_SCOPE { $$ = $1; }
;

memory_semantics:
    %empty { $$ = memory_semantics::relaxed; }
  | MEMORY_SEMANTICS { $$ = $1; }
;

valued_inst:
    ATOMIC_ADD memory_scope memory_semantics var[a] COMMA var[b] LSQBR optional_value_list RSQBR COLON scalar_type {
        yytry(ctx, [&] {
            $$ = atomic_add_inst::create($memory_scope, $memory_semantics, std::move($a), std::move($b),
                                         std::move($optional_value_list), $scalar_type, @valued_inst);
        });
    }
;

valued_inst:
    ATOMIC_MAX memory_scope memory_semantics var[a] COMMA var[b] LSQBR optional_value_list RSQBR COLON scalar_type {
        yytry(ctx, [&] {
            $$ = atomic_max_inst::create($memory_scope, $memory_semantics, std::move($a), std::move($b),
                                         std::move($optional_value_list), $scalar_type, @valued_inst);
        });
    }
;

valued_inst:
    ATOMIC_MIN memory_scope memory_semantics var[a] COMMA var[b] LSQBR optional_value_list RSQBR COLON scalar_type {
        yytry(ctx, [&] {
            $$ = atomic_min_inst::create($memory_scope, $memory_semantics, std::move($a), std::move($b),
                                         std::move($optional_value_list), $scalar_type, @valued_inst);
        });
    }
;

store_flag:
    %empty { $$ = store_flag::regular; }
  | ATOMIC { $$ = store_flag::atomic; }
  | ATOMIC_ADD { $$ = store_flag::atomic_add; }
  | ATOMIC_MAX { $$ = store_flag::atomic_max; }
  | ATOMIC_MIN { $$ = store_flag::atomic_min; }
;

instruction: if_inst { $$ = std::move($1); } ;
valued_inst: if_inst { $$ = std::move($1); } ;
if_inst:
    IF var[condition] optional_returned_values <unique_handle<tinytc_inst_t>>{
        yytry(ctx, [&] {
            auto loc = @IF;
            loc.end = @optional_returned_values.end;
            $$ = if_inst::create(std::move($condition), std::move($optional_returned_values), loc);
            auto inode = if_inst($$.get());
            ctx.push_region(&inode.then());
        });
    }[header] region {
        ctx.pop_region();
        auto inode = if_inst($header.get());
        ctx.push_region(&inode.otherwise());
    } else_region {
        ctx.pop_region();
        $$ = std::move($header);
    }
;

else_region:
    %empty {}
  | ELSE region {}
;

optional_returned_values:
    %empty { $$ = {}; }
  | ARROW LPAREN optional_return_type_list[tys] RPAREN { $$ = std::move($tys); }
;

optional_return_type_list:
    %empty {}
  | return_type_list { $$ = std::move($1); }
;

return_type_list:
    data_type { $$.push_back($data_type); }
  | return_type_list COMMA data_type {
        $$ = std::move($1); $$.push_back($data_type);
    }
;

valued_inst: COS var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = cos_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SIN var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = sin_inst::create($a, $ty, @valued_inst); }); };
valued_inst: EXP var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = exp_inst::create($a, $ty, @valued_inst); }); };
valued_inst: EXP2 var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = exp2_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NATIVE_COS var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = native_cos_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NATIVE_SIN var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = native_sin_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NATIVE_EXP var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = native_exp_inst::create($a, $ty, @valued_inst); }); };
valued_inst: NATIVE_EXP2 var[a] COLON data_type[ty] { yytry(ctx, [&] { $$ = native_exp2_inst::create($a, $ty, @valued_inst); }); };

instruction:
    PARALLEL <unique_handle<tinytc_inst_t>>{
        yytry(ctx, [&] {
            $$ = parallel_inst::create(@PARALLEL);
            auto inode = parallel_inst($$.get());
            ctx.push_region(&inode.body());
        });
    }[header] region {
        ctx.pop_region();
        $$ = std::move($header);
    }
;

valued_inst:
    SIZE var LSQBR INTEGER_CONSTANT[mode] RSQBR COLON scalar_type {
        yytry(ctx, [&] {
            $$ = size_inst::create($mode, std::move($var), $scalar_type, @valued_inst);
        });
    }
;

valued_inst:
    SUBGROUP_BROADCAST var[a] COMMA var[idx] COLON scalar_type {
        yytry(ctx, [&] {
            $$ =
                subgroup_broadcast_inst::create(std::move($a), std::move($idx), $scalar_type, @valued_inst);
        });
    }
;

valued_inst: SUBGROUP_EXCLUSIVE_SCAN_ADD var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_exclusive_scan_add_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_EXCLUSIVE_SCAN_MAX var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_exclusive_scan_max_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_EXCLUSIVE_SCAN_MIN var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_exclusive_scan_min_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_INCLUSIVE_SCAN_ADD var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_inclusive_scan_add_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_INCLUSIVE_SCAN_MAX var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_inclusive_scan_max_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_INCLUSIVE_SCAN_MIN var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_inclusive_scan_min_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_REDUCE_ADD var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_reduce_add_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_REDUCE_MAX var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_reduce_max_inst::create($a, $ty, @valued_inst); }); };
valued_inst: SUBGROUP_REDUCE_MIN var[a] COLON scalar_type[ty] { yytry(ctx, [&] { $$ = subgroup_reduce_min_inst::create($a, $ty, @valued_inst); }); };

valued_inst:
    SUBVIEW var LSQBR optional_slice_list RSQBR COLON memref_type[ty] {
        yytry(ctx, [&] {
            auto static_offsets = std::vector<std::int64_t>{};
            auto static_sizes = std::vector<std::int64_t>{};
            auto offsets = std::vector<tinytc_value_t>{};
            auto sizes = std::vector<tinytc_value_t>{};
            static_offsets.reserve($optional_slice_list.size());
            static_sizes.reserve($optional_slice_list.size());
            offsets.reserve($optional_slice_list.size());
            sizes.reserve($optional_slice_list.size());
            for (auto &s : $optional_slice_list) {
                std::visit(overloaded{
                               [&](std::int64_t i) { static_offsets.push_back(i); },
                               [&](tinytc_value_t v) {
                                   static_offsets.push_back(dynamic);
                                   offsets.push_back(v);
                               },
                           },
                           s.first);
                std::visit(overloaded{
                               [&](std::int64_t i) { static_sizes.push_back(i); },
                               [&](tinytc_value_t v) {
                                   static_sizes.push_back(dynamic);
                                   sizes.push_back(v);
                               },
                           },
                           s.second);
            }
            $$ = subview_inst::create(std::move(static_offsets), std::move(static_sizes), std::move($var),
                                      std::move(offsets), std::move(sizes), std::move($ty), @valued_inst);
        });
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
    if (m.size() > 0 || l.begin.line > 0) {
        ctx.report_error(l, m);
    }
}
}
