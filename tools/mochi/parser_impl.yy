// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "object.hpp"

    #include <cstddef>
    #include <cstdint>
    #include <new>
    #include <variant>
    #include <utility>

    namespace mochi {
        class lexer;
        class objects;
    }
}

%code {
    #include "lexer.hpp"
    #include "objects.hpp"
    #include "parser.hpp"

    #include <memory>
    #include <optional>
    #include <utility>
}

%header
%parse-param { lexer& lex }
%parse-param { objects& obj }
%parse-param { std::vector<char const*> const& search_paths }
%lex-param { lexer& lex }
%locations

%define parse.assert
%define parse.error detailed
%define parse.lac full
%define api.namespace {mochi}
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {TOK_}
%token
    CASE            "case"
    CXX             "cxx"
    ENUM            "enum"
    INCLUDE         "include"
    INST            "inst"
    OP              "op"
    PRIVATE         "private"
    PROP            "prop"
    REG             "reg"
    RET             "ret"
    TYPE            "type"
    SKIP_BUILDER    "skip_builder"
    ARROW           "=>"
    COLON           ":"
    LBRACE          "{"
    RBRACE          "}"
    QUESTION        "?"
    STAR            "*"
;
%token <std::string> GLOBAL_ID
%token <std::string> LOCAL_ID
%token <std::string> STRING
%token <std::int64_t> NUMBER
%token <builtin_type> BUILTIN_TYPE
%token <enum_flag> ENUM_FLAG
%token <std::uint32_t> INST_FLAG

%nterm <std::pair<std::uint32_t, std::vector<case_>>> enum_content
%nterm <case_> case
%nterm <std::pair<std::uint32_t, std::vector<inst_member>>> inst_members
%nterm <inst_member> inst_member
%nterm <std::pair<std::uint32_t, std::vector<type_member>>> type_members
%nterm <type_member> type_member
%nterm <prop> prop
%nterm <raw_cxx> cxx
%nterm <quantifier> op_quantifier
%nterm <quantifier> nonop_quantifier
%nterm <cxx_type> cxx_type
%nterm <inst*> parent_inst
%nterm <type*> parent_type
%nterm <bool> private
%nterm <std::string> optstring
%nterm <std::string> string

%%
stmt_list:
    stmt {}
  | stmt_list stmt {}

stmt:
    enum {}
  | inst {}
  | type {}
  | INCLUDE STRING {
        try {
            auto included_obj = parse_file($STRING, search_paths);
            if (!included_obj) {
                error(@STRING, "Could not find parse included file");
                YYERROR;
            }
            obj.add(std::move(*included_obj));
        } catch (std::exception const& e) {
            error(@STRING, e.what());
            YYERROR;
        }
    }
;

enum:
    ENUM GLOBAL_ID optstring LBRACE enum_content RBRACE {
        try {
            auto e =
                std::make_unique<enum_>($GLOBAL_ID, std::move($optstring),
                                        std::move($enum_content.second));
            e->flags($enum_content.first);
            obj.add(std::move(e));
        } catch (std::exception const &e) {
            error(@enum, e.what());
            YYERROR;
        }
    }
;

enum_content:
    %empty {}
  | enum_content ENUM_FLAG { $$ = std::move($1); $$.first |= static_cast<std::uint32_t>($ENUM_FLAG); }
  | enum_content case { $$ = std::move($1); $$.second.emplace_back(std::move($case)); }
;

case:
    CASE LOCAL_ID ARROW NUMBER optstring {
        $$ = case_{std::move($LOCAL_ID), std::move($optstring), $NUMBER};
    }
;

inst:
    INST GLOBAL_ID parent_inst optstring LBRACE inst_members RBRACE {
        try {
            auto i = std::make_unique<inst>($GLOBAL_ID, std::move($optstring),
                                            std::move($inst_members.second), $parent_inst);
            i->flags($inst_members.first);
            obj.add($parent_inst, std::move(i));
        } catch (std::exception const &e) {
            error(@inst, e.what());
            YYERROR;
        }
    }
