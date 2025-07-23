// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_20250611_HPP
#define CODEGEN_20250611_HPP

#include "object.hpp"

#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace mochi {

class objects;

class stream_writer {
  public:
    inline stream_writer(std::ostream &os) : os_{&os}, buffer_(2048) {}

    void operator()(std::string const &str) { *os_ << str; }
    void operator()(char const *fmt) { *os_ << fmt; }
    template <typename... T> void operator()(char const *fmt, T const &...args) {
        int size = 0;
        while (!size) {
            size = do_format(fmt, args...);
        }
        os_->write(buffer_.data(), size);
    }

    auto os() -> std::ostream & { return *os_; }

  private:
    template <typename T> auto convert_arg(T const &arg) {
        if constexpr (std::is_same_v<T, std::string>) {
            return arg.c_str();
        } else {
            return arg;
        }
    }

    template <typename... T> auto do_format(char const *fmt, T const &...args) -> int {
        int size = snprintf(buffer_.data(), buffer_.size(), fmt, convert_arg(args)...);
        if (size < 0) {
            throw std::runtime_error("snprintf failed");
        } else if (static_cast<std::size_t>(size) >= buffer_.size()) {
            buffer_.resize(size + 1);
            return 0;
        }
        return size;
    }

    std::ostream *os_;
    std::vector<char> buffer_;
};

template <typename F, typename... T> auto to_string(F generator, T &&...args) -> std::string {
    std::ostringstream oss;
    auto sw = stream_writer{oss};
    (*generator)(sw, std::forward<T>(args)...);
    return std::move(oss).str();
}

template <typename... T> auto format(char const *fmt, T const &...args) -> std::string {
    auto oss = std::ostringstream{};
    stream_writer{oss}(fmt, args...);
    return std::move(oss).str();
}

auto to_c_type(builtin_type ty) -> char const *;
auto to_cxx_type(builtin_type ty) -> char const *;
void generate_c_type(stream_writer &os, cxx_type const &ty);
void generate_cxx_type(stream_writer &os, cxx_type const &ty);
inline auto to_c_type(cxx_type const &ty) { return to_string(&generate_c_type, ty); }
inline auto to_cxx_type(cxx_type const &ty) { return to_string(&generate_cxx_type, ty); }
void generate_cxx_to_c_cast(stream_writer &sw, quantifier q, cxx_type const &ty, char const *name);
void generate_c_to_cxx_cast(stream_writer &sw, quantifier q, cxx_type const &ty, char const *name);

void generate_docstring(stream_writer &sw, std::string const &doc);

void generate_inst_params(
    inst *in,
    std::function<void(quantifier, cxx_type const &, char const *, char const *, bool)> format_arg);
void generate_inst_c_params(stream_writer &sw, inst *in);
void generate_inst_cxx_params(stream_writer &sw, inst *in);

void generate_type_params(
    type *root_ty,
    std::function<void(quantifier, cxx_type const &, char const *, char const *, bool)> format_arg);
void generate_type_c_params(stream_writer &sw, type *ty);
void generate_type_cxx_params(stream_writer &sw, type *ty);

void generate_api_builder_cpp(std::ostream &os, objects const &obj);
void generate_api_builder_h(std::ostream &os, objects const &obj);
void generate_api_builder_hpp(std::ostream &os, objects const &obj);

void generate_class_list_yaml(std::ostream &os, objects const &obj);

void generate_enum_cpp(std::ostream &os, objects const &obj);
void generate_enum_h(std::ostream &os, objects const &obj);
void generate_enum_hpp(std::ostream &os, objects const &obj);

void generate_inst_class(stream_writer &sw, inst *in);
void generate_inst_create(stream_writer &sw, inst *in);

void generate_inst_cpp(std::ostream &os, objects const &obj);
void generate_inst_hpp(std::ostream &os, objects const &obj);
void generate_inst_kind_cpp(std::ostream &os, objects const &obj);

auto needs_context_param(type *ty) -> bool;
void generate_type_class(stream_writer &sw, type *ty);

void generate_type_cpp(std::ostream &os, objects const &obj);
void generate_type_hpp(std::ostream &os, objects const &obj);

void generate_forward_hpp(std::ostream &os, objects const &obj);
void generate_visit_hpp(std::ostream &os, objects const &obj);

} // namespace mochi

#endif // CODEGEN_20250611_HPP
