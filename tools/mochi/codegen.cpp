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

constexpr std::string inst_base_parent = "inst_view";
constexpr std::string type_base_parent = "tinytc_type";

auto to_c_type(builtin_type ty) -> char const * {
    switch (ty) {
    case builtin_type::bool_:
        return "tinytc_bool_t";
    case builtin_type::compiler_context_t:
        return "tinytc_compiler_context_t";
    case builtin_type::i32:
        return "int32_t";
    case builtin_type::i64:
        return "int64_t";
    case builtin_type::type_t:
        return "tinytc_type_t";
    case builtin_type::value_t:
        return "tinytc_value_t";
    }
}
auto to_cxx_type(builtin_type ty) -> char const * {
    switch (ty) {
    case builtin_type::bool_:
        return "bool";
    case builtin_type::compiler_context_t:
        return "tinytc_compiler_context_t";
    case builtin_type::i32:
        return "std::int32_t";
    case builtin_type::i64:
        return "std::int64_t";
    case builtin_type::type_t:
        return "tinytc_type_t";
    case builtin_type::value_t:
        return "tinytc_value_t";
    }
}

void generate_c_type(std::ostream &os, cxx_type const &ty) {
    std::visit(overloaded{[&](builtin_type const &ty) { os << to_c_type(ty); },
                          [&](enum_ *const &ty) { os << std::format("tinytc_{}_t", ty->name()); },
                          [&](std::string const &ty) { os << ty; }},
               ty);
}
void generate_cxx_type(std::ostream &os, cxx_type const &ty) {
    std::visit(overloaded{[&](builtin_type const &ty) { os << to_cxx_type(ty); },
                          [&](enum_ *const &ty) { os << ty->name(); },
                          [&](std::string const &ty) { os << ty; }},
               ty);
}
void generate_cxx_to_c_cast(std::ostream &os, quantifier q, cxx_type const &ty,
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
void generate_c_to_cxx_cast(std::ostream &os, quantifier q, cxx_type const &ty,
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

void generate_inst_params(
    inst *root_in,
    std::function<void(quantifier, cxx_type const &, std::string_view, std::string_view, bool)>
        format_arg) {
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        for (auto &p : in->props()) {
            if (!p.private_) {
                format_arg(p.quantity, p.type, p.name, p.doc, false);
            }
        }
    });
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        for (auto &o : in->ops()) {
            format_arg(o.quantity, builtin_type::value_t, o.name, o.doc, false);
        }
    });
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        auto r = in->rets().begin();
        auto end = in->rets().end();
        for (; r != end; ++r) {
            format_arg(r->quantity, builtin_type::type_t, r->name, r->doc,
                       in == root_in && r + 1 == end);
        }
    });
}
void generate_inst_c_params(std::ostream &os, inst *in) {
    os << "tinytc_inst_t *instr, ";
    generate_inst_params(
        in, [&os](quantifier q, cxx_type const &ty, std::string_view name, std::string_view, bool) {
            const auto type = to_c_type(ty);
            if (q == quantifier::many) {
                os << std::format("size_t {1}_size, const {0} *{1}, ", type, name);
            } else {
                os << std::format("{} {}, ", type, name);
            }
        });
    os << "const tinytc_location_t *loc";
}
void generate_inst_cxx_params(std::ostream &os, inst *in) {
    generate_inst_params(
        in, [&](quantifier q, cxx_type const &ty, std::string_view name, std::string_view, bool) {
            const auto type = to_cxx_type(ty);
            if (q == quantifier::many) {
                os << std::format("array_view<{0}> {1}, ", type, name);
            } else {
                os << std::format("{} {}, ", type, name);
            }
        });
    os << "location const& loc";
}

