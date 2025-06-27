// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen.hpp"
#include "object.hpp"
#include "objects.hpp"
#include "util/overloaded.hpp"
#include "walk.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

using tinytc::overloaded;

namespace mochi {

constexpr std::string base_parent = "inst_view";

auto to_c_type(builtin_type ty) -> char const * {
    switch (ty) {
    case builtin_type::bool_:
        return "tinytc_bool_t";
    case builtin_type::i32:
        return "int32_t";
    case builtin_type::i64:
        return "int64_t";
    case builtin_type::type:
        return "tinytc_data_type_t";
    case builtin_type::value:
        return "tinytc_value_t";
    }
}
auto to_cxx_type(builtin_type ty, bool pub) -> char const * {
    switch (ty) {
    case builtin_type::bool_:
        return "bool";
    case builtin_type::i32:
        return "std::int32_t";
    case builtin_type::i64:
        return "std::int64_t";
    case builtin_type::type:
        return pub ? "data_type" : "tinytc_data_type_t";
    case builtin_type::value:
        return pub ? "value" : "tinytc_value_t";
    }
}

void generate_c_type(std::ostream &os, data_type const &ty) {
    std::visit(overloaded{[&](builtin_type const &ty) { os << to_c_type(ty); },
                          [&](enum_ *const &ty) { os << std::format("tinytc_{}_t", ty->name()); },
                          [&](std::string const &ty) { os << ty; }},
               ty);
}
void generate_cxx_type(std::ostream &os, data_type const &ty, bool pub) {
    std::visit(overloaded{[&](builtin_type const &ty) { os << to_cxx_type(ty, pub); },
                          [&](enum_ *const &ty) { os << ty->name(); },
                          [&](std::string const &ty) { os << ty; }},
               ty);
}
void generate_cxx_to_c_cast(std::ostream &os, quantifier q, data_type const &ty,
                            std::string_view name) {
    std::visit(overloaded{[&](builtin_type const &) {
                              if (q == quantifier::many) {
                                  os << std::format("{}.size(), ", name);
                                  os << std::format("{}.data()", name);
                              } else {
                                  os << name;
                              }
                          },
                          [&](enum_ *const &ty) {
                              if (q == quantifier::many) {
                                  os << std::format("reinterpret_cast<const {}*>({}.data())",
                                                    to_c_type(ty), name);
                              } else {
                                  os << std::format("static_cast<{}>({})", to_c_type(ty), name);
                              }
                          },
                          [&](std::string const &) { os << name; }},
               ty);
}
void generate_c_to_cxx_cast(std::ostream &os, quantifier q, data_type const &ty,
                            std::string_view name) {
    std::visit(overloaded{[&](builtin_type const &) {
                              if (q == quantifier::many) {
                                  os << std::format("array_view{{{0}, {0}_size}}", name);
                              } else {
                                  os << name;
                              }
                          },
                          [&](enum_ *const &ty) {
                              if (q == quantifier::many) {
                                  os << std::format("array_view{{{0}, {0}_size}}", name);
                              } else {
                                  os << std::format("enum_cast<{}>({})", to_cxx_type(ty), name);
                              }
                          },
                          [&](std::string const &) { os << name; }},
               ty);
}

void generate_docstring(std::ostream &os, std::string const &doc) {
    auto docstream = std::istringstream(doc);
    std::string docline;
    while (std::getline(docstream, docline)) {
        os << std::format(" * {}\n", docline);
    }
}

void generate_params(
    inst *in, std::function<void(quantifier, data_type const &, std::string_view, std::string_view)>
                  format_arg) {
    walk_up<walk_order::post_order>(in, [&](inst *in) {
        for (auto &p : in->props()) {
            if (!p.private_) {
                format_arg(p.quantity, p.type, p.name, p.doc);
            }
        }
    });
    walk_up<walk_order::post_order>(in, [&](inst *in) {
        for (auto &o : in->ops()) {
            format_arg(o.quantity, builtin_type::value, o.name, o.doc);
        }
    });
    walk_up<walk_order::post_order>(in, [&](inst *in) {
        for (auto &r : in->rets()) {
            format_arg(r.quantity, builtin_type::type, r.name, r.doc);
        }
    });
}
void generate_c_params(std::ostream &os, inst *in) {
    os << "tinytc_inst_t *instr, ";
    generate_params(
        in, [&os](quantifier q, data_type const &ty, std::string_view name, std::string_view) {
            const auto type = to_c_type(ty);
            if (q == quantifier::many) {
                os << std::format("size_t {1}_size, const {0} *{1}, ", type, name);
            } else {
                os << std::format("{} {}, ", type, name);
            }
        });
    os << "const tinytc_location_t *loc";
}
void generate_cxx_params(std::ostream &os, inst *in, bool pub) {
    generate_params(
        in, [&](quantifier q, data_type const &ty, std::string_view name, std::string_view) {
            const auto type = to_cxx_type(ty, pub);
            if (q == quantifier::many) {
                os << std::format("array_view<{0}> {1}, ", type, name);
            } else {
                os << std::format("{} {}, ", type, name);
            }
        });
    os << "location const& loc";
}

void generate_api_builder_cpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                os << "tinytc_status_t tinytc_" << in->class_name() << "_create(";
                generate_c_params(os, in);
                os << ") {\nif (instr == nullptr) {";
                os << "return tinytc_status_invalid_arguments; }\nreturn "
                      "exception_to_status_code([&] "
                      "{\n";
                os << std::format("*instr = {}::create(", in->class_name());
                generate_params(in, [&os](quantifier q, data_type const &ty, std::string_view name,
                                          std::string_view) {
                    generate_c_to_cxx_cast(os, q, ty, name);
                    os << ", ";
                });
                os << "get_optional(loc)).release();\n});\n}\n\n";
            }
        });
    }
}
void generate_api_builder_h(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                os << "/**\n";
                generate_docstring(os, in->doc());
                os << " *\n";
                os << " * @param instr [out] pointer to the inst object created\n";
                generate_params(in, [&os](quantifier q, data_type const &, std::string_view name,
                                          std::string_view doc) {
                    if (q == quantifier::many) {
                        os << std::format(" * @param {0}_size [in] array size of {0}\n"
                                          " * @param {0} [in][range(0, {0}_size)] {1}; may be "
                                          "nullptr if {0}_size is 0\n",
                                          name, doc);
                    } else if (q == quantifier::optional) {
                        os << std::format(" * @param {} [in][optional] {}; can be nullptr\n", name,
                                          doc);
                    } else {
                        os << std::format(" * @param {} [in] {}\n", name, doc);
                    }
                });
                os << " * @param loc [in][optional] Source code location; can be nullptr\n *\n";
                os << " * @return tinytc_status_success on success and error otherwise\n */\n";
                os << "TINYTC_EXPORT tinytc_status_t tinytc_" << in->class_name() << "_create(";
                generate_c_params(os, in);
                os << ");\n\n";
            }
        });
    }
}
void generate_api_builder_hpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                const auto ret_count = [&]() {
                    std::int32_t ret_count = 0;

                    walk_up<walk_order::post_order>(in, [&](inst *in) {
                        for (auto &r : in->rets()) {
                            if (r.quantity == quantifier::many) {
                                ret_count = std::numeric_limits<std::int32_t>::max();
                            }
                            if (ret_count < std::numeric_limits<std::int32_t>::max()) {
                                ++ret_count;
                            }
                        }
                    });

                    return ret_count;
                }();

                os << std::format("//! creator specialization for {0}\n"
                                  "template<> struct creator<{0}> {{\n"
                                  "//! Maximum number of results of {0} instruction\n"
                                  "constexpr static std::int32_t max_returned_values = {1};\n",
                                  in->class_name(), ret_count);
                os << "/**\n";
                generate_docstring(os, in->doc());
                os << " *\n";
                generate_params(in, [&os](quantifier q, data_type const &, std::string_view name,
                                          std::string_view doc) {
                    if (q == quantifier::optional || q == quantifier::many) {
                        os << std::format(" * @param {} {}; can be {{}}\n", name, doc);
                    } else {
                        os << std::format(" * @param {} {}\n", name, doc);
                    }
                });
                os << " * @param loc Source code location; can be {}\n *\n";
                os << " * @return Instruction\n */\n";
                os << "inline auto operator()(";
                generate_cxx_params(os, in, true);
                os << "= {}) -> inst {\n";
                os << "tinytc_inst_t instr;\n";
                os << std::format("CHECK_STATUS_LOC(tinytc_{0}_create(&instr, \n",
                                  in->class_name());
                generate_params(in, [&os](quantifier q, data_type const &ty, std::string_view name,
                                          std::string_view) {
                    generate_cxx_to_c_cast(os, q, ty, name);
                    os << ", ";
                });
                os << "&loc), loc);\nreturn inst(instr);\n}\n};\n\n";
            }
        });
    }
}

