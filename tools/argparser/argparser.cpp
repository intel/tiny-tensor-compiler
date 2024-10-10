// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "support/fnv1a.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace tinytc::cmd {

auto to_string(parser_status status) -> char const * {
    switch (status) {
    case parser_status::invalid_short_opt:
        return "Short options must be alphanumeric";
    case parser_status::unknown_short_opt:
        return "Unknown short option";
    case parser_status::invalid_long_opt:
        return "Long options must be lowercase alphanumeric words, optionally separated by hyphens";
    case parser_status::unknown_long_opt:
        return "Unknown long option";
    case parser_status::unknown_positional_arg:
        return "Unknown positional argument";
    case parser_status::required_argument_missing:
        return "Required argument missing";
    case parser_status::flag_does_not_take_argument:
        return "Flag does not take argument";
    case parser_status::converter_functional_missing:
        return "Non-default convertible type need converter functional";
    case parser_status::invalid_argument:
        return "Invalid argument";
    case parser_status::argument_out_of_range:
        return "Argument is out of range";
    case parser_status::required_must_not_follow_optional:
        return "Required positional argument must not follow optional positional argument";
    case parser_status::positional_must_not_follow_multiarg:
        return "Positional argument must not follow positional ellipsis argument";
    case parser_status::hash_conflict:
        return "Long option hash conflict, please rename one of the long options";
    case parser_status::success:
        break;
    }
    return "";
}

arg_parser::arg_parser() : short_(2 * 26 + 10) {}

void arg_parser::parse(int argc, char **argv) {
    auto const parse_short = [&](int pos, int subpos) -> int {
        char const *str = argv[pos];
        do {
            if (!std::isalnum(str[subpos])) {
                throw arg_parser_error(argc, argv, pos, subpos, parser_status::invalid_short_opt);
            }
            auto &shortopt = short_[short_index(argv[pos][subpos])];
            if (shortopt.par) {
                ++subpos;
                if (shortopt.par->is_flag()) {
                    shortopt.par->set(nullptr);
                } else {
                    auto status = parser_status::success;
                    if (str[subpos] != 0) {
                        status = shortopt.par->set(str + subpos);
                    } else if (shortopt.par->is_argument_required()) {
                        status = shortopt.par->set(pos + 1 < argc ? argv[++pos] : nullptr);
                    } else {
                        status = shortopt.par->set(nullptr);
                    }
                    if (status != parser_status::success) {
                        throw arg_parser_error(argc, argv, pos, subpos, status);
                    }
                    break;
                }
            } else {
                throw arg_parser_error(argc, argv, pos, subpos, parser_status::unknown_short_opt);
            }
        } while (str[subpos]);
        return pos;
    };

    auto const parse_long = [&](int pos, int subpos) -> int {
        char const *str = argv[pos];

        int key_end = subpos;
        while (str[key_end] != '=' && str[key_end] != 0) {
            ++key_end;
        }
        const auto key = fnv1a(str + subpos, key_end - subpos);
        if (long_.find(key) == long_.end()) {
            throw arg_parser_error(argc, argv, pos, subpos, parser_status::unknown_long_opt);
        }

        auto &longopt = long_[key];
        if (longopt.par->is_flag()) {
            longopt.par->set(nullptr);
            if (str[key_end] != 0) {
                throw arg_parser_error(argc, argv, pos, subpos,
                                       parser_status::flag_does_not_take_argument);
            }
        } else {
            auto status = parser_status::success;
            if (str[key_end] != 0) {
                status = longopt.par->set(str + key_end + 1);
            } else {
                status = longopt.par->set(nullptr);
            }
            if (status != parser_status::success) {
                throw arg_parser_error(argc, argv, pos, key_end + 1, status);
            }
        }
        return pos;
    };

    std::size_t positional_arg_index = 0;
    auto parse_positional = [&](int pos, int subpos) -> int {
        if (positional_arg_index >= positional_.size()) {
            throw arg_parser_error(argc, argv, pos, subpos, parser_status::unknown_positional_arg);
        }
        auto &arg = positional_[positional_arg_index];
        arg.par->set(argv[pos]);
        if (!arg.par->does_store_multiple()) {
            ++positional_arg_index;
        }
        return pos;
    };

    int pos = 1;
    for (; pos < argc; ++pos) {
        int subpos = 0;
        if (argv[pos][subpos] == '-') {
            ++subpos;
            if (argv[pos][subpos] == '-') {
                ++subpos;
                if (argv[pos][subpos] == 0) {
                    ++pos;
                    pos = parse_positional(pos, subpos);
                } else {
                    pos = parse_long(pos, subpos);
                }
            } else {
                pos = parse_short(pos, subpos);
            }
        } else {
            pos = parse_positional(pos, subpos);
        }
    }

    if (positional_arg_index < positional_.size() &&
        positional_[positional_arg_index].par->is_argument_required()) {
        throw arg_parser_error(argc, argv, pos, 0, parser_status::required_argument_missing);
    }
}