void generate_type_params(
    type *root_ty,
    std::function<void(quantifier, cxx_type const &, std::string_view, std::string_view, bool)>
        format_arg) {
    if (needs_context_param(root_ty)) {
        bool is_last = true;
        walk_up<walk_order::post_order, type>(root_ty, [&](type *ty) {
            if (ty->props().size() > 0) {
                is_last = false;
            }
        });
        format_arg(quantifier::single, builtin_type::compiler_context_t, "ctx", "compiler context",
                   is_last);
    }
    walk_up<walk_order::post_order, type>(root_ty, [&](type *ty) {
        auto p = ty->props().begin();
        auto end = ty->props().end();
        for (; p != end; ++p) {
            if (!p->private_) {
                format_arg(p->quantity, p->type, p->name, p->doc, ty == root_ty && p + 1 == end);
            }
        }
    });
}
void generate_type_c_params(std::ostream &os, type *ty) {
    os << "tinytc_type_t *ty_, ";
    generate_type_params(ty, [&os](quantifier q, cxx_type const &ty, std::string_view name,
                                   std::string_view, bool is_last) {
        const auto type = to_c_type(ty);
        if (q == quantifier::many) {
            os << std::format("size_t {1}_size, const {0} *{1}", type, name);
        } else {
            os << std::format("{} {}", type, name);
        }
        if (!is_last) {
            os << ", ";
        }
    });
}
void generate_type_cxx_params(std::ostream &os, type *ty) {
    generate_type_params(ty, [&](quantifier q, cxx_type const &ty, std::string_view name,
                                 std::string_view, bool is_last) {
        const auto type = to_cxx_type(ty);
        if (q == quantifier::many) {
            os << std::format("array_view<{0}> {1}", type, name);
        } else {
            os << std::format("{} {}", type, name);
        }
        if (!is_last) {
            os << ", ";
        }
    });
}

