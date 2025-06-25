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

auto op::offset_name() const -> std::string {
    return (std::ostringstream{} << name << "_offset_").str();
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

