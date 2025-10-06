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
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

using tinytc::overloaded;

namespace mochi {

static char const *inst_base_parent = "inst_view";
static char const *type_base_parent = "tinytc_type";

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
    return "unknown";
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
    return "unknown";
}

void generate_c_type(stream_writer &sw, cxx_type const &ty) {
    std::visit(overloaded{[&](builtin_type const &ty) { sw(to_c_type(ty)); },
                          [&](enum_ *const &ty) { sw("tinytc_%s_t", ty->name()); },
                          [&](std::string const &ty) { sw(ty); }},
               ty);
}
void generate_cxx_type(stream_writer &sw, cxx_type const &ty) {
    std::visit(overloaded{[&](builtin_type const &ty) { sw(to_cxx_type(ty)); },
                          [&](enum_ *const &ty) { sw(ty->name()); },
                          [&](std::string const &ty) { sw(ty); }},
               ty);
}
void generate_cxx_to_c_cast(stream_writer &sw, quantifier q, cxx_type const &ty, char const *name) {
    std::visit(overloaded{[&](builtin_type const &) {
                              if (q == quantifier::many) {
                                  sw("%s.size(), ", name);
                                  sw("%s.data()", name);
                              } else {
                                  sw(name);
                              }
                          },
                          [&](enum_ *const &ty) {
                              if (q == quantifier::many) {
                                  sw("reinterpret_cast<const %s*>(%s.data())", to_c_type(ty), name);
                              } else {
                                  sw("static_cast<%s>(%s)", to_c_type(ty), name);
                              }
                          },
                          [&](std::string const &) { sw(name); }},
               ty);
}
void generate_c_to_cxx_cast(stream_writer &sw, quantifier q, cxx_type const &ty, char const *name) {
    std::visit(overloaded{[&](builtin_type const &) {
                              if (q == quantifier::many) {
                                  sw("array_view{%s, %s_size}", name, name);
                              } else {
                                  sw(name);
                              }
                          },
                          [&](enum_ *const &ty) {
                              if (q == quantifier::many) {
                                  sw("array_view{%s, %s_size}", name, name);
                              } else {
                                  sw("enum_cast<%s>(%s)", to_cxx_type(ty), name);
                              }
                          },
                          [&](std::string const &) { sw(name); }},
               ty);
}

void generate_docstring(stream_writer &sw, std::string const &doc) {
    auto docstream = std::istringstream(doc);
    std::string docline;
    while (std::getline(docstream, docline)) {
        sw(" * %s\n", docline);
    }
}

void generate_inst_params(
    inst *root_in,
    std::function<void(quantifier, cxx_type const &, char const *, char const *, bool)>
        format_arg) {
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        for (auto &p : in->props()) {
            if (!p.private_) {
                format_arg(p.quantity, p.type, p.name.c_str(), p.doc.c_str(), false);
            }
        }
    });
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        for (auto &o : in->ops()) {
            format_arg(o.quantity, builtin_type::value_t, o.name.c_str(), o.doc.c_str(), false);
        }
    });
    walk_up<walk_order::post_order, inst>(root_in, [&](inst *in) {
        auto r = in->rets().begin();
        auto end = in->rets().end();
        for (; r != end; ++r) {
            format_arg(r->quantity, builtin_type::type_t, r->name.c_str(), r->doc.c_str(),
                       in == root_in && r + 1 == end);
        }
    });
}
void generate_inst_c_params(stream_writer &sw, inst *in) {
    sw("tinytc_inst_t *instr, ");
    generate_inst_params(
        in, [&sw](quantifier q, cxx_type const &ty, char const *name, char const *, bool) {
            const auto type = to_c_type(ty);
            if (q == quantifier::many) {
                sw("size_t %s_size, const %s *%s, ", name, type, name);
            } else {
                sw("%s %s, ", type, name);
            }
        });
    sw("const tinytc_location_t *loc");
}
void generate_inst_cxx_params(stream_writer &sw, inst *in) {
    generate_inst_params(
        in, [&sw](quantifier q, cxx_type const &ty, char const *name, char const *, bool) {
            const auto type = to_cxx_type(ty);
            char const *fmt = q == quantifier::many ? "array_view<%s> %s, " : "%s %s, ";
            sw(fmt, type, name);
        });
    sw("location const& loc");
}