void generate_api_builder_cpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&os](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                os << "tinytc_status_t tinytc_" << in->class_name() << "_create(";
                generate_inst_c_params(os, in);
                os << ") {\nif (instr == nullptr) {";
                os << "return tinytc_status_invalid_arguments; }\nreturn "
                      "exception_to_status_code([&] "
                      "{\n";
                os << std::format("*instr = {}::create(", in->class_name());
                generate_inst_params(in, [&os](quantifier q, cxx_type const &ty,
                                               std::string_view name, std::string_view, bool) {
                    generate_c_to_cxx_cast(os, q, ty, name);
                    os << ", ";
                });
                os << "get_optional(loc)).release();\n});\n}\n\n";
            }
        });
    }

    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&os](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                os << "tinytc_status_t tinytc_" << ty->class_name() << "_get(";
                generate_type_c_params(os, ty);
                os << ") {\nif (ty_ == nullptr) {";
                os << "return tinytc_status_invalid_arguments; }\nreturn "
                      "exception_to_status_code([&] "
                      "{\n";
                os << std::format("*ty_ = {}::get(", ty->class_name());
                generate_type_params(ty,
                                     [&os](quantifier q, cxx_type const &ty, std::string_view name,
                                           std::string_view, bool is_last) {
                                         generate_c_to_cxx_cast(os, q, ty, name);
                                         if (!is_last) {
                                             os << ", ";
                                         }
                                     });
                os << ");\n});\n}\n\n";
            }
        });
    }
}
void generate_api_builder_h(std::ostream &os, objects const &obj) {
    const auto param_doc = [&os](quantifier q, cxx_type const &, std::string_view name,
                                 std::string_view doc, bool) {
        if (q == quantifier::many) {
            os << std::format(" * @param {0}_size [in] array size of {0}\n"
                              " * @param {0} [in][range(0, {0}_size)] {1}; may be "
                              "nullptr if {0}_size is 0\n",
                              name, doc);
        } else if (q == quantifier::optional) {
            os << std::format(" * @param {} [in][optional] {}; can be nullptr\n", name, doc);
        } else {
            os << std::format(" * @param {} [in] {}\n", name, doc);
        }
    };
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&os, &param_doc](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                os << "/**\n";
                generate_docstring(os, in->doc());
                os << " *\n";
                os << " * @param instr [out] pointer to the inst object created\n";
                generate_inst_params(in, param_doc);
                os << " * @param loc [in][optional] Source code location; can be nullptr\n *\n";
                os << " * @return tinytc_status_success on success and error otherwise\n */\n";
                os << "TINYTC_EXPORT tinytc_status_t tinytc_" << in->class_name() << "_create(";
                generate_inst_c_params(os, in);
                os << ");\n\n";
            }
        });
    }
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&os, &param_doc](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                os << "/**\n";
                generate_docstring(os, ty->doc());
                os << " *\n";
                os << " * @param ty_ [out] pointer to the type object created\n";
                generate_type_params(ty, param_doc);
                os << " * @return tinytc_status_success on success and error otherwise\n */\n";
                os << "TINYTC_EXPORT tinytc_status_t tinytc_" << ty->class_name() << "_get(";
                generate_type_c_params(os, ty);
                os << ");\n\n";
            }
        });
    }
}
void generate_api_builder_hpp(std::ostream &os, objects const &obj) {
    auto const param_doc = [&os](quantifier q, cxx_type const &, std::string_view name,
                                 std::string_view doc, bool) {
        if (q == quantifier::optional || q == quantifier::many) {
            os << std::format(" * @param {} {}; can be {{}}\n", name, doc);
        } else {
            os << std::format(" * @param {} {}\n", name, doc);
        }
    };
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&os, &param_doc](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                const auto ret_count = [&]() {
                    std::int32_t ret_count = 0;

                    walk_up<walk_order::post_order, inst>(in, [&](inst *in) {
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
                generate_inst_params(in, param_doc);
                os << " * @param loc Source code location; can be {}\n *\n";
                os << " * @return Instruction\n */\n";
                os << "inline auto operator()(";
                generate_inst_cxx_params(os, in);
                os << "= {}) -> unique_handle<tinytc_inst_t> {\n";
                os << "tinytc_inst_t instr;\n";
                os << std::format("CHECK_STATUS_LOC(tinytc_{0}_create(&instr, \n",
                                  in->class_name());
                generate_inst_params(in, [&os](quantifier q, cxx_type const &ty,
                                               std::string_view name, std::string_view, bool) {
                    generate_cxx_to_c_cast(os, q, ty, name);
                    os << ", ";
                });
                os << "&loc), loc);\nreturn unique_handle{instr};\n}\n};\n\n";
            }
        });
    }
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&os, &param_doc](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                os << std::format("//! getter specialization for {0}\n"
                                  "template<> struct getter<{0}> {{\n",
                                  ty->class_name());
                os << "/**\n";
                generate_docstring(os, ty->doc());
                os << " *\n";
                generate_type_params(ty, param_doc);
                os << " * @return Instruction\n */\n";
                os << "inline auto operator()(";
                generate_type_cxx_params(os, ty);
                os << ") -> tinytc_type_t {\n";
                os << "tinytc_type_t ty_;\n";
                os << std::format("CHECK_STATUS(tinytc_{0}_get(&ty_, \n", ty->class_name());
                generate_type_params(ty,
                                     [&os](quantifier q, cxx_type const &ty, std::string_view name,
                                           std::string_view, bool is_last) {
                                         generate_cxx_to_c_cast(os, q, ty, name);
                                         if (!is_last) {
                                             os << ", ";
                                         }
                                     });
                os << "));\nreturn ty_;\n}\n};\n\n";
            }
        });
    }
}

void generate_class_list_yaml(std::ostream &os, objects const &obj) {
    os << "enum:\n";
    for (auto &o : obj.enums()) {
        os << std::format("- {}\n", o->name());
    }
    os << "type:\n";
    for (auto &o : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(
            o.get(), [&](type *ty) { os << std::format("- {}\n", ty->name()); });
    }
    os << "inst:\n";
    for (auto &o : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(
            o.get(), [&](inst *in) { os << std::format("- {}\n", in->name()); });
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
        os << std::format("//! Convert {0} to string\ninline auto to_string({0} val) -> char "
                          "const* {{ return "
                          "::tinytc_{0}_to_string(static_cast<tinytc_{0}_t>(val)); }}\n\n",
                          e->name());
    }
}