void generate_enum_cpp(std::ostream &os, objects const &obj) {
    for (auto &e : obj.enums()) {
        os << std::format("char const* tinytc_{0}_to_string(tinytc_{0}_t val) {{", e->name());
        os << "switch (val) {\n";
        for (auto &c : e->cases()) {
            os << std::format("case tinytc_{0}_{1}: return \"{2}\";\n", e->name(), c.name,
                              e->is_set(enum_flag::doc_to_string) ? c.doc : c.name);
        }
        os << "}\nreturn \"unknown\";\n";
        os << "}\n\n";
    }
}
void generate_enum_h(std::ostream &os, objects const &obj) {
    for (auto &e : obj.enums()) {
        os << "/**\n";
        generate_docstring(os, e->doc());
        os << " */\ntypedef enum {";
        int i = 0;
        for (auto &c : e->cases()) {
            os << std::format("tinytc_{}_{} = {}, ///< {}\n", e->name(), c.name, c.value, c.doc);
            ++i;
        }
        os << std::format("}} tinytc_{}_t;\n", e->name());
        std::string uname = e->name();
        std::transform(uname.begin(), uname.end(), uname.begin(),
                       [](auto c) { return std::toupper(c); });
        os << std::format("#define TINYTC_ENUM_NUM_{} {}\n", uname, i);
        os << std::format("//! Convert {0} to string\nTINYTC_EXPORT char const* "
                          "tinytc_{0}_to_string(tinytc_{0}_t val);\n\n",
                          e->name());
    }
}
void generate_enum_hpp(std::ostream &os, objects const &obj) {
    for (auto &e : obj.enums()) {
        os << "/**\n";
        generate_docstring(os, e->doc());
        os << std::format(" */\nenum class {} {{", e->name());
        for (auto &c : e->cases()) {
            auto name = c.name;
            // \todo remove when unnecessary
            if (name == "and" || name == "or" || name == "xor" || name == "not") {
                name += "_";
            }
            os << std::format("{0} = {1}, ///< {2}\n", name, c.value, c.doc);
        }
        os << "};\n";
        os << std::format(
            "//! Convert {0} to string\ninline auto to_string({0} val) -> char const* {{ return "
            "::tinytc_{0}_to_string(static_cast<tinytc_{0}_t>(val)); }}\n\n",
            e->name());
    }
}