void generate_type_params(
    type *root_ty,
    std::function<void(quantifier, cxx_type const &, char const *, char const *, bool)>
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
                format_arg(p->quantity, p->type, p->name.c_str(), p->doc.c_str(),
                           ty == root_ty && p + 1 == end);
            }
        }
    });
}
void generate_type_c_params(stream_writer &sw, type *ty) {
    sw("tinytc_type_t *ty_, ");
    generate_type_params(
        ty, [&sw](quantifier q, cxx_type const &ty, char const *name, char const *, bool is_last) {
            const auto type = to_c_type(ty);
            if (q == quantifier::many) {
                sw("size_t %s_size, const %s *%s", name, type, name);
            } else {
                sw("%s %s", type, name);
            }
            if (!is_last) {
                sw(", ");
            }
        });
}
void generate_type_cxx_params(stream_writer &sw, type *ty) {
    generate_type_params(
        ty, [&](quantifier q, cxx_type const &ty, char const *name, char const *, bool is_last) {
            const auto type = to_cxx_type(ty);
            if (q == quantifier::many) {
                sw("array_view<%s> %s", type, name);
            } else {
                sw("%s %s", type, name);
            }
            if (!is_last) {
                sw(", ");
            }
        });
}

void generate_api_builder_cpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&sw](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                const auto class_name = in->class_name();
                sw("tinytc_status_t tinytc_%s_create(", class_name);
                generate_inst_c_params(sw, in);
                sw(") {\nif (instr == nullptr) {");
                sw("return tinytc_status_invalid_arguments; }\nreturn "
                   "exception_to_status_code([&] "
                   "{\n");
                sw("*instr = %s::create(", class_name);
                generate_inst_params(in, [&sw](quantifier q, cxx_type const &ty, char const *name,
                                               char const *, bool) {
                    generate_c_to_cxx_cast(sw, q, ty, name);
                    sw(", ");
                });
                sw("get_optional(loc)).release();\n});\n}\n\n");
            }
        });
    }

    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&sw](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                const auto class_name = ty->class_name();
                sw("tinytc_status_t tinytc_%s_get(", class_name);
                generate_type_c_params(sw, ty);
                sw(") {\nif (ty_ == nullptr) {");
                sw("return tinytc_status_invalid_arguments; }\nreturn "
                   "exception_to_status_code([&] "
                   "{\n");
                sw("*ty_ = %s::get(", class_name);
                generate_type_params(ty, [&sw](quantifier q, cxx_type const &ty, char const *name,
                                               char const *, bool is_last) {
                    generate_c_to_cxx_cast(sw, q, ty, name);
                    if (!is_last) {
                        sw(", ");
                    }
                });
                sw(");\n});\n}\n\n");
            }
        });
    }
}
void generate_api_builder_h(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    const auto param_doc = [&sw](quantifier q, cxx_type const &, char const *name, char const *doc,
                                 bool) {
        if (q == quantifier::many) {
            sw(" * @param %s_size [in] array size of %s\n"
               " * @param %s [in][range(0, %s_size)] %s; may be "
               "nullptr if %s_size is 0\n",
               name, name, name, name, doc, name);
        } else if (q == quantifier::optional) {
            sw(" * @param %s [in][optional] %s; can be nullptr\n", name, doc);
        } else {
            sw(" * @param %s [in] %s\n", name, doc);
        }
    };
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&sw, &param_doc](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                sw("/**\n");
                generate_docstring(sw, in->doc());
                sw(" *\n");
                sw(" * @param instr [out] pointer to the inst object created\n");
                generate_inst_params(in, param_doc);
                sw(" * @param loc [in][optional] Source code location; can be nullptr\n *\n");
                sw(" * @return tinytc_status_success on success and error otherwise\n */\n");
                sw("TINYTC_EXPORT tinytc_status_t tinytc_%s_create(", in->class_name());
                generate_inst_c_params(sw, in);
                sw(");\n\n");
            }
        });
    }
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&sw, &param_doc](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                sw("/**\n");
                generate_docstring(sw, ty->doc());
                sw(" *\n");
                sw(" * @param ty_ [out] pointer to the type object created\n");
                generate_type_params(ty, param_doc);
                sw(" * @return tinytc_status_success on success and error otherwise\n */\n");
                sw("TINYTC_EXPORT tinytc_status_t tinytc_%s_get(", ty->class_name());
                generate_type_c_params(sw, ty);
                sw(");\n\n");
            }
        });
    }
}
void generate_api_builder_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    auto const param_doc = [&sw](quantifier q, cxx_type const &, char const *name, char const *doc,
                                 bool) {
        if (q == quantifier::optional || q == quantifier::many) {
            sw(" * @param %s %s; can be {}\n", name, doc);
        } else {
            sw(" * @param %s %s\n", name, doc);
        }
    };
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(), [&sw, &param_doc](inst *in) {
            if (!in->has_children() && !in->is_set(inst_flag::skip_builder)) {
                auto class_name = in->class_name();
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

                sw("//! creator specialization for %s\n"
                   "template<> struct creator<%s> {\n"
                   "//! Maximum number of results of %s instruction\n"
                   "constexpr static std::int32_t max_returned_values = %d;\n",
                   class_name, class_name, class_name, ret_count);
                sw("/**\n");
                generate_docstring(sw, in->doc());
                sw(" *\n");
                generate_inst_params(in, param_doc);
                sw(" * @param loc Source code location; can be {}\n *\n");
                sw(" * @return Instruction\n */\n");
                sw("inline auto operator()(");
                generate_inst_cxx_params(sw, in);
                sw("= {}) -> unique_handle<tinytc_inst_t> {\n");
                sw("tinytc_inst_t instr;\n");
                sw("CHECK_STATUS_LOC(tinytc_%s_create(&instr, \n", class_name);
                generate_inst_params(in, [&sw](quantifier q, cxx_type const &ty, char const *name,
                                               char const *, bool) {
                    generate_cxx_to_c_cast(sw, q, ty, name);
                    sw(", ");
                });
                sw("&loc), loc);\nreturn unique_handle{instr};\n}\n};\n\n");
            }
        });
    }
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(t.get(), [&sw, &param_doc](type *ty) {
            if (!ty->has_children() && !ty->is_set(inst_flag::skip_builder)) {
                auto class_name = ty->class_name();
                sw("//! getter specialization for %s\n"
                   "template<> struct getter<%s> {\n",
                   class_name, class_name);
                sw("/**\n");
                generate_docstring(sw, ty->doc());
                sw(" *\n");
                generate_type_params(ty, param_doc);
                sw(" * @return Instruction\n */\n");
                sw("inline auto operator()(");
                generate_type_cxx_params(sw, ty);
                sw(") -> tinytc_type_t {\n");
                sw("tinytc_type_t ty_;\n");
                sw("CHECK_STATUS(tinytc_%s_get(&ty_, \n", class_name);
                generate_type_params(ty, [&sw](quantifier q, cxx_type const &ty, char const *name,
                                               char const *, bool is_last) {
                    generate_cxx_to_c_cast(sw, q, ty, name);
                    if (!is_last) {
                        sw(", ");
                    }
                });
                sw("));\nreturn ty_;\n}\n};\n\n");
            }
        });
    }
}