void generate_inst_class(std::ostream &os, inst *in) {
    inst *parent = in->parent();
    const auto parent_name = parent ? parent->class_name() : inst_base_parent;
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
        os << std::format("return IK::{} <= i.type_id() && i.type_id() < IK::{};\n",
                          in->kind_name(), in->kind_name(true));
    } else {
        os << std::format("return IK::{} == i.type_id();\n", in->kind_name());
    }
    os << "}\n";

    // create function
    if (!in->has_children()) {
        os << "static auto create(";
        generate_inst_cxx_params(os, in);
        os << ") -> unique_handle<tinytc_inst_t>;\n\n";
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
        walk_up<walk_order::post_order, inst>(parent,
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
        walk_up<walk_order::post_order, inst>(parent,
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
    os << std::format("static_assert(alignof({0}::properties) == alignof(tinytc_inst));\n"
                      "static_assert(sizeof({0}::properties) <= "
                      "std::numeric_limits<std::uint32_t>::max());\n\n",
                      in->class_name());
}

void generate_inst_create(std::ostream &os, inst *in) {
    os << "auto " << in->class_name() << "::create(";
    generate_inst_cxx_params(os, in);
    os << ") -> unique_handle<tinytc_inst_t> {\n";

    os << "std::int32_t num_operands = 0;\n"
       << "std::int32_t num_results = 0;\n";
    std::int32_t num_static_operands = 0, num_static_results = 0, num_child_regions = 0;
    walk_up<walk_order::post_order, inst>(in, [&](inst *in) {
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
auto in = unique_handle{{tinytc_inst::create(IK::{2}, layout, loc)}};
[[maybe_unused]] std::int32_t ret_no = 0;
[[maybe_unused]] std::int32_t op_no = 0;
)CXXT",
        num_child_regions, in->class_name(), in->kind_name());

    walk_up<walk_order::post_order, inst>(in, [&os](inst *in) {
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
    walk_up<walk_order::post_order, inst>(in, [&os](inst *in) {
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
    walk_up<walk_order::post_order, inst>(in, [&os](inst *in) {
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
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&os](inst *in) {
            os << std::format("case IK::IK_{0}: return \"{0}\";\n", in->name());
        });
    }
    os << "default: break;\n"
          "}\nreturn \"unknown\";\n"
          "}\n\n";

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(
            i.get(), [&os](inst *in) { generate_inst_create(os, in); });
    }
}

void generate_inst_hpp(std::ostream &os, objects const &obj) {
    os << "enum class IK {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(
            i.get(), [&os](inst *in) { os << in->kind_name() << ",\n"; },
            [&os](inst *in) {
                if (in->has_children()) {
                    os << in->kind_name(true) << ",\n";
                }
            }

        );
    }
    os << "};\n\n";
    os << "auto to_string(IK ik) -> char const*;\n\n";

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(),
                                               [&os](inst *in) { generate_inst_class(os, in); });
    }
}

void generate_inst_kind_cpp(std::ostream &os, objects const &obj) {
    os << "auto tinytc_inst::kind() -> inst_execution_kind {\n"
          "switch (type_id()) {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&os](inst *in) {
            char const *kind = "mixed";
            if (in->is_set(inst_flag::collective) && !in->is_set(inst_flag::spmd)) {
                kind = "collective";
            } else if (!in->is_set(inst_flag::collective) && in->is_set(inst_flag::spmd)) {
                kind = "spmd";
            }
            os << std::format("case IK::IK_{0}: return inst_execution_kind::{1};\n", in->name(),
                              kind);
        });
    }
    os << "default: break;\n"
          "};\n"
          "throw internal_compiler_error();\n"
          "}\n\n";
}

auto needs_context_param(type *ty) -> bool {
    bool needs_ctx = true;
    walk_up<walk_order::post_order, type>(ty, [&](type *ty) {
        for (auto &p : ty->props()) {
            if (const auto *btype = std::get_if<builtin_type>(&p.type); btype) {
                if (*btype == builtin_type::type_t) {
                    needs_ctx = false;
                }
            }
        }
    });
    return needs_ctx;
}

