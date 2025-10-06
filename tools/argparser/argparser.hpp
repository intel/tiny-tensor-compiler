// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGPARSER_20241008_HPP
#define ARGPARSER_20241008_HPP

#include <cerrno>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iosfwd>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc::cmd {

enum class parser_status {
    success,
    invalid_short_opt,
    unknown_short_opt,
    invalid_long_opt,
    unknown_long_opt,
    unknown_positional_arg,
    required_argument_missing,
    flag_does_not_take_argument,
    converter_functional_missing,
    invalid_argument,
    validator_failed,
    argument_out_of_range,
    required_must_not_follow_optional,
    positional_must_not_follow_multiarg,
    hash_conflict,
};
auto to_string(parser_status status) -> char const *;

template <typename T> struct default_converter;
template <> struct default_converter<char> {
    auto operator()(char const *str, char &val) const -> parser_status {
        val = str[0];
        return parser_status::success;
    }
};
template <std::integral T> struct default_converter<T> {
    auto operator()(char const *str, T &val) const -> parser_status {
        long v = strtol(str, nullptr, 0);
        if (errno == ERANGE || static_cast<T>(v) < std::numeric_limits<T>::min() ||
            static_cast<T>(v) > std::numeric_limits<T>::max()) {
            return parser_status::argument_out_of_range;
        }
        val = v;
        return parser_status::success;
    }
};
template <> struct default_converter<char const *> {
    auto operator()(char const *str, char const *&val) const -> parser_status {
        val = str;
        return parser_status::success;
    }
};

template <typename> struct is_defined : std::false_type {};
template <typename T>
requires(sizeof(T) > 0)
struct is_defined<T> : std::true_type {};
template <typename T> constexpr bool is_defined_v = is_defined<T>::value;

class par_concept {
  public:
    virtual ~par_concept() = default;
    virtual auto set(char const *optional_argument) -> parser_status = 0;
    virtual auto is_flag() const -> bool = 0;
    virtual auto is_argument_required() const -> bool = 0;
    virtual auto does_store_multiple() const -> bool = 0;
};

template <typename T> class par_model : public par_concept {
  public:
    using value_type = T;

    par_model(T *ptr, std::optional<T> default_argument)
        : ptr_(ptr), default_argument_(std::move(default_argument)) {}
    auto set(char const *optional_argument) -> parser_status override {
        auto status = parser_status::success;
        if (optional_argument != nullptr) {
            if (converter_) {
                status = converter_(optional_argument, *ptr_);
            } else {
                if constexpr (is_defined_v<default_converter<T>>) {
                    status = default_converter<T>{}(optional_argument, *ptr_);
                } else {
                    status = parser_status::converter_functional_missing;
                }
            }
        } else if (default_argument_.has_value()) {
            *ptr_ = *default_argument_;
        } else {
            status = parser_status::required_argument_missing;
        }
        if (validator_ && status == parser_status::success && !validator_(*ptr_)) {
            status = parser_status::validator_failed;
        }
        return status;
    }
    auto is_flag() const -> bool override { return false; }
    auto is_argument_required() const -> bool override { return !default_argument_.has_value(); }
    auto does_store_multiple() const -> bool override { return false; }

    template <typename F> auto converter(F &&fun) -> par_model<T> & {
        converter_ = std::forward<F>(fun);
        return *this;
    }
    template <typename F> auto validator(F &&fun) -> par_model<T> & {
        validator_ = std::forward<F>(fun);
        return *this;
    }

  protected:
    T *ptr_;

  private:
    std::optional<T> default_argument_;
    std::function<parser_status(char const *, T &)> converter_;
    std::function<bool(T const &)> validator_;
};

template <> class par_model<bool> : public par_concept {
  public:
    par_model(bool *ptr) : ptr_(ptr) {}
    auto set(char const *) -> parser_status override {
        *ptr_ = true;
        return parser_status::success;
    }
    auto is_flag() const -> bool override { return true; }
    auto is_argument_required() const -> bool override { return false; }
    auto does_store_multiple() const -> bool override { return false; }

  protected:
    bool *ptr_;
};

