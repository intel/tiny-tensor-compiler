// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

%require "3.8.2"
%language "c++"

%code requires {
    #include "inst.hpp"

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
    COLLECTIVE      "collective"
    CXX             "cxx"
    ENUM            "enum"
    INST            "inst"
    MIXED           "mixed"
    OP              "op"
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

%nterm <std::vector<member>> members
%nterm <member> member
%nterm <quantifier> op_quantifier
%nterm <quantifier> nonop_quantifier
%nterm <inst*> parent

%%
entity:
    inst {}
  | entity inst {}
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
        auto parent = obj.find($GLOBAL_ID);
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
    OP op_quantifier LOCAL_ID {
        $$ = op{$op_quantifier, std::move($LOCAL_ID)};
    }
  | PROP nonop_quantifier LOCAL_ID ARROW STRING {
        $$ = prop{$nonop_quantifier, std::move($LOCAL_ID), std::move($STRING)};
    }
  | RET nonop_quantifier LOCAL_ID { $$ = ret{$nonop_quantifier, std::move($LOCAL_ID)}; }
  | REG LOCAL_ID { $$ = reg{std::move($LOCAL_ID)}; }
  | CXX STRING { $$ = raw_cxx{std::move($STRING)}; }
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
%%

namespace mochi {
void parser::error(location_type const& l, std::string const& m) {
    std::cerr << l << ": " << m << std::endl;
}
}