void generate_class_list_yaml(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("enum:\n");
    for (auto &o : obj.enums()) {
        sw("- %s\n", o->name());
    }
    sw("type:\n");
    for (auto &o : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(o.get(), [&](type *ty) {
            if (!ty->is_set(inst_flag::skip_builder)) {
                sw("- %s\n", ty->name());
            }
        });
    }
    sw("inst:\n");
    for (auto &o : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(o.get(), [&](inst *in) {
            if (!in->is_set(inst_flag::skip_builder)) {
                sw("- %s\n", in->name());
            }
        });
    }
}

void generate_enum_cpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    for (auto &e : obj.enums()) {
        auto const &name = e->name();
        sw("char const* tinytc_%s_to_string(tinytc_%s_t val) {", name, name);
        sw("switch (val) {\n");
        for (auto &c : e->cases()) {
            sw("case tinytc_%s_%s: return \"%s\";\n", name, c.name,
               e->is_set(enum_flag::doc_to_string) ? c.doc : c.name);
        }
        sw("}\nreturn \"unknown\";\n");
        sw("}\n\n");
    }
}
void generate_enum_h(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    for (auto &e : obj.enums()) {
        sw("/**\n");
        generate_docstring(sw, e->doc());
        sw(" */\ntypedef enum {");
        int i = 0;
        for (auto &c : e->cases()) {
            sw("tinytc_%s_%s = %d, ///< %s\n", e->name(), c.name, c.value, c.doc);
            ++i;
        }
        sw("} tinytc_%s_t;\n", e->name());
        std::string uname = e->name();
        std::transform(uname.begin(), uname.end(), uname.begin(),
                       [](auto c) { return std::toupper(c); });
        sw("#define TINYTC_ENUM_NUM_%s %d\n", uname, i);
        auto const &name = e->name();
        sw("//! Convert %s to string\nTINYTC_EXPORT char const* "
           "tinytc_%s_to_string(tinytc_%s_t val);\n\n",
           name, name, name);
    }
}
void generate_enum_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    for (auto &e : obj.enums()) {
        sw("/**\n");
        generate_docstring(sw, e->doc());
        sw(" */\nenum class %s {", e->name());
        for (auto &c : e->cases()) {
            sw("%s = %d, ///< %s\n", c.name, c.value, c.doc);
        }
        sw("};\n");
        auto const &name = e->name();
        sw("//! Convert %s to string\ninline auto to_string(%s val) -> char "
           "const* { return ::tinytc_%s_to_string(static_cast<tinytc_%s_t>(val)); }\n\n",
           name, name, name, name);
    }
}