void generate_inst_class(std::ostream &os, inst *in) {
    inst *parent = in->parent();
    const auto parent_name = parent ? parent->class_name() : base_parent;
    os << std::format(R"CXXT(
class {0} : public {1} {{
public:
    using {1}::{1};
    struct alignas(8) properties : {1}::properties {{
)CXXT",
                      in->class_name(), parent_name);

    // Properties struct
    for (auto &o : in->ops()) {
        if (o.has_offset_property) {
            os << "std::int32_t " << o.offset_name() << ";\n";
        }
    }
    for (auto &p : in->props()) {
        const auto type = p.quantity == quantifier::many
                              ? std::format("std::vector<{}>", to_cxx_type(p.type))
                              : to_cxx_type(p.type);
        os << std::format("{} {};\n", type, p.name);
    }
    os << "};\n";

    // classof function
    os << "inline static bool classof(tinytc_inst const& i) {\n";
    if (in->has_children()) {
        os << std::format("return IK::{} <= i.type_id() && i.type_id() < IK::{};\n", in->ik_name(),
                          in->ik_name(true));
    } else {
        os << std::format("return IK::{} == i.type_id();\n", in->ik_name());
    }
    os << "}\n";

    // create function
    if (!in->has_children()) {
        os << "static auto create(";
        generate_cxx_params(os, in);
        os << ") -> inst;\n\n";
    }
    os << "\n";

    // property access
    os << "inline auto props() -> properties& { return "
          "*static_cast<properties*>(get().props()); }\n";
    for (auto &p : in->props()) {
        auto type = p.quantity == quantifier::many
                        ? std::format("array_view<{}>", to_cxx_type(p.type))
                        : to_cxx_type(p.type);
        os << std::format("inline auto {0}() -> {1} {{ return props().{0}; }}\n", p.name, type);
        os << std::format("inline void {0}({1} val) {{ props().{0} = val; }}\n", p.name, type);
    }
    os << "\n";

    // named operand access
    std::int32_t op_no = 0;
    for (auto it = in->ops().begin(); it != in->ops().end(); ++it) {
        auto offset =
            it->has_offset_property ? "props()." + it->offset_name() : std::to_string(op_no);
        auto peek = it + 1;
        auto next_offset = peek == in->ops().end()
                               ? "get().num_operands()"
                               : (peek->has_offset_property ? "props()." + peek->offset_name()
                                                            : std::to_string(op_no + 1));
        switch (it->quantity) {
        case quantifier::optional:
            os << std::format("inline auto has_{}() -> bool {{ return {} < {}; }}\n", it->name,
                              offset, next_offset);
        case quantifier::single:
            os << std::format("inline auto {0}() -> tinytc_value & {{ return get().op({1}); }}\n",
                              it->name, offset);
            break;
        case quantifier::many:
            os << std::format("inline auto {0}() -> op_range {{ return {{get().op_begin() + {1}, "
                              "get().op_begin() + {2}}}; }}\n",
                              it->name, offset, next_offset);
            break;
        }
        ++op_no;
    }
    os << "\n";

    // region access
    std::int32_t reg_no = 0;
    if (parent) {
        walk_up<walk_order::post_order>(parent,
                                        [&reg_no](inst *in) { reg_no += in->regs().size(); });
    }
    for (auto &r : in->regs()) {
        os << std::format(
            "inline auto {0}() -> tinytc_region& {{ return get().child_region({1}); }}\n", r.name,
            reg_no++);
    }
    os << "\n";

    // result access
    std::int32_t ret_no = 0;
    if (parent) {
        walk_up<walk_order::post_order>(parent,
                                        [&ret_no](inst *in) { ret_no += in->rets().size(); });
    }
    for (auto &r : in->rets()) {
        if (r.quantity == quantifier::many) {
            os << std::format("inline auto {0}() -> result_range {{ return {{get().result_begin() "
                              "+ {1}, get().result_end()}}; }}\n",
                              r.name, ret_no);
        } else {
            os << std::format(
                "inline auto {0}() -> tinytc_value& {{ return get().result({1}); }}\n", r.name,
                ret_no);
        }
        ++ret_no;
    }
    os << "\n";

    for (auto &code : in->cxx()) {
        os << code << "\n";
    }

    os << "void setup_and_check(); // throws compilation_error on invalid IR\n";

    os << "};\n";
    os << std::format(
        "static_assert(alignof({0}::properties) == alignof(tinytc_inst));\n"
        "static_assert(sizeof({0}::properties) <= std::numeric_limits<std::uint32_t>::max());\n\n",
        in->class_name());
}

void generate_inst_create(std::ostream &os, inst *in) {
    os << "auto " << in->class_name() << "::create(";
    generate_cxx_params(os, in);
    os << ") -> inst {\n";

    os << "std::int32_t num_operands = 0;\n"
       << "std::int32_t num_results = 0;\n";
    std::int32_t num_static_operands = 0, num_static_results = 0, num_child_regions = 0;
    walk_up<walk_order::post_order>(in, [&](inst *in) {
        for (auto &o : in->ops()) {
            switch (o.quantity) {
            case quantifier::single:
                ++num_static_operands;
                break;
            case quantifier::optional:
                os << std::format("safe_increase(num_operands, {} ? 1 : 0);\n", o.name);
                break;
            case quantifier::many:
                os << std::format("safe_increase(num_operands, {}.size());\n", o.name);
                break;
            }
        }
        num_child_regions += in->regs().size();
        for (auto &r : in->rets()) {
            if (r.quantity == quantifier::many) {
                os << std::format("safe_increase(num_results, {}.size());\n", r.name);
            } else {
                ++num_static_results;
            }
        }
    });
    if (num_static_operands) {
        os << std::format("safe_increase(num_operands, {});\n", num_static_operands);
    }
    if (num_static_results) {
        os << std::format("safe_increase(num_results, {});\n", num_static_results);
    }

    os << std::format(
        R"CXXT(auto layout = inst_layout{{
    num_results,
    num_operands,
    sizeof({1}::properties),
    {0},
}};
auto in = inst{{tinytc_inst::create(IK::{2}, layout, loc)}};
[[maybe_unused]] std::int32_t ret_no = 0;
[[maybe_unused]] std::int32_t op_no = 0;
)CXXT",
        num_child_regions, in->class_name(), in->ik_name());

    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &r : in->rets()) {
            if (r.quantity == quantifier::many) {
                os << std::format(R"CXXT(for (auto &r : {}) {{
    in->result(ret_no++, r);
}}
)CXXT",
                                  r.name);
            } else {
                os << std::format("in->result(ret_no++, {});\n", r.name);
            }
        }
    });
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                os << std::format("std::int32_t {} = op_no;\n", o.offset_name());
            }
            switch (o.quantity) {
            case quantifier::single:
                os << std::format("in->op(op_no++, {});\n", o.name);
                break;
            case quantifier::optional:
                os << std::format("if ({0}) {{ in->op(op_no++, {0}); }}", o.name);
                break;
            case quantifier::many:
                os << std::format("for (auto& o_ : {0}) {{ in->op(op_no++, o_); }}", o.name);
                break;
            }
        }
    });
    os << std::format("[[maybe_unused]] auto view = {}(in.get());\n", in->class_name());
    os << std::format("[[maybe_unused]] {}::properties& props = view.props();\n", in->class_name());
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                os << std::format("props.{0} = {0};\n", o.offset_name());
            }
        }
        for (auto &p : in->props()) {
            if (!p.private_) {
                os << std::format("props.{0} = std::move({0});", p.name);
            }
        }
    });
    os << "\n\n";

    os << "view.setup_and_check();\n\n";
    os << "return in;\n";
    os << "}\n\n";
}

