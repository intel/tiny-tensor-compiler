// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "object.hpp"
#include "util/overloaded.hpp"

#include <format>
#include <sstream>
#include <stdexcept>
#include <utility>

using tinytc::overloaded;

namespace mochi {

auto enum_::c_name() const -> std::string { return std::format("tinytc_{}_t", name_); }

auto to_c_type(basic_type ty) -> char const * {
    switch (ty) {
    case basic_type::bool_:
        return "tinytc_bool_t";
    case basic_type::i32:
        return "int32_t";
    case basic_type::i64:
        return "int64_t";
    }
}
auto to_cxx_type(basic_type ty) -> char const * {
    switch (ty) {
    case basic_type::bool_:
        return "bool";
    case basic_type::i32:
        return "std::int32_t";
    case basic_type::i64:
        return "std::int64_t";
    }
}

auto op::offset_name() const -> std::string {
    return (std::ostringstream{} << name << "_offset_").str();
}

auto prop::c_type() const -> std::string {
    return std::visit(overloaded{[&](basic_type const &ty) -> std::string { return to_c_type(ty); },
                                 [&](enum_ *const &ty) -> std::string { return ty->c_name(); },
                                 [&](std::string const &ty) -> std::string { return ty; }},
                      type);
}
auto prop::cxx_type() const -> std::string {
    return std::visit(
        overloaded{[&](basic_type const &ty) -> std::string { return to_cxx_type(ty); },
                   [&](enum_ *const &ty) -> std::string { return ty->cxx_name(); },
                   [&](std::string const &ty) -> std::string { return ty; }},
        type);
}

inst::inst(std::string name, std::string doc, std::vector<member> members, inst *parent)
    : name_{std::move(name)}, doc_{std::move(doc)}, parent_{parent} {
    bool needs_offset_property = false;
    bool has_star_ret = false;
    if (parent) {
        if (parent->ops().size() > 0) {
            needs_offset_property = parent->ops().back().has_offset_property;
        }
        if (parent->rets().size() > 0) {
            has_star_ret = parent->rets().back().quantity == quantifier::many;
        }
    }
    for (auto &&m : members) {
        std::visit(
            overloaded{
                [&](op &&i) {
                    i.has_offset_property = needs_offset_property;
                    ops_.emplace_back(std::move(i));
                    if (i.quantity != quantifier::single) {
                        needs_offset_property = true;
                    }
                },
                [&](prop &&i) { props_.emplace_back(std::move(i)); },
                [&](reg &&i) { regs_.emplace_back(std::move(i)); },
                [&](ret &&i) {
                    if (has_star_ret && i.quantity == quantifier::many) {
                        throw std::logic_error(
                            "Inst hierarchy must only have a single ret* and it must be last");
                    } else {
                        has_star_ret = true;
                    }
                    rets_.emplace_back(std::move(i));
                },
                [&](raw_cxx &&i) { cxx_.emplace_back(std::move(i.code)); }},
            std::move(m));
    }
}

auto inst::class_name() const -> std::string {
    return (std::ostringstream{} << name_ << "_inst").str();
}
auto inst::ik_name(bool end) const -> std::string {
    auto oss = std::ostringstream{};
    oss << "IK";
    if (end) {
        oss << "END";
    }
    oss << "_" << name_;
    return std::move(oss).str();
}

} // namespace mochi

