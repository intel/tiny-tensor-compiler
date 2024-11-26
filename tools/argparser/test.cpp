// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"

#include <exception>
#include <iostream>
#include <optional>
#include <vector>

using namespace tinytc;

struct args {
    bool f = false;
    int a = -1;
    short b = -1;
    bool foo = false;
    short bar = -1;
    bool help = false;
    int c = 0;
    int d = 0;
    std::vector<int> m = {};
    std::vector<int> m2 = {};
};

int main(int argc, char **argv) {
    auto a = args{};
    auto parser = cmd::arg_parser{};

    try {
        parser.set_short_opt('f', &a.f, "f opt");
        parser.set_short_opt('a', &a.a, "a opt", 5).validator([](int a) { return a > 0; });
        parser.set_short_opt('b', &a.b, "b opt");
        parser.set_short_opt('h', &a.help, "show help");
        parser.set_long_opt("help", &a.help, "show help");
        parser.set_long_opt("foo", &a.foo, "foo opt");
        parser.set_long_opt("bar", &a.bar, "bar opt");
        parser.set_long_opt("bar2", &a.bar, "bar opt", 5);
        parser.add_positional_arg("c", &a.c, "c arg", true);
        parser.add_positional_arg("d", &a.d, "d arg");
        parser.set_short_opt('m', &a.m, "m arg");
        parser.add_positional_arg("m2", &a.m2, "m2 arg");
        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        if (!a.help) {
            std::cerr << e.what() << std::endl;
            return -1;
        }
    }
    if (a.help) {
        parser.print_help(std::cout, "test-argparser", "Test of libargparser");
        return 0;
    }

    std::cout << "f: " << a.f << std::endl;
    std::cout << "a: " << a.a << std::endl;
    std::cout << "b: " << a.b << std::endl;
    std::cout << "foo: " << a.foo << std::endl;
    std::cout << "bar: " << a.bar << std::endl;
    std::cout << "c: " << a.c << std::endl;
    std::cout << "d: " << a.d << std::endl;
    std::cout << "m: ";
    for (auto const &mm : a.m) {
        std::cout << mm << ' ';
    }
    std::cout << std::endl;
    std::cout << "m2: ";
    for (auto const &mm : a.m2) {
        std::cout << mm << ' ';
    }
    std::cout << std::endl;

    return 0;
}
