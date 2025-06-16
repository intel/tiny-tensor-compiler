// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen.hpp"
#include "inst.hpp"
#include "objects.hpp"
#include "walk.hpp"

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace mochi {

constexpr std::string base_parent = "inst_view";

void generate_inst_class(std::ostream &os, inst *in) {
    inst *parent = in->parent();
    const auto parent_name = parent ? parent->class_name() : base_parent;
    os << std::format(R"CXXT(
class {0} : public {1} {{
public:
    using {1}::{1};
    struct properties : {1}::properties {{
)CXXT",
                      in->class_name(), parent_name);

    // Properties struct
    for (auto &o : in->ops()) {
        if (o.has_offset_property) {
            os << "std::int32_t " << o.offset_name() << ";\n";
        }
    }
    for (auto &p : in->props()) {
        os << p.cxx_storage_type() << " " << p.name << ";\n";
    }
    os << "};\n";

    // classof function
    os << "inline static bool classof(inst_node const& i) {\n";
    if (in->has_children()) {
        os << std::format("return IK::{} <= i.type_id() && i.type_id() < IK::{};\n", in->ik_name(),
                          in->ik_name(true));
    } else {
        os << std::format("return IK::{} == i.type_id();\n", in->ik_name());
    }
    os << "}\n";

    // create function
    if (!in->has_children()) {
        os << "static ";
        generate_inst_create_prototype(os, in);
        os << ";\n\n";
    }
    os << "\n";

    // property access
    os << "inline auto props() -> properties const& { return "
          "*static_cast<properties*>(in().props()); }\n";
    for (auto &p : in->props()) {
        os << std::format("inline auto {0}() const -> {1} {{ return props().{0}; }}\n", p.name,
                          p.cxx_type());
    }
    os << "\n";

    // named operand access
    std::int32_t op_no = 0;
    for (auto it = in->ops().begin(); it != in->ops().end(); ++it) {
        auto offset =
            it->has_offset_property ? "props()." + it->offset_name() : std::to_string(op_no);
        auto peek = it + 1;
        auto next_offset = peek == in->ops().end()
                               ? "in().num_operands()"
                               : (peek->has_offset_property ? "props()." + peek->offset_name()
                                                            : std::to_string(op_no + 1));
        switch (it->quantity) {
        case quantifier::optional:
            os << std::format("inline auto has_{}() -> bool {{ return {} < {}; }}\n", it->name,
                              offset, next_offset);
        case quantifier::single:
            os << std::format(R"CXXT(inline auto {0}() -> tinytc_value & {{ return in().op({1}); }}
inline auto {0}() const -> tinytc_value const& {{ return in().op({1}); }}
)CXXT",
                              it->name, offset);
            break;
        case quantifier::many:
            os << std::format(
                R"CXXT(inline auto {0}() -> op_range {{ {{op_begin() + {1}, op_end() + {2}}}; }}
inline auto {0}() const -> const_op_range {{ {{op_begin() + {1}, op_end() + {2}}}; }}
)CXXT",
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
            R"CXXT(inline auto {0}() -> tinytc_region& {{ in().child_region({1}); }}
inline auto {0}() const -> tinytc_region const& {{ in().child_region({1}); }}
)CXXT",
            r.name, reg_no++);
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
            os << std::format(
                R"CXXT(inline auto {0}() -> result_range {{ return {{result_begin() + {1}, result_end()}}; }}
inline auto {0}() const -> const_result_range {{ return {{result_begin() + {1}, result_end()}}; }}
)CXXT",
                r.name, ret_no);
        } else {
            os << std::format(R"CXXT(inline auto {0}() -> tinytc_value& {{ in().result({1}); }}
inline auto {0}() const -> tinytc_value const& {{ in().result({1}); }}
)CXXT",
                              r.name, ret_no);
        }
        ++ret_no;
    }
    os << "\n";

    for (auto &code : in->cxx()) {
        os << code << "\n";
    }

    os << "protected:\n";
    os << "void check(); // throws compilation_error on invalid IR\n";
    if (reg_no > 0) {
        os << "void setup_regions();\n";
    }

    os << "};\n\n";
}

void generate_inst_create_prototype(std::ostream &os, inst *in, bool insert_class_name) {
    os << "auto ";
    if (insert_class_name) {
        os << in->class_name() << "::";
    }
    os << "create(";

    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &p : in->props()) {
            os << std::format("{} {},", p.cxx_type(), p.name);
        }
    });
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            os << std::format("{} {},", o.cxx_type(), o.name);
        }
    });
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &r : in->rets()) {
            os << std::format("{} {},", r.cxx_type(), r.name);
        }
    });
    os << "location const& lc) -> tinytc_inst_t";
}

void generate_inst_create(std::ostream &os, inst *in) {
    generate_inst_create_prototype(os, in, true);
    os << " {\n";

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

    os << std::format(R"CXXT(auto layout = inst_layout{{num_operands, num_results, {}}};
auto in = inst{{tinytc_inst::create(IK::{}, layout)}};
std::int32_t op_no = 0;
)CXXT",
                      num_child_regions, in->ik_name());
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
    os << "*static_cast<properties*>(in->props()) = properties{\n";
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                os << o.offset_name() << ",";
            }
        }
        for (auto &p : in->props()) {
            os << std::format("std::move({}),", p.name);
        }
    });

    os << "};\n\n";

    if (num_child_regions > 0) {
        os << "in->setup_regions();\n\n";
    }

    os << std::format("{}(in.get()).check();\n\n", in->class_name());

    os << "return in.release();\n";

    os << "}\n\n";
}

void generate_inst_header(std::ostream &os, objects const &obj) {
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

    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) { generate_inst_class(os, in); });
    }
}

void generate_inst_cpp(std::ostream &os, objects const &obj) {
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order, true>(i.get(),
                                               [&os](inst *in) { generate_inst_create(os, in); });
    }
}

void generate_inst_visit_header(std::ostream &os, objects const &obj) {
    os << "template <typename Visitor> auto visit(Visitor && visitor, tinytc_inst &in) {\n";
    os << "switch(in.type_id()) {\n";
    for (auto &i : obj.insts()) {
        walk_down<walk_order::pre_order>(i.get(), [&os](inst *in) {
            if (!in->has_children()) {
                os << std::format("case IK::{}: return visitor({}{{&in}});\n", in->ik_name(),
                                  in->class_name());
            }
        });
    }
    os << "default: break;\n}\nthrow status::internal_compiler_error;\n}\n";
}

} // namespace mochi

