// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "node/inst_view.hpp"
    #include "tinytc/builder.hpp"
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
            attr dict;
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
    #include "node/prog.hpp"
    #include "node/region.hpp"
    #include "node/value.hpp"
    #include "parser/lexer.hpp"
    #include "parser/parse_context.hpp"
    #include "tinytc/tinytc.hpp"
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
    void report_error(compiler_context const& cctx, compilation_error const& e) {
        if (e.extra_info().size() > 0) {
            auto what = (std::ostringstream{} << e.what() << " (" << e.extra_info() << ')').str();
            cctx.get()->report_error(e.loc(), e.ref_values(), what.c_str());
        } else {
            cctx.get()->report_error(e.loc(), e.ref_values(), e.what());
        }
    }
    template <typename F>
    void yytry(parse_context& ctx, F&& f) {
        try {
            f();
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            throw parser::syntax_error({}, "");
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
    NOTRANS         ".n"
    TRANS           ".t"
    ATOMIC          ".atomic"
    ATOMIC_ADD      ".atomic_add"
    ATOMIC_MAX      ".atomic_max"
    ATOMIC_MIN      ".atomic_min"
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
    ALLOCA                      "alloca"
    BARRIER                     "barrier"
    CAST                        "cast"
    CONSTANT                    "constant"
    COOPERATIVE_MATRIX_APPLY    "cooperative_matrix_apply"
    COOPERATIVE_MATRIX_EXTRACT  "cooperative_matrix_extract"
    COOPERATIVE_MATRIX_INSERT   "cooperative_matrix_insert"
    COOPERATIVE_MATRIX_LOAD     "cooperative_matrix_load"
    COOPERATIVE_MATRIX_MUL_ADD  "cooperative_matrix_mul_add"
    COOPERATIVE_MATRIX_PREFETCH "cooperative_matrix_prefetch"
    COOPERATIVE_MATRIX_SCALE    "cooperative_matrix_scale"
    COOPERATIVE_MATRIX_STORE    "cooperative_matrix_store"
    EXPAND                      "expand"
    FUSE                        "fuse"
    LOAD                        "load"
    IF                          "if"
    ELSE                        "else"
    PARALLEL                    "parallel"
    SIZE                        "size"
    SUBGROUP_BROADCAST          "subgroup_broadcast"
    SUBVIEW                     "subview"
    STORE                       "store"
    YIELD                       "yield"
    ADD                         "add"
    SUB                         "sub"
    MUL                         "mul"
    DIV                         "div"
    REM                         "rem"
    SHL                         "shl"
    SHR                         "shr"
    AND                         "and"
    OR                          "or"
    XOR                         "xor"
    MIN                         "min"
    MAX                         "max"
    AXPBY                       "axpby"
    CUMSUM                      "cumsum"
    SUM                         "sum"
    GEMM                        "gemm"
    GEMV                        "gemv"
    GER                         "ger"
    HADAMARD                    "hadamard"
    FOR                         "for"
    FOREACH                     "foreach"
;
%token <identifier> LOCAL_IDENTIFIER
%token <std::string> GLOBAL_IDENTIFIER
%token <std::string> ATTR_NAME
%token <std::string> STRING
%token <bool> BOOLEAN_CONSTANT
%token <std::int64_t> INTEGER_CONSTANT
%token <double> FLOATING_CONSTANT
%token <scalar_type> INTEGER_TYPE
%token <scalar_type> FLOATING_TYPE
%token <arithmetic_unary> ARITHMETIC_UNARY
%token <builtin> BUILTIN
%token <cmp_condition> CMP_CONDITION
%token <std::pair<group_arithmetic,reduce_mode>> COOPERATIVE_MATRIX_REDUCE
%token <std::pair<group_arithmetic,group_operation>> SUBGROUP_OPERATION
%token <math_unary> MATH_UNARY
%token <matrix_use> MATRIX_USE
%token <checked_flag> CHECKED

%nterm <prog> prog
%nterm <std::vector<func>> func_list
%nterm <func> func
%nterm <std::pair<std::vector<param_attrs>,std::vector<tinytc_data_type_t>>> parameters
%nterm <std::pair<param_attrs,tinytc_data_type_t>> parameter
%nterm <tinytc_attr_t> function_attributes
%nterm <tinytc_attr_t> attribute
%nterm <tinytc_attr_t> array_attribute
%nterm <std::vector<tinytc_attr_t>> attribute_list
%nterm <tinytc_attr_t> dictionary_attribute
%nterm <std::vector<named_attr>> named_attribute_list
%nterm <named_attr> named_attribute
%nterm <tinytc_attr_t> attribute_name
%nterm <tinytc_attr_t> optional_dictionary_attribute
%nterm <tinytc_data_type_t> data_type
%nterm <tinytc_data_type_t> boolean_type
%nterm <tinytc_data_type_t> scalar_type
%nterm <tinytc_data_type_t> coopmatrix_type
%nterm <tinytc_data_type_t> memref_type
%nterm <address_space> optional_address_space
%nterm <std::vector<std::int64_t>> mode_list
%nterm <std::vector<std::int64_t>> optional_stride_list
%nterm <std::vector<std::int64_t>> stride_list
%nterm <std::int64_t> constant_or_dynamic
%nterm <tinytc_data_type_t> group_type
%nterm <std::int64_t> group_offset
%nterm <tinytc_value_t> var
%nterm <inst> instruction
%nterm <bool> atomic
%nterm <std::vector<tinytc_value_t>> optional_value_list
%nterm <std::vector<tinytc_value_t>> value_list
%nterm <std::int32_t> optional_global_attr
%nterm <std::int32_t> optional_local_attr
%nterm <transpose> transpose
%nterm <inst> for_inst
%nterm <std::tuple<std::vector<identifier>, std::vector<tinytc_value_t>, std::vector<tinytc_data_type_t>>> optional_loop_carried_values
%nterm <std::pair<std::vector<identifier>, std::vector<tinytc_value_t>>> init_value_list
%nterm <std::pair<identifier, tinytc_value_t>> init_value
%nterm <tinytc_value_t> optional_step
%nterm <inst> if_inst
%nterm <std::vector<tinytc_data_type_t>> optional_returned_values
%nterm <std::vector<tinytc_data_type_t>> optional_return_type_list
%nterm <std::vector<tinytc_data_type_t>> return_type_list
%nterm <std::vector<identifier>> identifier_list
%nterm <inst> valued_inst
%nterm <checked_flag> checked
%nterm <int_or_val> integer_constant_or_identifier
%nterm <std::vector<int_or_val>> expand_shape
%nterm <store_flag> store_flag
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> optional_slice_list
%nterm <std::vector<std::pair<int_or_val,int_or_val>>> slice_list
%nterm <std::pair<int_or_val,int_or_val>> slice
%nterm <int_or_val> slice_size

%%
prog:
    func_list {
        auto p = prog { std::make_unique<tinytc_prog>(ctx.cctx(), @prog).release() };
        ctx.program(p);
        $$ = std::move(p);
        for (auto& f : $func_list) {
            add_function($$, std::move(f));
        }
    }
;

func_list:
    func { $$.emplace_back(std::move($func)); }
  | func_list func { $$ = std::move($1); $$.emplace_back(std::move($func)); }

func:
    FUNC GLOBAL_IDENTIFIER LPAREN parameters RPAREN function_attributes <func>{
        auto loc = @FUNC;
        loc.end = @RPAREN.end;
        try {
            ctx.add_global_name($GLOBAL_IDENTIFIER, loc);
            auto func_node = std::make_unique<tinytc_func>($GLOBAL_IDENTIFIER, $parameters.second,
                                                           get_void(ctx.cctx()), loc);
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
            $$ = func{func_node.release()};
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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
  | BOOLEAN_CONSTANT { $$ = boolean_attr::get(ctx.cctx().get(), $BOOLEAN_CONSTANT); }
  | dictionary_attribute { $$ = $dictionary_attribute; }
  | INTEGER_CONSTANT { $$ = integer_attr::get(ctx.cctx().get(), $INTEGER_CONSTANT); }
  | STRING { $$ = string_attr::get(ctx.cctx().get(), $STRING); }
;

array_attribute:
    LSQBR RSQBR { $$ = array_attr::get(ctx.cctx().get(), {}); }
  | LSQBR attribute_list RSQBR { $$ = array_attr::get(ctx.cctx().get(), $attribute_list); }

attribute_list:
    attribute { $$.push_back($attribute); }
  | attribute_list COMMA attribute { $$ = std::move($1); $$.push_back($attribute); }
;

dictionary_attribute:
    LBRACE RBRACE { $$ = dictionary_attr::get(ctx.cctx().get(), {}); }
  | LBRACE named_attribute_list RBRACE {
        dictionary_attr::sort($named_attribute_list);
        $$ = dictionary_attr::get(ctx.cctx().get(), $named_attribute_list);
    }
;

named_attribute_list:
    named_attribute { $$.push_back($named_attribute); }
  | named_attribute_list COMMA named_attribute { $$ = std::move($1); $$.push_back($named_attribute); }
;

named_attribute:
    attribute_name EQUALS attribute {
        $$ = named_attr{$attribute_name, $attribute};
    }
;

attribute_name:
    ATTR_NAME { $$ = string_attr::get(ctx.cctx().get(), $ATTR_NAME); }
  | STRING { $$ = string_attr::get(ctx.cctx().get(), $STRING); }

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
    BOOLEAN { $$ = get_boolean(ctx.cctx()); }
;

scalar_type:
    INTEGER_TYPE  { $$ = get_scalar(ctx.cctx(), $INTEGER_TYPE); }
  | FLOATING_TYPE { $$ = get_scalar(ctx.cctx(), $FLOATING_TYPE); }
;

coopmatrix_type:
    COOPMATRIX LCHEV scalar_type TIMES INTEGER_CONSTANT[rows] TIMES INTEGER_CONSTANT[cols] COMMA MATRIX_USE RCHEV {
        try {
            $$ = get_coopmatrix($scalar_type, $rows, $cols, $MATRIX_USE, @coopmatrix_type);
        } catch (compilation_error const& e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

memref_type:
    MEMREF LCHEV scalar_type mode_list optional_address_space RCHEV {
        try {
            $$ = get_memref($scalar_type, $mode_list, {}, $optional_address_space, @memref_type);
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
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
            $$ = get_memref($scalar_type, $mode_list, $optional_stride_list,
                            $optional_address_space, @memref_type);
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
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
    GROUP LCHEV memref_type TIMES constant_or_dynamic[group_size] group_offset RCHEV {
        $$ = get_group(std::move($memref_type), $group_size, $group_offset, @group_type);
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
    AXPBY transpose[ta] atomic var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b] {
        try {
            $$ = inst {
                axpby_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($beta),
                                   std::move($b), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
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

instruction:
    BARRIER optional_global_attr optional_local_attr {
        int32_t fence_flags = 0;
        fence_flags |= $optional_global_attr;
        fence_flags |= $optional_local_attr;
        try {
            $$ = inst { barrier_inst::create(fence_flags, @instruction) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
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

instruction:
    CUMSUM atomic var[alpha] COMMA var[a] COMMA INTEGER_CONSTANT[mode] COMMA var[beta] COMMA var[b] {
        try {
            $$ = inst {
                cumsum_inst::create($atomic, $mode, std::move($alpha), std::move($a), std::move($beta),
                                    std::move($b), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    GEMM transpose[ta] transpose[tb] atomic var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        try {
            $$ = inst {
                gemm_inst::create($atomic, $ta, $tb, std::move($alpha), std::move($a), std::move($b),
                                  std::move($beta), std::move($c), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    GEMV transpose[ta] atomic var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        try {
            $$ = inst {
                gemv_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($b),
                                  std::move($beta), std::move($c), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

transpose:
    NOTRANS { $$ = transpose::N; }
  | TRANS { $$ = transpose::T; }
;

instruction:
    GER atomic var[alpha] COMMA var[a] COMMA var[b] COMMA var[beta] COMMA var[c] {
        try {
            $$ = inst {
                ger_inst::create($atomic, std::move($alpha), std::move($a), std::move($b), std::move($beta),
                                 std::move($c), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction: for_inst { $$ = std::move($1); } ;
valued_inst: for_inst { $$ = std::move($1); } ;
for_inst:
    FOR LOCAL_IDENTIFIER[loop_var] EQUALS var[from] COMMA var[to] optional_step optional_loop_carried_values[lcv] <inst> {
        try {
            auto &[lcv_id, lcv_init, lcv_type] = $lcv;
            location loc = @FOR;
            loc.end = @lcv.end;
            $$ = inst{for_inst::create($from, $to, $optional_step, lcv_init, lcv_type, loc)};
            auto inode = for_inst($$.get());
            ctx.push_scope();
            auto &loop_var = inode.loop_var();
            ctx.val($loop_var, loop_var, @loop_var);
            for (std::int32_t i = 0; i < inode.get().num_results(); ++i) {
                ctx.val(lcv_id[i], inode.iter_arg(i), @lcv);
            }
            ctx.push_region(&inode.body());
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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
            LPAREN value_list[from] RPAREN COMMA LPAREN value_list[to] RPAREN[header_end] <inst>{
        try {
            location loc = @FOREACH;
            loc.end = @header_end.end;
            $$ = inst{foreach_inst::create($from, $to, loc)};
            auto inode = foreach_inst($$.get());
            ctx.push_scope();
            auto loop_vars = inode.loop_vars().begin();
            for (std::int64_t i = 0; i < inode.dim(); ++i) {
                ctx.val($loop_var[i], loop_vars[i], @loop_var);
            }
            ctx.push_region(&inode.body());
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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
        try {
            $$ = inst {
                hadamard_inst::create($atomic, std::move($alpha), std::move($a), std::move($b),
                                      std::move($beta), std::move($c), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    SUM transpose[ta] atomic var[alpha] COMMA var[a] COMMA var[beta] COMMA var[b] {
        try {
            $$ = inst {
                sum_inst::create($atomic, $ta, std::move($alpha), std::move($a), std::move($beta),
                                 std::move($b), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    YIELD LPAREN optional_value_list[vals] RPAREN {
        try {
            $$ = inst { yield_inst::create(std::move($vals), @instruction) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    ALLOCA optional_dictionary_attribute[dict] COLON memref_type {
        try {
            $$ = inst { alloca_inst::create(std::move($memref_type), @valued_inst) };
            $$->attr($dict);
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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

valued_inst:
    ARITHMETIC_UNARY var[a] COLON data_type[ty] {
        try {
            $$ = inst {
                arith_unary_inst::create($ARITHMETIC_UNARY, std::move($a), std::move($ty),
                                         @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    BUILTIN COLON data_type[ty] {
        try {
            $$ = inst { builtin_inst::create($BUILTIN, $ty, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    CAST var[a] COLON data_type[to] {
        try {
            $$ = inst { cast_inst::create(std::move($a), $to, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    CMP_CONDITION var[a] COMMA var[b] COLON boolean_type {
        try {
            $$ = inst {
                compare_inst::create($CMP_CONDITION, std::move($a), std::move($b), std::move($boolean_type),
                                     @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    CONSTANT LSQBR FLOATING_CONSTANT[re] COMMA FLOATING_CONSTANT[im] RSQBR COLON data_type {
        try {
            $$ = inst { constant_inst::create(std::complex<double>{$re, $im}, $data_type, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
  | CONSTANT FLOATING_CONSTANT COLON data_type {
        try {
            $$ = inst { constant_inst::create($FLOATING_CONSTANT, $data_type, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
  | CONSTANT INTEGER_CONSTANT COLON data_type {
        try {
            $$ = inst { constant_inst::create($INTEGER_CONSTANT, $data_type, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
  | CONSTANT BOOLEAN_CONSTANT COLON data_type {
        try {
            $$ = inst { constant_inst::create($BOOLEAN_CONSTANT, $data_type, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    COOPERATIVE_MATRIX_APPLY
    LPAREN LOCAL_IDENTIFIER[row] COMMA LOCAL_IDENTIFIER[col] COMMA LOCAL_IDENTIFIER[val] RPAREN
    EQUALS var ARROW data_type[result_ty] <inst> {
        try {
            location loc = @COOPERATIVE_MATRIX_APPLY;
            loc.end = @result_ty.end;
            $$ = inst{cooperative_matrix_apply_inst::create($var, $result_ty, loc)};
            auto inode = cooperative_matrix_apply_inst($$.get());
            ctx.push_scope();
            auto &row = inode.row();
            ctx.val($row, row, @row);
            auto &col = inode.col();
            ctx.val($col, col, @col);
            auto &val = inode.val();
            ctx.val($val, val, @val);
            ctx.push_region(&inode.body());
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }[apply_header] region {
        ctx.pop_region();
        ctx.pop_scope();
        $$ = std::move($apply_header);
    }
;

valued_inst:
    COOPERATIVE_MATRIX_EXTRACT var[mat] LSQBR INTEGER_CONSTANT[index] RSQBR COLON data_type[ty]  {
        try {
            $$ = inst {
                cooperative_matrix_extract_inst::create($index, std::move($mat), std::move($ty),
                                                        @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    COOPERATIVE_MATRIX_INSERT var[val] COMMA var[mat] LSQBR INTEGER_CONSTANT[index] RSQBR COLON data_type[ty]  {
        try {
            $$ = inst {
                cooperative_matrix_insert_inst::create($index, std::move($val), std::move($mat),
                                                       std::move($ty), @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    COOPERATIVE_MATRIX_LOAD transpose checked var[op] LSQBR var[p0] COMMA var[p1] RSQBR COLON data_type[result_ty]  {
        try {
            $$ = inst {
                cooperative_matrix_load_inst::create($transpose, $checked, std::move($op), std::move($p0),
                                                     std::move($p1), std::move($result_ty),
                                                     @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

checked:
    %empty { $$ = checked_flag::none; }
  | CHECKED { $$ = $CHECKED; }
;

valued_inst:
    COOPERATIVE_MATRIX_MUL_ADD var[a] COMMA var[b] COMMA var[c] COLON data_type[to_ty] {
        try {
            $$ = inst {
                cooperative_matrix_mul_add_inst::create(std::move($a), std::move($b), std::move($c),
                                                        std::move($to_ty), @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    COOPERATIVE_MATRIX_PREFETCH INTEGER_CONSTANT[cache_level] COMMA var[op] LSQBR var[p0] COMMA var[p1] RSQBR COMMA INTEGER_CONSTANT[rows] COMMA INTEGER_CONSTANT[cols] {
        try {
            $$ = inst {
                cooperative_matrix_prefetch_inst::create($cache_level, $rows, $cols, std::move($op),
                                                         std::move($p0), std::move($p1),
                                                         @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    COOPERATIVE_MATRIX_REDUCE var[a] COLON data_type[ty] {
        try {
            $$ = inst {
                cooperative_matrix_reduce_inst::create($COOPERATIVE_MATRIX_REDUCE.first,
                                                       $COOPERATIVE_MATRIX_REDUCE.second, std::move($a),
                                                       std::move($ty), @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    COOPERATIVE_MATRIX_SCALE var[a] COMMA var[b] COLON data_type[ty] {
        try {
            $$ = inst {
                cooperative_matrix_scale_inst::create(std::move($a), std::move($b), std::move($ty),
                                                      @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    COOPERATIVE_MATRIX_STORE checked store_flag var[val] COMMA var[op] LSQBR var[p0] COMMA var[p1] RSQBR {
        try {
            $$ = inst {
                cooperative_matrix_store_inst::create($checked, $store_flag, std::move($val),
                                                      std::move($op), std::move($p0), std::move($p1),
                                                      @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    EXPAND var LSQBR INTEGER_CONSTANT[expanded_mode] ARROW expand_shape RSQBR COLON memref_type[ty] {
        try {
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
            $$ = inst {
                expand_inst::create($expanded_mode, std::move(static_shape), std::move($var),
                                    std::move(dynamic_shape), $ty, @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        } catch (std::exception const &e) {
            error(@valued_inst, e.what());
        }
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
        try {
            $$ = inst { fuse_inst::create($from, $to, std::move($var), $ty, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    LOAD var LSQBR optional_value_list RSQBR COLON data_type {
        try {
            $$ = inst {
                load_inst::create(std::move($var), std::move($optional_value_list), std::move($data_type),
                                  @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    STORE store_flag var[a] COMMA var[b] LSQBR optional_value_list RSQBR {
        try {
            $$ = inst {
                store_inst::create($store_flag, std::move($a), std::move($b),
                                   std::move($optional_value_list), @instruction)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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
    IF var[condition] optional_returned_values <inst>{
        try {
            auto loc = @IF;
            loc.end = @optional_returned_values.end;
            $$ = inst{if_inst::create(std::move($condition), std::move($optional_returned_values), loc)};
            auto inode = if_inst($$.get());
            ctx.push_region(&inode.then());
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
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

valued_inst:
    MATH_UNARY var[a] COLON data_type[ty] {
        try {
            $$ = inst {
                math_unary_inst::create($MATH_UNARY, std::move($a), std::move($ty), @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

instruction:
    PARALLEL <inst>{
        try {
            $$ = inst { parallel_inst::create(@PARALLEL) };
            auto inode = parallel_inst($$.get());
            ctx.push_region(&inode.body());
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }[header] region {
        ctx.pop_region();
        $$ = std::move($header);
    }
;

valued_inst:
    SIZE var LSQBR INTEGER_CONSTANT[mode] RSQBR COLON scalar_type {
        try {
            $$ = inst { size_inst::create($mode, std::move($var), $scalar_type, @valued_inst) };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    SUBGROUP_BROADCAST var[a] COMMA var[idx] COLON scalar_type {
        try {
            $$ = inst {
                subgroup_broadcast_inst::create(std::move($a), std::move($idx), $scalar_type,
                                                @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    SUBGROUP_OPERATION var[a] COLON scalar_type {
        try {
            $$ = inst {
                subgroup_operation_inst::create($SUBGROUP_OPERATION.first, $SUBGROUP_OPERATION.second,
                                                std::move($a), $scalar_type, @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        }
    }
;

valued_inst:
    SUBVIEW var LSQBR optional_slice_list RSQBR COLON memref_type[ty] {
        try {
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
            $$ = inst {
                subview_inst::create(std::move(static_offsets), std::move(static_sizes), std::move($var),
                                     std::move(offsets), std::move(sizes), std::move($ty), @valued_inst)
            };
        } catch (compilation_error const &e) {
            report_error(ctx.cctx(), e);
            YYERROR;
        } catch (std::exception const &e) {
            error(@valued_inst, e.what());
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
    if (m.size() > 0 || l.begin.line > 0) {
        ctx.report_error(l, m);
    }
}
}