void generate_inst_class(stream_writer &sw, inst *in) {
    inst *parent = in->parent();
    const auto parent_name = parent ? parent->class_name() : inst_base_parent;
    const auto class_name = in->class_name();
    sw(R"CXXT(
class %s : public %s {
public:
    using %s::%s;
    struct alignas(8) properties : %s::properties {
)CXXT",
       class_name, parent_name, parent_name, parent_name, parent_name);

    // Properties struct
    for (auto &o : in->ops()) {
        if (o.has_offset_property) {
            sw("std::int32_t %s;\n", o.offset_name());
        }
    }
    for (auto &p : in->props()) {
        const auto type = p.quantity == quantifier::many
                              ? mochi::format("std::vector<%s>", to_cxx_type(p.type))
                              : to_cxx_type(p.type);
        sw("%s %s;\n", type, p.name);
    }
    sw("};\n");

    // classof function
    sw("inline static bool classof(tinytc_inst const& i) {\n");
    if (in->has_children()) {
        sw("return IK::%s <= i.type_id() && i.type_id() < IK::%s;\n", in->kind_name(),
           in->kind_name(true));
    } else {
        sw("return IK::%s == i.type_id();\n", in->kind_name());
    }
    sw("}\n");

    // create function
    if (!in->has_children()) {
        sw("static auto create(");
        generate_inst_cxx_params(sw, in);
        sw(") -> unique_handle<tinytc_inst_t>;\n\n");
    }
    sw("\n");

    // property access
    sw("inline auto props() -> properties& { return "
       "*static_cast<properties*>(get().props()); }\n");
    for (auto &p : in->props()) {
        auto type = p.quantity == quantifier::many
                        ? mochi::format("array_view<%s>", to_cxx_type(p.type))
                        : to_cxx_type(p.type);
        sw("inline auto %s() -> %s { return props().%s; }\n", p.name, type, p.name);
        sw("inline void %s(%s val) { props().%s = val; }\n", p.name, type, p.name);
    }
    sw("\n");

    // named operand access
    std::int32_t op_no = 0;
    for (auto it = in->ops().begin(); it != in->ops().end(); ++it) {
        auto offset = it->has_offset_property ? mochi::format("props().%s", it->offset_name())
                                              : std::to_string(op_no);
        auto peek = it + 1;
        auto next_offset =
            peek == in->ops().end()
                ? "get().num_operands()"
                : (peek->has_offset_property ? mochi::format("props().%s", peek->offset_name())
                                             : std::to_string(op_no + 1));
        switch (it->quantity) {
        case quantifier::optional:
            sw("inline auto has_%s() -> bool { return %s < %s; }\n", it->name, offset, next_offset);
            [[fallthrough]];
        case quantifier::single:
            sw("inline auto %s() -> tinytc_value & { return get().op(%s); }\n", it->name, offset);
            break;
        case quantifier::many:
            sw("inline auto %s() -> op_range { return {get().op_begin() + %s, "
               "get().op_begin() + %s}; }\n",
               it->name, offset, next_offset);
            break;
        }
        ++op_no;
    }
    sw("\n");

    // region access
    std::int32_t reg_no = 0;
    if (parent) {
        walk_up<walk_order::post_order, inst>(parent,
                                              [&reg_no](inst *in) { reg_no += in->regs().size(); });
    }
    for (auto &r : in->regs()) {
        sw("inline auto %s() -> tinytc_region& { return get().child_region(%d); }\n", r.name,
           reg_no++);
    }
    sw("\n");

    // result access
    std::int32_t ret_no = 0;
    if (parent) {
        walk_up<walk_order::post_order, inst>(parent,
                                              [&ret_no](inst *in) { ret_no += in->rets().size(); });
    }
    for (auto &r : in->rets()) {
        if (r.quantity == quantifier::many) {
            sw("inline auto %s() -> result_range { return {get().result_begin() "
               "+ %d, get().result_end()}; }\n",
               r.name, ret_no);
        } else {
            sw("inline auto %s() -> tinytc_value& { return get().result(%d); }\n", r.name, ret_no);
        }
        ++ret_no;
    }
    sw("\n");

    for (auto &code : in->cxx()) {
        sw("%s\n", code);
    }

    sw("void setup_and_check(); // throws compilation_error on invalid IR\n");

    sw("};\n");
    sw("static_assert(alignof(%s::properties) == alignof(tinytc_inst));\n"
       "static_assert(sizeof(%s::properties) <= "
       "std::numeric_limits<std::uint32_t>::max());\n\n",
       class_name, class_name);
}

void generate_inst_create(stream_writer &sw, inst *in) {
    const auto class_name = in->class_name();
    sw("auto %s::create(", class_name);
    generate_inst_cxx_params(sw, in);
    sw(") -> unique_handle<tinytc_inst_t> {\n");

    sw("std::int32_t num_operands = 0;\n"
       "std::int32_t num_results = 0;\n");
    std::int32_t num_static_operands = 0, num_static_results = 0, num_child_regions = 0;
    walk_up<walk_order::post_order, inst>(in, [&](inst *in) {
        for (auto &o : in->ops()) {
            switch (o.quantity) {
            case quantifier::single:
                ++num_static_operands;
                break;
            case quantifier::optional:
                sw("safe_increase(num_operands, %s ? 1 : 0);\n", o.name);
                break;
            case quantifier::many:
                sw("safe_increase(num_operands, %s.size());\n", o.name);
                break;
            }
        }
        num_child_regions += in->regs().size();
        for (auto &r : in->rets()) {
            if (r.quantity == quantifier::many) {
                sw("safe_increase(num_results, %s.size());\n", r.name);
            } else {
                ++num_static_results;
            }
        }
    });
    if (num_static_operands) {
        sw("safe_increase(num_operands, %d);\n", num_static_operands);
    }
    if (num_static_results) {
        sw("safe_increase(num_results, %d);\n", num_static_results);
    }

    const auto kind_name = in->kind_name();
    sw(
        R"CXXT(auto layout = inst_layout{
    num_results,
    num_operands,
    sizeof(%s::properties),
    %d,
};
auto in = unique_handle{tinytc_inst::create(IK::%s, layout, loc)};
[[maybe_unused]] std::int32_t ret_no = 0;
[[maybe_unused]] std::int32_t op_no = 0;
)CXXT",
        class_name, num_child_regions, kind_name);

    walk_up<walk_order::post_order, inst>(in, [&sw](inst *in) {
        for (auto &r : in->rets()) {
            if (r.quantity == quantifier::many) {
                sw(R"CXXT(for (auto &r : %s) {
    in->result(ret_no++, r);
}
)CXXT",
                   r.name);
            } else {
                sw("in->result(ret_no++, %s);\n", r.name);
            }
        }
    });
    walk_up<walk_order::post_order, inst>(in, [&sw](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                sw("std::int32_t %s = op_no;\n", o.offset_name());
            }
            switch (o.quantity) {
            case quantifier::single:
                sw("in->op(op_no++, %s);\n", o.name);
                break;
            case quantifier::optional:
                sw("if (%s) { in->op(op_no++, %s); }", o.name, o.name);
                break;
            case quantifier::many:
                sw("for (auto& o_ : %s) { in->op(op_no++, o_); }", o.name);
                break;
            }
        }
    });
    sw("[[maybe_unused]] auto view = %s(in.get());\n", class_name);
    sw("[[maybe_unused]] %s::properties& props = view.props();\n", class_name);
    walk_up<walk_order::post_order, inst>(in, [&sw](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                auto offset_name = o.offset_name();
                sw("props.%s = %s;\n", offset_name, offset_name);
            }
        }
        for (auto &p : in->props()) {
            if (!p.private_) {
                sw("props.%s = std::move(%s);", p.name, p.name);
            }
        }
    });
    sw("\n\n");

    sw("view.setup_and_check();\n\n");
    sw("return in;\n");
    sw("}\n\n");
}