void generate_inst_cpp(std::ostream &os, objects const &obj) {
    os << "auto to_string(IK ik) -> char const* {\n"
          "switch (ik) {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, true>(i.get(), [&os](inst *in) {
            os << std::format("case IK::IK_{0}: return \"{0}\";\n", in->name());
        });
    }
    os << "default: break;\n"
          "}\nreturn \"unknown\";\n"
          "}\n\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, true>(i.get(),
                                               [&os](inst *in) { generate_inst_create(os, in); });
    }
}

void generate_inst_hpp(std::ostream &os, objects const &obj) {
    os << "enum class IK {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(
            i.get(), [&os](inst *in) { os << in->ik_name() << ",\n"; },
            [&os](inst *in) {
                if (in->has_children()) {
                    os << in->ik_name(true) << ",\n";
                }
            }

        );
    }
    os << "};\n\n";
    os << "auto to_string(IK ik) -> char const*;\n\n";

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) { generate_inst_class(os, in); });
    }
}

void generate_inst_forward_hpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, true>(i.get(), [&os](inst *in) {
            os << std::format("class {0}; // IWYU pragma: export\n", in->class_name());
        });
    }
    os << "\n";
}

void generate_inst_visit_hpp(std::ostream &os, objects const &obj) {
    os << "template <typename Visitor> auto visit(Visitor && visitor, tinytc_inst &in) {\n";
    os << "switch(in.type_id()) {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, true>(i.get(), [&os](inst *in) {
            os << std::format("case IK::{}: {{ return visitor({}{{&in}}); }}\n", in->ik_name(),
                              in->class_name());
        });
    }
    os << "default: break;\n}\nthrow status::internal_compiler_error;\n}\n";
}

} // namespace mochi

