// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "omochi.hpp"

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>

namespace mochi {

auto lex_generator(std::size_t str_length, char const *str) -> std::optional<generator> {
    const std::uint8_t *YYLIMIT = reinterpret_cast<std::uint8_t const *>(str + str_length);
    const std::uint8_t *YYCURSOR = reinterpret_cast<std::uint8_t const *>(str);
    const std::uint8_t *YYMARKER;
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = std::uint8_t;
        re2c:eof = 1;

        "api_builder_cpp"   { return generator::api_builder_cpp; }
        "api_builder_h"     { return generator::api_builder_h; }
        "api_builder_hpp"   { return generator::api_builder_hpp; }
        "enum_cpp"          { return generator::enum_cpp; }
        "enum_h"            { return generator::enum_h; }
        "enum_hpp"          { return generator::enum_hpp; }
        "inst_hpp"          { return generator::inst_hpp; }
        "inst_cpp"          { return generator::inst_cpp; }
        "inst_visit_hpp"    { return generator::inst_visit_hpp; }
        "template"          { return generator::template_; }

        $                   { return std::nullopt; }
        *                   { return std::nullopt; }
    */
}

auto lex_omochi(std::size_t str_length, char const *str) -> std::optional<action> {
    const std::uint8_t *YYLIMIT = reinterpret_cast<std::uint8_t const *>(str + str_length);
    const std::uint8_t *YYCURSOR = reinterpret_cast<std::uint8_t const *>(str);
    const std::uint8_t *YYMARKER;

    const std::uint8_t *g1, *g2, *s1, *s2;
    /*!stags:re2c format = 'const std::uint8_t *@@;'; */

    auto a = action{};
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = std::uint8_t;
        re2c:eof = 1;
        re2c:tags = 1;
        re2c:flags:8 = 1;

        wsp = [ \t];
        omochi = [oO]? "mochi" | "お"? ("もち" | "餅");
        generator = [a-zA-Z0-9_]+;
        str = "\"" [^\"]* "\"";

        "//" wsp* omochi wsp+ @g1 generator @g2 wsp+ @s1 str @s2 {
            auto gen = lex_generator(g2-g1, reinterpret_cast<char const*>(g1));
            if (gen) {
                a.gen = *gen;
                a.filename = std::string(s1+1, s2-1);
                return a;
            }
            return std::nullopt;
        }

        $ { return std::nullopt; }
        * { return std::nullopt; }
    */
}

} // namespace mochi
