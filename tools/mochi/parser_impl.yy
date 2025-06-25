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
    COLLECTIVE      "collective"
    CXX             "cxx"
    ENUM            "enum"
    INCLUDE         "include"
    INST            "inst"
    MIXED           "mixed"
    OP              "op"
    PRIVATE         "private"
    PROP            "prop"
    REG             "reg"
    RET             "ret"
    SKIP_BUILDER    "skip_builder"
    SPMD            "spmd"
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
%token <inst_flag> INST_FLAG

%nterm <std::pair<std::uint32_t, std::vector<case_>>> enum_content
%nterm <case_> case
%nterm <std::pair<std::uint32_t, std::vector<member>>> members
%nterm <member> member
%nterm <quantifier> op_quantifier
%nterm <quantifier> nonop_quantifier
%nterm <data_type> data_type
%nterm <inst*> parent
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
    INST GLOBAL_ID parent optstring LBRACE members RBRACE {
        try {
            auto i = std::make_unique<inst>($GLOBAL_ID, std::move($optstring),
                                            std::move($members.second), $parent);
            i->flags($members.first);
            obj.add($parent, std::move(i));
        } catch (std::exception const &e) {
            error(@inst, e.what());
            YYERROR;
        }
    }
;

parent:
    %empty { $$ = nullptr; }
  | COLON GLOBAL_ID {
        auto parent = obj.find_inst($GLOBAL_ID);
        if (!parent) {
            error(@GLOBAL_ID, "Could not find parent class definition");
            YYERROR;
        }
        $$ = parent;
    }
;

members:
    %empty {}
  | members INST_FLAG { $$ = std::move($1); $$.first |= static_cast<std::uint32_t>($INST_FLAG); }
  | members member { $$ = std::move($1); $$.second.emplace_back(std::move($member)); }
;

member:
    OP op_quantifier LOCAL_ID optstring {
        $$ = op{$op_quantifier, std::move($LOCAL_ID), std::move($optstring)};
    }
  | PROP nonop_quantifier LOCAL_ID private ARROW data_type optstring {
        $$ = prop{$nonop_quantifier, std::move($LOCAL_ID), std::move($optstring),
                  std::move($data_type), $private};
    }
  | RET nonop_quantifier LOCAL_ID optstring {
        $$ = ret{$nonop_quantifier, std::move($LOCAL_ID), std::move($optstring)};
    }
  | REG LOCAL_ID optstring { $$ = reg{std::move($LOCAL_ID), std::move($optstring)}; }
  | CXX string { $$ = raw_cxx{std::move($string)}; }
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

data_type:
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