void generate_type_class(std::ostream &os, type *ty) {
    type *parent = ty->parent();
    const auto parent_name = parent ? parent->class_name() : type_base_parent;
    os << std::format(R"CXXT(
class {0} : public {1} {{
public:
)CXXT",
                      ty->class_name(), parent_name);

    // classof function
    os << "inline static bool classof(tinytc_type const& t) {\n";
    if (ty->has_children()) {
        os << std::format("return TK::{} <= t.type_id() && t.type_id() < TK::{};\n",
                          ty->kind_name(), ty->kind_name(true));
    } else {
        os << std::format("return TK::{} == t.type_id();\n", ty->kind_name());
    }
    os << "}\n";

    // property access
    for (auto &p : ty->props()) {
        auto type = p.quantity == quantifier::many
                        ? std::format("array_view<{}>", to_cxx_type(p.type))
                        : to_cxx_type(p.type);
        os << std::format("inline auto {0}() const -> {1} {{ return {0}_; }}\n", p.name, type);
    }
    os << "\n";

    for (auto &code : ty->cxx()) {
        os << code << "\n";
    }

    // get function
    auto oss = std::ostringstream{};
    generate_type_cxx_params(oss, ty);
    auto params = std::move(oss).str();
    if (!ty->has_children()) {
        os << std::format("static auto get({}) -> tinytc_type_t;\n\n", params);
    }
    os << "protected:\n";
    char const *extra_arg = ty->has_children() ? "TK tid, " : "";
    os << std::format("{}({}{});\n", ty->class_name(), extra_arg, params);
    if (!ty->has_children()) {
        os << "friend class compiler_context_cache;\n\n";
    }

    os << "private:\n";
    // Properties struct
    for (auto &p : ty->props()) {
        const auto type = p.quantity == quantifier::many
                              ? std::format("std::vector<{}>", to_cxx_type(p.type))
                              : to_cxx_type(p.type);
        os << std::format("{} {}_;\n", type, p.name);
    }
    os << "};\n";
}

void generate_type_cpp(std::ostream &os, objects const &obj) {
    os << "auto to_string(TK tk) -> char const* {\n"
          "switch (tk) {\n";
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(t.get(), [&os](type *ty) {
            os << std::format("case TK::TK_{0}: return \"{0}\";\n", ty->name());
        });
    }
    os << "default: break;\n"
          "}\nreturn \"unknown\";\n"
          "}\n\n";
}

void generate_type_hpp(std::ostream &os, objects const &obj) {
    os << "enum class TK {\n";
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(
            t.get(), [&os](type *ty) { os << ty->kind_name() << ",\n"; },
            [&os](type *ty) {
                if (ty->has_children()) {
                    os << ty->kind_name(true) << ",\n";
                }
            }

        );
    }
    os << "};\n\n";
    os << "auto to_string(TK tk) -> char const*;\n\n";

    for (auto &o : obj.types()) {
        walk_down<walk_order::pre_order, type>(o.get(),
                                               [&os](type *ty) { generate_type_class(os, ty); });
    }
}

void generate_forward_hpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&os](inst *in) {
            os << std::format("class {0}; // IWYU pragma: export\n", in->class_name());
        });
    }
    os << "\n";
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(t.get(), [&os](type *ty) {
            os << std::format("class {0}; // IWYU pragma: export\n", ty->class_name());
        });
    }
    os << "\n";
}

void generate_visit_hpp(std::ostream &os, objects const &obj) {
    if (!obj.insts().empty()) {
        os << "template <typename Visitor> auto visit(Visitor && visitor, tinytc_inst &in) {\n";
        os << "switch(in.type_id()) {\n";
        for (auto &i : obj.insts()) {
            walk_down<walk_order::pre_order, inst, true>(i.get(), [&os](inst *in) {
                os << std::format("case IK::{}: {{ return visitor({}{{&in}}); }}\n",
                                  in->kind_name(), in->class_name());
            });
        }
        os << "default: break;\n}\nthrow status::internal_compiler_error;\n}\n";
    }

    if (!obj.types().empty()) {
        os << "template <typename Visitor> auto visit(Visitor && visitor, tinytc_type &ty) {\n";
        os << "switch(ty.type_id()) {\n";
        for (auto &t : obj.types()) {
            walk_down<walk_order::pre_order, type, true>(t.get(), [&os](type *ty) {
                os << std::format("case TK::{}: {{ return visitor(*static_cast<{}*>(&ty)); }}\n",
                                  ty->kind_name(), ty->class_name());
            });
        }
        os << "default: break;\n}\nthrow status::internal_compiler_error;\n}\n";
    }
}

} // namespace mochi

