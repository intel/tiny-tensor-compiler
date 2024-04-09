// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/builder.hpp"
#include "ir/node/function_node.hpp"
#include "ir/node/inst_node.hpp"
#include "ir/node/program_node.hpp"
#include "ir/node/region_node.hpp"
#include "ir/node/value_node.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/ir/value.hpp"

#include <clir/handle.hpp>

#include <algorithm>

namespace tinytc::ir {

location to_location(std::source_location const loc) {
    auto l = location{};
    l.begin.filename = loc.file_name();
    l.begin.line = loc.line();
    l.begin.column = loc.column();
    l.end = l.begin;
    ++l.end.column;
    return l;
}

std::string internal::unique_name_giver::name(std::string const &prefix) {
    if (auto nc = names_.find(prefix); nc != names_.end()) {
        std::string new_name;
        do {
            ++nc->second;
            new_name = prefix + std::to_string(nc->second);
        } while (names_.find(new_name) != names_.end());
        return new_name;
    }
    names_[prefix] = 0;
    return prefix + std::to_string(0);
}

/* region builder */
region_builder::region_builder() : reg_{region{std::make_shared<rgn>()}} {}

region region_builder::get_product() { return reg_; }

value region_builder::create_binary_op(binary_op op, value a, value b, std::string const &prefix,
                                       std::source_location const loc) {
    return insert(
        std::make_shared<binary_op_inst>(op, std::move(a), std::move(b), to_location(loc)), prefix);
}

value region_builder::create_cast(value a, scalar_type to_ty, std::string const &prefix,
                                  std::source_location const loc) {
    return insert(std::make_shared<cast_inst>(std::move(a), to_ty, to_location(loc)), prefix);
}

value region_builder::create_cmp(cmp_condition cond, value a, value b, std::string const &prefix,
                                 std::source_location const loc) {
    return insert(
        std::make_shared<compare_inst>(cond, std::move(a), std::move(b), to_location(loc)), prefix);
}

value region_builder::create_neg(value a, std::string const &prefix,
                                 std::source_location const loc) {
    return insert(std::make_shared<neg_inst>(std::move(a), to_location(loc)), prefix);
}

value region_builder::create_alloca(data_type ty, std::string const &prefix,
                                    std::source_location const loc) {
    return insert(std::make_shared<alloca_inst>(std::move(ty), to_location(loc)), prefix);
}

void region_builder::create_axpby(transpose tA, value alpha, value A, value beta, value B,
                                  bool atomic, std::source_location const loc) {
    insert(std::make_shared<axpby_inst>(tA, std::move(alpha), std::move(A), std::move(beta),
                                        std::move(B), atomic, to_location(loc)));
}

void region_builder::create_barrier(std::source_location const loc) {
    auto b = std::make_shared<barrier_inst>();
    b->loc(to_location(loc));
    insert(std::move(b));
}

value region_builder::create_expand(value arg, std::int64_t mode, std::vector<value> expand_shape,
                                    std::string const &prefix, std::source_location const loc) {
    return insert(std::make_shared<expand_inst>(std::move(arg), mode, std::move(expand_shape),
                                                to_location(loc)),
                  prefix);
}

value region_builder::create_fuse(value arg, std::int64_t from, std::int64_t to,
                                  std::string const &prefix, std::source_location const loc) {
    return insert(std::make_shared<fuse_inst>(std::move(arg), from, to, to_location(loc)), prefix);
}

value region_builder::create_load(value arg, std::vector<value> index_list,
                                  std::string const &prefix, std::source_location const loc) {
    return insert(
        std::make_shared<load_inst>(std::move(arg), std::move(index_list), to_location(loc)),
        prefix);
}

value region_builder::create_group_id(std::string const &prefix, std::source_location const loc) {
    auto g = std::make_shared<group_id_inst>();
    g->loc(to_location(loc));
    return insert(std::move(g), prefix);
}

value region_builder::create_group_size(std::string const &prefix, std::source_location const loc) {
    auto g = std::make_shared<group_size_inst>();
    g->loc(to_location(loc));
    return insert(std::move(g), prefix);
}

void region_builder::create_gemm(transpose tA, transpose tB, value alpha, value A, value B,
                                 value beta, value C, bool atomic, std::source_location const loc) {
    insert(std::make_shared<gemm_inst>(tA, tB, std::move(alpha), std::move(A), std::move(B),
                                       std::move(beta), std::move(C), atomic, to_location(loc)));
}

void region_builder::create_gemv(transpose tA, value alpha, value A, value B, value beta, value C,
                                 bool atomic, std::source_location const loc) {
    insert(std::make_shared<gemv_inst>(tA, std::move(alpha), std::move(A), std::move(B),
                                       std::move(beta), std::move(C), atomic, to_location(loc)));
}

void region_builder::create_ger(value alpha, value A, value B, value beta, value C, bool atomic,
                                std::source_location const loc) {
    insert(std::make_shared<ger_inst>(std::move(alpha), std::move(A), std::move(B), std::move(beta),
                                      std::move(C), atomic, to_location(loc)));
}

void region_builder::create_hadamard(value alpha, value A, value B, value beta, value C,
                                     bool atomic, std::source_location const loc) {
    insert(std::make_shared<hadamard_inst>(std::move(alpha), std::move(A), std::move(B),
                                           std::move(beta), std::move(C), atomic,
                                           to_location(loc)));
}

value region_builder::create_size(value arg, std::int64_t mode, std::string const &prefix,
                                  std::source_location const loc) {
    return insert(std::make_shared<size_inst>(std::move(arg), mode, to_location(loc)), prefix);
}

value region_builder::create_subview(value op, std::vector<slice> slices, std::string const &prefix,
                                     std::source_location const loc) {
    return insert(
        std::make_shared<subview_inst>(std::move(op), std::move(slices), to_location(loc)), prefix);
}

void region_builder::create_store(value val, value op, std::vector<value> index_list,
                                  std::string const &prefix, std::source_location const loc) {
    insert(std::make_shared<store_inst>(std::move(val), std::move(op), std::move(index_list),
                                        to_location(loc)),
           prefix);
}

void region_builder::create_sum(transpose tA, value alpha, value A, value beta, value B,
                                bool atomic, std::source_location const loc) {
    insert(std::make_shared<sum_inst>(tA, std::move(alpha), std::move(A), std::move(beta),
                                      std::move(B), atomic, to_location(loc)));
}

void region_builder::create_for(value loop_var, value from, value to, region body,
                                std::source_location const loc) {
    auto f = std::make_shared<for_inst>(std::move(loop_var), std::move(from), std::move(to),
                                        std::move(body), to_location(loc));
    insert(std::move(f));
}
void region_builder::create_for(value loop_var, value from, value to, value step, region body,
                                std::source_location const loc) {
    auto f = std::make_shared<for_inst>(std::move(loop_var), std::move(from), std::move(to),
                                        std::move(step), std::move(body), to_location(loc));
    insert(std::move(f));
}

void region_builder::create_foreach(value loop_var, value from, value to, region body,
                                    std::source_location const loc) {
    auto f = std::make_shared<foreach_inst>(std::move(loop_var), std::move(from), std::move(to),
                                            std::move(body), to_location(loc));
    insert(std::move(f));
}

void region_builder::create_if(value condition, region then, region otherwise,
                               std::source_location const loc) {
    auto i = std::make_shared<if_inst>(std::move(condition), std::move(then), std::move(otherwise));
    i->loc(to_location(loc));
    insert(std::move(i));
}

value region_builder::insert(std::shared_ptr<inst_node> s, std::string const &prefix) {
    auto result = s->result();
    auto r = dynamic_cast<rgn *>(reg_.get());
    r->insts().emplace_back(inst(std::move(s)));
    result->name(this->name(prefix));
    return result;
}

/* function builder */
function_builder::function_builder(std::string name)
    : proto_{func{std::make_shared<prototype>(std::move(name))}}, body_(nullptr) {}

func function_builder::get_product() {
    if (!body_.get()) {
        body_ = region(std::make_shared<rgn>());
    }
    return func(std::make_shared<function>(proto_, body_, work_group_size_, subgroup_size_));
}

value function_builder::argument(data_type ty, std::string const &prefix,
                                 std::source_location const loc) {
    auto v = value(std::move(ty), this->name(prefix));
    v->loc(to_location(loc));
    auto p = dynamic_cast<prototype *>(proto_.get());
    p->args().emplace_back(std::move(v));
    return p->args().back();
}

/* program builder */
void program_builder::add(func f) { functions_.emplace_back(std::move(f)); }

prog program_builder::get_product() { return prog(std::make_shared<program>(functions_)); }

} // namespace tinytc::ir