void arg_parser::print_help(std::ostream &os, char const *name, char const *description) {
    constexpr int optwidth = 20;

    const auto print = [&](auto const &key, auto const &par, char const *init, char const *sep_req,
                           char const *sep_nonreq) {
        if (par) {
            os << '[' << init << key;
            if (!par->is_flag()) {
                const bool req = par->is_argument_required();
                if (!req) {
                    os << '[';
                }
                os << (req ? sep_req : sep_nonreq);
                os << "arg";
                if (!req) {
                    os << ']';
                }
            }
            os << ']';
            if (par->does_store_multiple()) {
                os << "...";
            }
        }
    };
    const auto print_short = [&](char i) {
        auto const &par = short_[short_index(i)].par;
        print(i, par, "-", " ", "");
    };
    const auto print_short_help = [&](char i) {
        auto const &opt = short_[short_index(i)];
        if (opt.par) {
            os << "   -" << std::left << std::setw(optwidth) << i << opt.help << std::endl;
        }
    };
    os << "Usage: " << name;
    for (char i = '0'; i < '9'; ++i) {
        print_short(i);
    }
    for (char i = 'a'; i < 'z'; ++i) {
        print_short(i);
        print_short(std::toupper(i));
    }
    auto long_opts = std::vector<long_opt *>{};
    long_opts.reserve(long_.size());
    for (auto it = long_.begin(); it != long_.end(); ++it) {
        long_opts.emplace_back(&it->second);
    }
    std::sort(long_opts.begin(), long_opts.end(),
              [&](long_opt *a, long_opt *b) { return std::strcmp(a->opt, b->opt) < 0; });
    for (auto const &opt : long_opts) {
        print(opt->opt, opt->par, "--", "=", "=");
    }
    for (auto const &pos : positional_) {
        const bool req = pos.par->is_argument_required();
        os << (req ? ' ' : '[');
        os << pos.opt;
        if (pos.par->does_store_multiple()) {
            os << "...";
        }
        os << (req ? ' ' : ']');
    }
    os << std::endl << description << std::endl << std::endl;

    os << "Positional arguments:" << std::endl;
    for (auto const &pos : positional_) {
        os << "    " << std::left << std::setw(optwidth) << pos.opt << pos.help << std::endl;
    }

    os << std::endl << "Options:" << std::endl;
    for (char i = '0'; i < '9'; ++i) {
        print_short_help(i);
    }
    for (char i = 'a'; i < 'z'; ++i) {
        print_short_help(i);
        print_short_help(std::toupper(i));
    }
    for (auto const &opt : long_opts) {
        os << "  --" << std::left << std::setw(optwidth) << opt->opt << opt->help << std::endl;
    }
}

auto arg_parser::short_index(char opt) const -> std::size_t {
    if (opt >= 'a') {
        return 10 + 26 + (opt - 'a');
    } else if (opt >= 'A') {
        return 10 + (opt - 'A');
    }
    return opt - '0';
}

void arg_parser::set_short_opt(char key, short_opt value) {
    if (!std::isalnum(key)) {
        throw std::logic_error(to_string(parser_status::invalid_short_opt));
    }
    short_[short_index(key)] = std::move(value);
}

void arg_parser::set_long_opt(long_opt value) {
    const auto opt_len = std::strlen(value.opt);
    if (!std::all_of(value.opt, value.opt + opt_len,
                     [](char c) { return std::islower(c) || std::isdigit(c) || c == '-'; })) {
        throw std::logic_error(to_string(parser_status::invalid_long_opt));
    }
    const auto key = fnv1a(value.opt, opt_len);
    if (auto it = long_.find(key);
        it != long_.end() && std::strcmp(value.opt, it->second.opt) != 0) {
        throw std::logic_error(to_string(parser_status::hash_conflict));
    }
    long_[key] = std::move(value);
}

void arg_parser::add_positional_arg(positional_arg value) {
    if (!positional_.empty()) {
        if (value.par->is_argument_required() && !positional_.back().par->is_argument_required()) {
            throw std::logic_error(to_string(parser_status::required_must_not_follow_optional));
        }
        if (positional_.back().par->does_store_multiple()) {
            throw std::logic_error(to_string(parser_status::positional_must_not_follow_multiarg));
        }
    }
    positional_.emplace_back(std::move(value));
}

arg_parser_error::arg_parser_error(int argc, char **argv, int pos, int subpos,
                                   parser_status status) {
    auto oss = std::ostringstream{};
    oss << "==> Error in" << std::endl;
    int offset = 0;
    for (int i = 0; i < pos; ++i) {
        oss << argv[i] << ' ';
        offset += std::strlen(argv[i]) + 1;
    }
    if (pos < argc) {
        oss << argv[pos];
    }
    oss << std::endl;
    const auto offset_str = std::string(offset + subpos, ' ');
    oss << offset_str << '^' << std::endl;
    oss << offset_str << to_string(status);

    what_ = std::move(oss).str();
}

} // namespace tinytc::cmd
