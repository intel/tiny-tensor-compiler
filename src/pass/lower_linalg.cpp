// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

namespace tinytc {

auto lower_linalg_pass::get_memref_type(value_node const &v) const -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty().get());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

lower_linalg_pass::lower_linalg_pass(::tinytc_core_info const *info) : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

/* Data type nodes */
// bool lower_linalg_pass::operator()(void_data_type &) { return false; }
// bool lower_linalg_pass::operator()(group_data_type &b) { return visit(*this, *b.ty()); }
// bool lower_linalg_pass::operator()(memref_data_type &m) {
// return m.addrspace() == clir::address_space::local_t;
//}
// bool lower_linalg_pass::operator()(scalar_data_type &) { return false; }

//[> Value nodes <]
// value_node *lower_linalg_pass::operator()(float_imm &) { return nullptr; }
// value_node *lower_linalg_pass::operator()(int_imm &) { return nullptr; }
// value_node *lower_linalg_pass::operator()(val &v) {
// if (visit(*this, *v.ty())) {
// return &v;
//}
// return nullptr;
//}

/* Inst nodes */
inst lower_linalg_pass::operator()(inst_node &) { return inst{nullptr}; }

inst lower_linalg_pass::operator()(loop_inst &p) {
    visit(*this, *p.body());
    return inst{nullptr};
}

inst lower_linalg_pass::operator()(if_inst &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
    return inst{nullptr};
}

inst lower_linalg_pass::operator()(parallel_inst &p) {
    visit(*this, *p.body());
    return inst{nullptr};
}

inst lower_linalg_pass::operator()(ger_inst &g) {
    // auto at = get_memref_type(*g.A());
    // auto bt = get_memref_type(*g.B());
    auto ct = get_memref_type(*g.C());

    auto bb = region_builder{};
    auto sgid = bb.add(make_subgroup_id(g.loc()));
    auto m_tiles_imm = make_imm(tiling_.m_tiles(), g.loc());
    auto sg_n = bb.add(make_arith(arithmetic::div, sgid, m_tiles_imm, g.loc()));
    auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, m_tiles_imm, g.loc()));
    auto m = bb.add(make_subgroup_local_id(g.loc()));
    auto m_index = bb.add(make_cast(m, scalar_type::index, g.loc()));

    auto c_shape1 = is_dynamic_value(ct->shape(1)) ? bb.add(make_size(g.C(), 1, g.loc()))
                                                   : make_index(ct->shape(1), g.loc());
    auto c_shape0 = is_dynamic_value(ct->shape(0)) ? bb.add(make_size(g.C(), 0, g.loc()))
                                                   : make_index(ct->shape(0), g.loc());
    tile_loop_uniformly_new(
        bb, std::move(c_shape1), core_cfg_.subgroup_size, tiling_.n_tiles(), std::move(sg_n),
        [&](region_builder &bb, value block, value trip_count) {
            bb.for_loop(scalar_type::index, make_index(0, g.loc()), trip_count,
                        [&](region_builder &bb, value const &n) {
                            auto nn = bb.add(make_arith(arithmetic::add, block, n, g.loc()));
                            auto b = bb.add(make_load(g.B(), {nn}, g.loc()));
                            b->name("b");
                            tile_loop_by_sgs_new(
                                bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles(), sg_m,
                                [&](region_builder &bb, value const &block, bool is_remainder,
                                    value const &inner_trip_count) {
                                    auto mm = bb.add(
                                        make_arith(arithmetic::add, block, m_index, g.loc()));
                                    auto a = bb.add(make_load(g.A(), {mm}, g.loc()));
                                    a->name("a");
                                    auto ab = bb.add(make_arith(arithmetic::mul, a, b, g.loc()));
                                    bb.add(make_store(ab, g.C(), {mm, nn}, g.loc()));
                                });
                        });
        });
    return make_parallel(bb.get_product(), g.loc());

    /*auto alpha = visit(*this, *g.alpha());
    auto beta = visit(*this, *g.beta());
    auto alpha_ty = get_scalar_type(*g.alpha()->ty());
    auto beta_ty = get_scalar_type(*g.beta()->ty());

    auto A = visit(*this, *g.A());
    auto B = visit(*this, *g.B());
    auto C = visit(*this, *g.C());

    auto bb = clir::block_builder{};
    auto sg_n = bb.declare_assign(clir::generic_uint(), "sg_n",
                                  clir::get_sub_group_id() / tiling_.m_tiles());
    auto sg_m = bb.declare_assign(clir::generic_uint(), "sg_m",
                                  clir::get_sub_group_id() % tiling_.m_tiles());
    tile_loop_uniformly(
        bb, cdv.shape(1), core_cfg_.subgroup_size, tiling_.n_tiles(), std::move(sg_n),
        [&](clir::block_builder &bb, clir::expr block, clir::expr trip_count) {
            auto n = clir::var("n");
            bb.add(clir::for_loop_builder(clir::declaration_assignment(clir::generic_int(), n,
    0), n < std::move(trip_count), ++n) .body([&](clir::block_builder &bb) { auto b =
    bb.declare_assign(to_clir_ty(bt->element_ty()), "b", B + (block + n) * bdv.stride(0)); auto
    Cb = bb.declare_assign(this->operator()(*ct), "Cb", C + (block + n) * cdv.stride(1)); auto m
    = bb.declare_assign(clir::generic_uint(), "m", clir::get_sub_group_local_id());
                           tile_loop_by_sgs(
                               bb, cdv.shape(0), core_cfg_.subgroup_size, tiling_.m_tiles(),
    sg_m,
                               [&](clir::block_builder &bb, clir::expr block, bool is_remainder,
                                   clir::expr inner_trip_count) {
                                   auto const inner_loop = [&](clir::block_builder &bb) {
                                       auto a = A[(block + m) * adv.stride(0)];
                                       auto c = bb.declare_assign((*this)(*ct), "c",
                                                                  Cb + (block + m) *
    cdv.stride(0)); auto ab = bb.declare_assign( to_clir_ty(ct->element_ty()), "ab",
                                           multiply(at->element_ty(), bt->element_ty(),
                                                    std::move(a), b));
                                       const auto ab_scaled = multiply(alpha_ty,
    ct->element_ty(), alpha, std::move(ab)); store_helper(bb, g.atomic(), c, ct->element_ty(),
                                                    ct->addrspace(), std::move(ab_scaled),
    beta_ty, beta);
                                   };
                                   if (is_remainder) {
                                       bb.add(clir::if_selection_builder(
                                                  m < std::move(inner_trip_count))
                                                  .then(inner_loop)
                                                  .get_product());
                                   } else {
                                       inner_loop(bb);
                                   }
                               });
                       })
                       .get_product());
        });*/
}

/* Region nodes */
void lower_linalg_pass::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        if (auto lowered_inst = visit(*this, *s); lowered_inst) {
            s = lowered_inst;
        }
    }
}

/* Function nodes */
void lower_linalg_pass::operator()(prototype &) {}

void lower_linalg_pass::operator()(function &fn) {
    auto const subgroup_size = fn.subgroup_size();
    try {
        core_cfg_ = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    auto const work_group_size = fn.work_group_size();
    tiling_[0] = work_group_size[0] / subgroup_size;
    tiling_[1] = work_group_size[1];

    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void lower_linalg_pass::operator()(program &p) {
    for (auto &fn : p.functions()) {
        visit(*this, *fn);
    }
}

} // namespace tinytc