;

parent_inst:
    %empty { $$ = nullptr; }
  | COLON GLOBAL_ID {
        auto parent_inst = obj.find_inst($GLOBAL_ID);
        if (!parent_inst) {
            error(@GLOBAL_ID, "Could not find parent class definition");
            YYERROR;
        }
        $$ = parent_inst;
    }
;

inst_members:
    %empty {}
  | inst_members INST_FLAG { $$ = std::move($1); $$.first |= $INST_FLAG; }
  | inst_members inst_member { $$ = std::move($1); $$.second.emplace_back(std::move($inst_member)); }
;

inst_member:
    OP op_quantifier LOCAL_ID optstring {
        $$ = op{$op_quantifier, std::move($LOCAL_ID), std::move($optstring)};
    }
  | prop { $$ = std::move($prop); }
  | RET nonop_quantifier LOCAL_ID optstring {
        $$ = ret{$nonop_quantifier, std::move($LOCAL_ID), std::move($optstring)};
    }
  | REG LOCAL_ID optstring { $$ = reg{std::move($LOCAL_ID), std::move($optstring)}; }
  | cxx { $$ = std::move($cxx); }
;

type:
    TYPE GLOBAL_ID parent_type optstring LBRACE type_members RBRACE {
        try {
            auto t = std::make_unique<type>($GLOBAL_ID, std::move($optstring),
                                            std::move($type_members.second), $parent_type);
            t->flags($type_members.first);
            obj.add($parent_type, std::move(t));
        } catch (std::exception const &e) {
            error(@type, e.what());
            YYERROR;
        }
    }
;

parent_type:
    %empty { $$ = nullptr; }
  | COLON GLOBAL_ID {
        auto parent_type = obj.find_type($GLOBAL_ID);
        if (!parent_type) {
            error(@GLOBAL_ID, "Could not find parent class definition");
            YYERROR;
        }
        $$ = parent_type;
    }
;

type_members:
    %empty {}
  | type_members ENUM_FLAG { $$ = std::move($1); $$.first |= static_cast<std::uint32_t>($ENUM_FLAG); }
  | type_members type_member { $$ = std::move($1); $$.second.emplace_back(std::move($type_member)); }
;

type_member:
    prop { $$ = std::move($prop); }
  | cxx { $$ = std::move($cxx); }
;

prop:
    PROP nonop_quantifier LOCAL_ID private ARROW cxx_type optstring {
        $$ = prop{$nonop_quantifier, std::move($LOCAL_ID), std::move($optstring),
                  std::move($cxx_type), $private};
    }
;

cxx:
   CXX string { $$ = raw_cxx{std::move($string)}; }
;

op_quantifier:
    %empty   { $$ = quantifier::single; }
  | QUESTION { $$ = quantifier::optional; }
  | STAR     { $$ = quantifier::many; }
;

nonop_quantifier:
    %empty   { $$ = quantifier::single; }
  | STAR     { $$ = quantifier::many; }
;

cxx_type:
    BUILTIN_TYPE { $$ = std::move($1); }
  | STRING     { $$ = std::move($1); }
  | GLOBAL_ID  {
        auto ty = obj.find_enum($GLOBAL_ID);
        if (!ty) {
            error(@GLOBAL_ID, "Could not find enum definition");
            YYERROR;
        }
        $$ = std::move(ty);
    }
;

private:
    %empty { $$ = false; }
  | PRIVATE { $$ = true; }
;

optstring:
    %empty { $$ = ""; }
  | string { $$ = std::move($1); }
;

string:
    STRING { $$ = std::move($1); }
  | string STRING { $$ = std::move($1) + std::move($2); }
;

%%

namespace mochi {
void parser::error(location_type const& l, std::string const& m) {
    std::cerr << l << ": " << m << std::endl;
}
}