void generate_inst_cpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("auto to_string(IK ik) -> char const* {\n"
       "switch (ik) {\n");
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&sw](inst *in) {
            sw("case IK::IK_%s: return \"%s\";\n", in->name(), in->name());
        });
    }
    sw("default: break;\n"
       "}\nreturn \"unknown\";\n"
       "}\n\n");

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(
            i.get(), [&sw](inst *in) { generate_inst_create(sw, in); });
    }
}

void generate_inst_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("enum class IK {\n");
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(
            i.get(), [&sw](inst *in) { sw("%s,\n", in->kind_name()); },
            [&sw](inst *in) {
                if (in->has_children()) {
                    sw("%s,\n", in->kind_name(true));
                }
            }

        );
    }
    sw("};\n\n");
    sw("auto to_string(IK ik) -> char const*;\n\n");

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst>(i.get(),
                                               [&sw](inst *in) { generate_inst_class(sw, in); });
    }
}

void generate_inst_kind_cpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("auto tinytc_inst::kind() -> inst_execution_kind {\n"
       "switch (type_id()) {\n");
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&sw](inst *in) {
            char const *kind = "mixed";
            if (in->is_set(inst_flag::collective) && !in->is_set(inst_flag::spmd)) {
                kind = "collective";
            } else if (!in->is_set(inst_flag::collective) && in->is_set(inst_flag::spmd)) {
                kind = "spmd";
            }
            sw("case IK::IK_%s: return inst_execution_kind::%s;\n", in->name(), kind);
        });
    }
    sw("default: break;\n"
       "};\n"
       "throw internal_compiler_error();\n"
       "}\n\n");
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

