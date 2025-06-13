// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen.hpp"
#include "inst.hpp"
#include "objects.hpp"
#include "walk.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace mochi {

constexpr std::string base_parent = "inst_view";

void generate_inst_class(std::ostream &os, inst *in) {
    inst *parent = in->parent();
    os << "class " << in->class_name();
    const auto parent_name = parent ? parent->class_name() : base_parent;
    os << " : public " << parent_name << " {\npublic:\n";
    os << "using " << parent_name << "::" << parent_name << ";\n\n";

    // Properties struct
    os << "struct properties";
    if (parent) {
        os << " : " << parent->class_name() << "::properties";
    }
    os << " {\n";
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
        os << "return IK::" << in->ik_name()
           << " <= i.type_id() && i.type_id() < IK::" << in->ik_name(true) << ";\n";
    } else {
        os << "return IK::" << in->ik_name() << " == i.type_id();\n";
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
        os << "inline auto " << p.name << "() const -> " << p.cxx_type() << " { return props()."
           << p.name << "}\n";
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
            os << "inline auto has_" << it->name << "() -> bool { return " << offset << " < "
               << next_offset << "; }\n";
        case quantifier::single:
            os << "inline auto " << it->name << "() -> tinytc_value & { return in().op(" << offset
               << "); }\n";
            break;
        case quantifier::many:
            os << "inline auto " << it->name << "() -> op_range { return {op_begin() + " << offset
               << ", op_begin() + " << next_offset << "}; }\n";
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
        os << "inline auto " << r.name << "() const -> tinytc_region& { in().child_region("
           << reg_no++ << ");}\n";
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
            os << "inline auto " << r.name << "() -> result_range { return {result_begin() + "
               << ret_no++ << ", result_end()}; }\n";
        } else {
            os << "inline auto " << r.name << "() const -> tinytc_value& { in().result(" << ret_no++
               << ");}\n";
        }
    }
    os << "\n";

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
            os << p.cxx_type() << " " << p.name << ", ";
        }
    });
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            os << o.cxx_type() << " " << o.name << ", ";
        }
    });
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &r : in->rets()) {
            os << r.cxx_type() << " " << r.name << ", ";
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
                os << "safe_increase(num_operands, " << o.name << " ? 1 : 0);\n";
                break;
            case quantifier::many:
                os << "safe_increase(num_operands, " << o.name << ".size());\n";
                break;
            }
        }
        num_child_regions += in->regs().size();
        for (auto &r : in->rets()) {
            if (r.quantity == quantifier::many) {
                os << "safe_increase(num_results, " << r.name << ".size());\n";
            } else {
                ++num_static_results;
            }
        }
    });
    if (num_static_operands) {
        os << "safe_increase(num_operands, " << num_static_operands << ");\n";
    }
    if (num_static_results) {
        os << "safe_increase(num_results, " << num_static_results << ");\n";
    }

    os << "auto layout = inst_layout{num_operands, num_results, " << num_child_regions << "};\n"
       << "auto in = inst{tinytc_inst::create(IK::" << in->ik_name() << ", layout)};\n";
    os << "std::int32_t op_no = 0;\n";
    walk_up<walk_order::post_order>(in, [&os](inst *in) {
        for (auto &o : in->ops()) {
            if (o.has_offset_property) {
                os << "std::int32_t " << o.offset_name() << " = op_no;\n";
            }
            switch (o.quantity) {
            case quantifier::single:
                os << "in->op(op_no++, " << o.name << ");\n";
                break;
            case quantifier::optional:
                os << "if (" << o.name << ") {\n";
                os << "in->op(op_no++, " << o.name << ");\n";
                os << "}\n";
                break;
            case quantifier::many:
                os << "for (auto& o_ : " << o.name << ") {\n";
                os << "in->op(op_no++, o_);\n";
                os << "}\n";
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
            os << "std::move(" << p.name << "),";
        }
    });

    os << "};\n";
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

} // namespace mochi

