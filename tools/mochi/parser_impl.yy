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

    #include <memory>
    #include <utility>
}

%header
%parse-param { lexer& lex }
%parse-param { objects& obj }
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
    DOC_TO_STRING   "doc_to_string"
    ENUM            "enum"
    INST            "inst"
    MIXED           "mixed"
    OP              "op"
    PRIVATE         "private"
    PROP            "prop"
    REG             "reg"
    RET             "ret"
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

%nterm <std::vector<case_>> cases
%nterm <case_> case
%nterm <std::vector<member>> members
%nterm <member> member
%nterm <quantifier> op_quantifier
%nterm <quantifier> nonop_quantifier
%nterm <inst*> parent
%nterm <bool> private
%nterm <bool> doc_to_string
%nterm <std::string> optstring
%nterm <std::string> string

%%
stmt_list:
    stmt {}
  | stmt_list stmt {}

stmt:
    enum {}
  | inst {}
;

enum:
    ENUM GLOBAL_ID doc_to_string optstring LBRACE cases RBRACE {
        try {
            obj.add(std::make_unique<enum_>($GLOBAL_ID, std::move($optstring), std::move($cases), $doc_to_string));
        } catch (std::exception const& e) {
            error(@enum, e.what());
            YYERROR;
        }
    }
;

cases:
    %empty {}
  | cases case { $$ = std::move($1); $$.emplace_back(std::move($case)); }
;

case:
    CASE LOCAL_ID ARROW NUMBER optstring {
        $$ = case_{std::move($LOCAL_ID), std::move($optstring), $NUMBER};
    }
;

inst:
    INST GLOBAL_ID parent LBRACE members RBRACE {
        try {
            obj.add($parent, std::make_unique<inst>($GLOBAL_ID, std::move($members), $parent));
        } catch (std::exception const& e) {
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
  | members member { $$ = std::move($1); $$.emplace_back(std::move($member)); }
;

member:
    OP op_quantifier LOCAL_ID optstring {
        $$ = op{$op_quantifier, std::move($LOCAL_ID), std::move($optstring)};
    }
  | PROP nonop_quantifier LOCAL_ID private ARROW STRING optstring {
        $$ = prop{$nonop_quantifier, std::move($LOCAL_ID), std::move($STRING), std::move($optstring),
                  $private};
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

private:
    %empty { $$ = false; }
  | PRIVATE { $$ = true; }
;

doc_to_string:
    %empty { $$ = false; }
  | DOC_TO_STRING { $$ = true; }
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