void generate_type_class(stream_writer &sw, type *ty) {
    type *parent = ty->parent();
    const auto parent_name = parent ? parent->class_name() : type_base_parent;
    sw(R"CXXT(
class %s : public %s {
public:
)CXXT",
       ty->class_name(), parent_name);

    // classof function
    sw("inline static bool classof(tinytc_type const& t) {\n");
    if (ty->has_children()) {
        sw("return TK::%s <= t.type_id() && t.type_id() < TK::%s;\n", ty->kind_name(),
           ty->kind_name(true));
    } else {
        sw("return TK::%s == t.type_id();\n", ty->kind_name());
    }
    sw("}\n");

    // property access
    for (auto const &p : ty->props()) {
        auto type = p.quantity == quantifier::many
                        ? mochi::format("array_view<%s>", to_cxx_type(p.type))
                        : to_cxx_type(p.type);
        sw("inline auto %s() const -> %s { return %s_; }\n", p.name, type, p.name);
    }
    sw("\n");

    for (auto &code : ty->cxx()) {
        sw("%s\n", code);
    }

    // get function
    auto oss = std::ostringstream{};
    auto tmp_writer = stream_writer{oss};
    generate_type_cxx_params(tmp_writer, ty);
    auto params = std::move(oss).str();
    if (!ty->has_children()) {
        sw("static auto get(%s) -> tinytc_type_t;\n\n", params);
    }
    sw("protected:\n");
    char const *extra_arg = ty->has_children() ? "TK tid, " : "";
    sw("%s(%s%s);\n", ty->class_name(), extra_arg, params);
    if (!ty->has_children()) {
        sw("friend class compiler_context_cache;\n\n");
    }

    sw("private:\n");
    // Properties struct
    for (auto &p : ty->props()) {
        const auto type = p.quantity == quantifier::many
                              ? mochi::format("std::vector<%s>", to_cxx_type(p.type))
                              : to_cxx_type(p.type);
        sw("%s %s_;\n", type, p.name);
    }
    sw("};\n");
}

