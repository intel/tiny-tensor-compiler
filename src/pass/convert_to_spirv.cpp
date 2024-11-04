// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/convert_to_spirv.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "spv/instructions.hpp"
#include "support/visit.hpp"

#include <unordered_map>

namespace tinytc {

class spirv_converter {
  public:
    inline spirv_converter(spv::mod &mod, tinytc_compiler_context_t ctx) : mod_(&mod), ctx_(ctx) {}

    auto operator()(data_type_node const &ty) -> spv::spv_inst *;

    // Instruction nodes
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);

    void run_on_program(program_node const &p);

  private:
    auto declare(value_node const &v, spv::spv_inst *in);
    auto val(value_node const &v) -> spv::spv_inst *;
    void run_on_region(region_node const &fn);
    void run_on_function(function_node const &fn);
    template <spv::section S, typename T, typename... Args> auto add_to(Args &&...args) -> T * {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...).release();
        mod_->insts(S).push_back(ptr);
        return ptr;
    }

    template <typename T, typename... Args> auto add(Args &&...args) -> T * {
        return add_to<spv::section::function, T>(std::forward<Args>(args)...);
    }

    spv::mod *mod_;
    tinytc_compiler_context_t ctx_;
    std::unordered_map<const_tinytc_data_type_t, spv::spv_inst *> spv_tys_;
    std::unordered_map<const_tinytc_value_t, spv::spv_inst *> vals_;
};

auto spirv_converter::declare(value_node const &v, spv::spv_inst *in) { vals_[&v] = in; }
auto spirv_converter::val(value_node const &v) -> spv::spv_inst * {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        return it->second;
    }
    throw status::internal_compiler_error;
}

auto spirv_converter::operator()(data_type_node const &ty) -> spv::spv_inst * {
    auto it = spv_tys_.find(&ty);
    if (it == spv_tys_.end()) {
        auto spv_ty = visit(
            overloaded{
                [&](void_data_type const &) -> spv::spv_inst * {
                    return add_to<spv::section::type, spv::OpTypeVoid>();
                },
                [&](scalar_data_type const &ty) -> spv::spv_inst * {
                    switch (ty.ty()) {
                    case scalar_type::i1:
                        return add_to<spv::section::type, spv::OpTypeBool>();
                    case scalar_type::i8:
                        add_to<spv::section::capability, spv::OpCapability>(spv::Capability::Int8);
                        return add_to<spv::section::type, spv::OpTypeInt>(8, 1);
                    case scalar_type::i16:
                        add_to<spv::section::capability, spv::OpCapability>(spv::Capability::Int16);
                        return add_to<spv::section::type, spv::OpTypeInt>(16, 1);
                    case scalar_type::i32:
                    case scalar_type::i64:
                    case scalar_type::index:
                        return add_to<spv::section::type, spv::OpTypeInt>(size(ty.ty()) * 8, 1);
                    case scalar_type::f32:
                    case scalar_type::f64:
                        return add_to<spv::section::type, spv::OpTypeFloat>(size(ty.ty()) * 8,
                                                                            std::nullopt);
                    case scalar_type::c32:
                    case scalar_type::c64: {
                        auto float_ty = add_to<spv::section::type, spv::OpTypeFloat>(
                            size(ty.ty()) * 8 / 2, std::nullopt);
                        return add_to<spv::section::type, spv::OpTypeVector>(float_ty, 2);
                    }
                    }
                    throw status::internal_compiler_error;
                },
                [&](coopmatrix_data_type const &ty) -> spv::spv_inst * {
                    // @todo
                    throw status::internal_compiler_error;
                    return nullptr;
                },
                [](auto const &) -> spv::spv_inst * {
                    // @todo
                    throw status::internal_compiler_error;
                    return nullptr;
                }},
            ty);
        spv_tys_[&ty] = spv_ty;
        return spv_ty;
    }
    return it->second;
}

void spirv_converter::operator()(inst_node const &) {
    // @todo
    throw status::internal_compiler_error;
}