template <typename T> class par_model<std::vector<T>> : public par_model<T> {
  public:
    par_model(std::vector<T> *ptr, std::optional<T> default_argument)
        : par_model<T>{nullptr, std::move(default_argument)}, vptr_(ptr) {}

    auto set(char const *optional_argument) -> parser_status override {
        vptr_->emplace_back(T{});
        this->ptr_ = &vptr_->back();
        return this->template par_model<T>::set(optional_argument);
    }
    auto does_store_multiple() const -> bool override { return true; }

  private:
    std::vector<T> *vptr_;
};

class arg_parser {
  public:
    static int optindent;
    static int optwidth;

    arg_parser();

    inline void set_short_opt(char opt, bool *ptr, char const *help = nullptr) {
        set_short_opt(opt, {help, std::make_unique<par_model<bool>>(ptr)});
    }

    template <typename T>
    auto
    set_short_opt(char opt, T *ptr, char const *help = nullptr,
                  std::optional<typename par_model<T>::value_type> default_argument = std::nullopt)
        -> par_model<T> & {
        auto model = std::make_unique<par_model<T>>(ptr, std::move(default_argument));
        auto model_ptr = model.get();
        set_short_opt(opt, {help, std::move(model)});
        return *model_ptr;
    }

    inline void set_long_opt(char const *opt, bool *ptr, char const *help = nullptr) {
        set_long_opt({opt, help, std::make_unique<par_model<bool>>(ptr)});
    }

    template <typename T>
    auto
    set_long_opt(char const *opt, T *ptr, char const *help = nullptr,
                 std::optional<typename par_model<T>::value_type> default_argument = std::nullopt)
        -> par_model<T> & {
        auto model = std::make_unique<par_model<T>>(ptr, std::move(default_argument));
        auto model_ptr = model.get();
        set_long_opt({opt, help, std::move(model)});
        return *model_ptr;
    }

    template <typename T>
    auto add_positional_arg(char const *opt, T *ptr, char const *help = nullptr,
                            bool required = false) -> par_model<T> & {
        auto model =
            std::make_unique<par_model<T>>(ptr, required ? std::nullopt : std::make_optional(*ptr));
        auto model_ptr = model.get();
        add_positional_arg(positional_arg{opt, help, std::move(model)});
        return *model_ptr;
    }

    template <typename T>
    auto add_positional_arg(char const *opt, std::vector<T> *ptr, char const *help = nullptr)
        -> par_model<T> & {
        auto model = std::make_unique<par_model<std::vector<T>>>(ptr, std::make_optional(T{}));
        auto model_ptr = model.get();
        add_positional_arg({opt, help, std::move(model)});
        return *model_ptr;
    }

    void parse(int argc, char **argv);
    void print_help(std::ostream &os, char const *name, char const *description);

  private:
    struct short_opt {
        char const *help;
        std::unique_ptr<par_concept> par;
    };
    struct long_opt {
        char const *opt;
        char const *help;
        std::unique_ptr<par_concept> par;
    };
    struct positional_arg {
        char const *opt;
        char const *help;
        std::unique_ptr<par_concept> par;
    };

    auto short_index(char opt) const -> std::size_t;
    void set_short_opt(char key, short_opt value);
    void set_long_opt(long_opt value);
    void add_positional_arg(positional_arg value);

    std::vector<short_opt> short_;
    std::unordered_map<std::uint64_t, long_opt> long_;
    std::vector<positional_arg> positional_;
};

class arg_parser_error : public std::exception {
  public:
    arg_parser_error(int argc, char **argv, int pos, int subpos, parser_status status);
    inline char const *what() const noexcept override { return what_.c_str(); }

  private:
    std::string what_;
};

} // namespace tinytc::cmd

#endif // ARGPARSER_20241008_HPP