void generate_type_cpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("auto to_string(TK tk) -> char const* {\n"
       "switch (tk) {\n");
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(t.get(), [&sw](type *ty) {
            sw("case TK::TK_%s: return \"%s\";\n", ty->name(), ty->name());
        });
    }
    sw("default: break;\n"
       "}\nreturn \"unknown\";\n"
       "}\n\n");
}

void generate_type_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    sw("enum class TK {\n");
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type>(
            t.get(), [&sw](type *ty) { sw("%s,\n", ty->kind_name()); },
            [&sw](type *ty) {
                if (ty->has_children()) {
                    sw("%s,\n", ty->kind_name(true));
                }
            }

        );
    }
    sw("};\n\n");
    sw("auto to_string(TK tk) -> char const*;\n\n");

    for (auto &o : obj.types()) {
        walk_down<walk_order::pre_order, type>(o.get(),
                                               [&sw](type *ty) { generate_type_class(sw, ty); });
    }
}

void generate_forward_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, inst, true>(i.get(), [&sw](inst *in) {
            sw("class %s; // IWYU pragma: export\n", in->class_name());
        });
    }
    sw("\n");
    for (auto &t : obj.types()) {
        walk_down<walk_order::pre_order, type, true>(t.get(), [&sw](type *ty) {
            sw("class %s; // IWYU pragma: export\n", ty->class_name());
        });
    }
    sw("\n");
}

void generate_visit_hpp(std::ostream &os, objects const &obj) {
    auto sw = stream_writer{os};
    if (!obj.insts().empty()) {
        sw("template <typename Visitor> auto visit(Visitor && visitor, tinytc_inst &in) {\n");
        sw("switch(in.type_id()) {\n");
        for (auto &i : obj.insts()) {
            walk_down<walk_order::pre_order, inst, true>(i.get(), [&sw](inst *in) {
                sw("case IK::%s: return visitor(%s{&in});\n", in->kind_name(), in->class_name());
            });
        }
        sw("default: break;\n}\nthrow status::internal_compiler_error;\n}\n");

        sw("template <typename Visitor> void visit_noexcept(Visitor && visitor, tinytc_inst &in) "
           "noexcept {\n");
        sw("switch(in.type_id()) {\n");
        for (auto &i : obj.insts()) {
            walk_down<walk_order::pre_order, inst, true>(i.get(), [&sw](inst *in) {
                sw("case IK::%s: visitor(%s{&in}); break;\n", in->kind_name(), in->class_name());
            });
        }
        sw("default: break;\n}\n}\n");
    }

    if (!obj.types().empty()) {
        sw("template <typename Visitor> auto visit(Visitor && visitor, tinytc_type &ty) {\n");
        sw("switch(ty.type_id()) {\n");
        for (auto &t : obj.types()) {
            walk_down<walk_order::pre_order, type, true>(t.get(), [&sw](type *ty) {
                sw("case TK::%s: return visitor(*static_cast<%s*>(&ty));\n", ty->kind_name(),
                   ty->class_name());
            });
        }
        sw("default: break;\n}\nthrow status::internal_compiler_error;\n}\n");

        sw("template <typename Visitor> void visit_noexcept(Visitor && visitor, tinytc_type &ty) "
           "noexcept {\n");
        sw("switch(ty.type_id()) {\n");
        for (auto &t : obj.types()) {
            walk_down<walk_order::pre_order, type, true>(t.get(), [&sw](type *ty) {
                sw("case TK::%s: visitor(*static_cast<%s*>(&ty)); break;\n", ty->kind_name(),
                   ty->class_name());
            });
        }
        sw("default: break;\n}\n}\n");
    }
}

} // namespace mochi