void spirv_converter::operator()(arith_inst const &in) {
    auto const make_boolean = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                                  spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::and_:
            return add<spv::OpLogicalAnd>(ty, a, b);
        case arithmetic::or_:
            return add<spv::OpLogicalOr>(ty, a, b);
        case arithmetic::xor_:
            return add<spv::OpLogicalNotEqual>(ty, a, b);
        default:
            break;
        }
        throw status::ir_i1_unsupported;
    };
    auto const make_int = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                              spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::add:
            return add<spv::OpIAdd>(ty, a, b);
        case arithmetic::sub:
            return add<spv::OpISub>(ty, a, b);
        case arithmetic::mul:
            return add<spv::OpIMul>(ty, a, b);
        case arithmetic::div:
            return add<spv::OpSDiv>(ty, a, b);
        case arithmetic::rem:
            return add<spv::OpSRem>(ty, a, b);
        case arithmetic::shl:
            return add<spv::OpShiftLeftLogical>(ty, a, b);
        case arithmetic::shr:
            return add<spv::OpShiftRightArithmetic>(ty, a, b);
        case arithmetic::and_:
            return add<spv::OpBitwiseAnd>(ty, a, b);
        case arithmetic::or_:
            return add<spv::OpBitwiseOr>(ty, a, b);
        case arithmetic::xor_:
            return add<spv::OpBitwiseXor>(ty, a, b);
        }
        throw status::internal_compiler_error;
    };
    auto const make_float_complex = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                                        spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::add:
            return add<spv::OpFAdd>(ty, a, b);
        case arithmetic::sub:
            return add<spv::OpFSub>(ty, a, b);
        case arithmetic::mul:
            return add<spv::OpFMul>(ty, a, b);
        case arithmetic::div:
            return add<spv::OpFDiv>(ty, a, b);
        case arithmetic::rem:
            return add<spv::OpFRem>(ty, a, b);
        default:
            break;
        }
        throw status::ir_fp_unsupported;
    };
    auto const make = [&](scalar_type sty, arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                          spv::spv_inst *b) -> spv::spv_inst * {
        switch (sty) {
        case scalar_type::i1:
            return make_boolean(op, ty, a, b);
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return make_int(op, ty, a, b);
        case scalar_type::f32:
        case scalar_type::f64:
        case scalar_type::c32:
        case scalar_type::c64:
            return make_float_complex(op, ty, a, b);
        }
        throw status::internal_compiler_error;
    };

    auto ty = visit(*this, *in.result(0).ty());
    auto av = val(in.a());
    auto bv = val(in.b());

    if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        make(st->ty(), in.operation(), ty, av, bv);
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        // auto clinst = std::vector<clir::stmt>{};
        // auto const len = ct->length(core_cfg_.subgroup_size);
        // clinst.reserve(len + 1);
        // clinst.emplace_back(declaration(std::move(lhs_ty), lhs));
        // const auto sty = ct->component_ty();
        // for (std::int64_t i = 0; i < len; ++i) {
        // auto op = make(a.operation(), av[i], bv[i], sty);
        // clinst.emplace_back(expression_statement(assignment(lhs[i], std::move(op))));
        //}
        // return clinst;
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void spirv_converter::run_on_region(region_node const &reg) {
    add<spv::OpLabel>();
    for (auto const &i : reg) {
        visit(*this, i);
    }
}

void spirv_converter::run_on_function(function_node const &fn) {
    // Function type
    auto void_ty = visit(*this, *void_data_type::get(ctx_));
    auto params = std::vector<spv::spv_inst *>{};
    params.reserve(fn.num_params());
    for (auto const &p : fn.params()) {
        params.push_back(visit(*this, *p.ty()));
    }
    auto fun_ty = add<spv::OpTypeFunction>(void_ty, std::move(params));

    // Function
    auto fun = add<spv::OpFunction>(void_ty, spv::FunctionControl::None, fun_ty);
    for (auto const &p : fn.params()) {
        declare(p, add<spv::OpFunctionParameter>(visit(*this, *p.ty())));
    }
    run_on_region(fn.body());
    add<spv::OpFunctionEnd>();

    // Entry point
    add_to<spv::section::entry_point, spv::OpEntryPoint>(
        spv::ExecutionModel::Kernel, fun, std::string{fn.name()}, std::vector<spv::spv_inst *>{});

    // Execution mode
    auto const work_group_size = fn.work_group_size();
    add_to<spv::section::execution_mode, spv::OpExecutionMode>(
        fun, spv::ExecutionMode::LocalSize,
        spv::ExecutionModeAttr{
            std::array<std::int32_t, 3u>{work_group_size[0], work_group_size[1], 1}});
    add_to<spv::section::execution_mode, spv::OpExecutionMode>(
        fun, spv::ExecutionMode::SubgroupSize, spv::ExecutionModeAttr{fn.subgroup_size()});

    // Function decoration
    auto linkage_decoration =
        spv::DecorationAttr{std::make_pair(std::string{fn.name()}, spv::LinkageType::Export)};
    add_to<spv::section::decoration, spv::OpDecorate>(fun, spv::Decoration::LinkageAttributes,
                                                      std::move(linkage_decoration));
}

void spirv_converter::run_on_program(program_node const &p) {
    add_to<spv::section::capability, spv::OpCapability>(spv::Capability::Addresses);
    add_to<spv::section::capability, spv::OpCapability>(spv::Capability::Kernel);
    add_to<spv::section::capability, spv::OpCapability>(spv::Capability::Linkage);
    add_to<spv::section::capability, spv::OpCapability>(spv::Capability::SubgroupDispatch);
    add_to<spv::section::memory_model, spv::OpMemoryModel>(spv::AddressingModel::Physical64,
                                                           spv::MemoryModel::OpenCL);

    for (auto const &fn : p) {
        run_on_function(fn);
    }
}

convert_to_spirv_pass::convert_to_spirv_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

auto convert_to_spirv_pass::run_on_program(program_node const &p) -> std::unique_ptr<spv::mod> {
    auto m = std::make_unique<spv::mod>();

    spirv_converter(*m, p.context()).run_on_program(p);

    return m;
}

} // namespace tinytc
