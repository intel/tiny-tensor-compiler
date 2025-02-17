// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/memref.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/attr_node.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdlib> // IWYU pragma: keep
#include <functional>
#include <utility>

namespace tinytc {

auto is_aligned(std::vector<std::int64_t> const &offset_gcds,
                std::vector<std::int64_t> const &stride_gcds, std::int32_t alignment) -> bool {
    if (offset_gcds.size() != stride_gcds.size()) {
        throw internal_compiler_error{};
    }

    auto const check = [&alignment](std::int64_t a, std::int64_t b) {
        return ((a * b) % alignment) == 0;
    };

    auto oit = offset_gcds.begin();
    auto sit = stride_gcds.begin();

    for (; oit != offset_gcds.end() && sit != stride_gcds.end(); ++oit, ++sit) {
        if (!check(*oit, *sit)) {
            return false;
        }
    }
    return true;
}

memref_info::memref_info(std::int32_t alignment, std::int32_t sty_size,
                         std::vector<std::int64_t> shape_gcd, std::vector<std::int64_t> stride_gcd)
    : alignment_(alignment), sty_size_(sty_size), shape_gcd_(std::move(shape_gcd)),
      stride_gcd_(std::move(stride_gcd)) {}

auto memref_info::compute_max_alignment(std::vector<std::int64_t> const &offset_gcds) const
    -> std::int32_t {
    auto max_alignment = alignment();

    for (std::int32_t alignment = max_alignment; alignment > sty_size(); alignment /= 2) {
        if (is_aligned(offset_gcds, stride_gcd(), alignment / sty_size())) {
            return alignment;
        }
    }
    return sty_size();
}

auto memref_analysis_result::get_if(::const_tinytc_value_t a) const -> memref_info const * {
    if (auto it = memref_info_.find(a); it != memref_info_.end()) {
        return &it->second;
    }
    return nullptr;
}
auto memref_analysis_result::get_if(::tinytc_value const &a) const -> memref_info const * {
    return get_if(&a);
}
void memref_analysis_result::set(::tinytc_value const &a, memref_info g) {
    memref_info_[&a] = std::move(g);
}

class memref_helper {
  public:
    memref_helper(std::int32_t default_alignment) : default_alignment_{default_alignment} {}

    void operator()(inst_node const &in);
    void operator()(alloca_inst const &in);
    void operator()(expand_inst const &in);
    void operator()(fuse_inst const &in);
    void operator()(load_inst const &in);
    void operator()(subview_inst const &in);

    void set_from_attributes(function_node const &fn);

    auto get_result() && { return std::move(mr_); }

  private:
    std::int32_t default_alignment_;
    memref_analysis_result mr_;
};

void memref_helper::operator()(inst_node const &) {}
void memref_helper::operator()(alloca_inst const &in) {
    if (in.stack_ptr() >= 0) {
        const auto rt = get_memref_type(in.result(0).ty());
        std::int32_t i = rt->element_alignment();
        while (i < default_alignment_) {
            const auto i2 = 2 * i;
            if (in.stack_ptr() % i2 != 0) {
                break;
            }
            i = i2;
        }
        // alloca shape/stride must be static, therefore we can set shape_gcd/stride_gcd to
        // shape/stride
        mr_.set(in.result(0), memref_info(i, size(rt->element_ty()), rt->shape(), rt->stride()));
    }
}
void memref_helper::operator()(expand_inst const &in) {
    // @todo
    // known_alignment(in.result(0), known_alignment(in.operand()));
}
void memref_helper::operator()(fuse_inst const &in) {
    // @todo
    // known_alignment(in.result(0), known_alignment(in.operand()));
}
//}
void memref_helper::operator()(load_inst const &in) {
    // @todo
    // if (isa<group_data_type>(*in.operand().ty())) {
    // known_alignment(in.result(0), known_alignment(in.operand()));
    // return;
    //}
    // const auto align = compute_max_alignment(in.operand(), get_gcds(in.index_list()));
    // if (in.align() == 0 && align != 0) {
    // in.align(align);
    //}
}
void memref_helper::operator()(subview_inst const &in) {
    // @todo
    // auto offset_gcds = std::vector<std::int64_t>{};
    // std::int64_t dim = in.static_offsets().size();
    // offset_gcds.reserve(dim);
    // std::int64_t ri = 0;
    // for (std::int64_t i = 0; i < dim; ++i) {
    // auto stat_off = in.static_offsets()[i];
    // if (is_dynamic_value(stat_off)) {
    // offset_gcds.emplace_back(gcd_.get(in.offsets()[ri++]));
    //++ri;
    //} else {
    // offset_gcds.emplace_back(stat_off);
    //}
    //}

    // const auto align = compute_max_alignment(in.operand(), offset_gcds);
    // if (align != 0) {
    // known_alignment(in.result(0), align);
    //}
}
// void memref_helper::operator()(store_inst &in) {
// const auto align = compute_max_alignment(in.operand(), get_int_constants(in.index_list()));
// if (in.align() == 0 && align != 0) {
// in.align(align);
//}
//}

// auto memref_helper::known_alignment(value_node const &val) const -> std::int32_t {
// if (auto it = known_alignment_.find(&val); it != known_alignment_.end()) {
// return it->second;
//}
// return 0;
//}
// void memref_helper::known_alignment(value_node const &val, std::int32_t align) {
// if (align != 0) {
// known_alignment_[&val] = align;
//}
//}

void memref_helper::set_from_attributes(function_node const &fn) {
    auto known_memref_info = [&](memref_data_type const *mr, tinytc_attr_t dict) -> memref_info {
        const std::int64_t alignment = [&]() -> std::int64_t {
            if (auto alignment_attr = get_attr(dict, "alignment"); alignment_attr) {
                auto ia = dyn_cast<integer_attr>(alignment_attr);
                if (ia) {
                    return ia->value();
                }
                throw status::ir_expected_integer_attribute;
            }
            return default_alignment_;
        }();

        auto shape_gcd = [&]() -> std::vector<std::int64_t> {
            if (auto shape_attr = get_attr(dict, "shape_gcd"); shape_attr) {
                return get_array_attr_as<std::int64_t>(shape_attr);
            }
            return std::vector<std::int64_t>{};
        }();
        if (shape_gcd.size() < static_cast<std::size_t>(mr->dim())) {
            std::size_t i = shape_gcd.size();
            shape_gcd.resize(mr->dim());
            for (; i < shape_gcd.size(); ++i) {
                const auto s = mr->shape(i);
                shape_gcd[i] = !is_dynamic_value(s) ? s : 1;
            }
        }

        auto stride_gcd = [&]() -> std::vector<std::int64_t> {
            if (auto stride_attr = get_attr(dict, "stride_gcd"); stride_attr) {
                return get_array_attr_as<std::int64_t>(stride_attr);
            }
            return std::vector<std::int64_t>{};
        }();
        if (stride_gcd.size() < static_cast<std::size_t>(mr->dim())) {
            std::size_t i = stride_gcd.size();
            stride_gcd.resize(mr->dim());
            for (; i < stride_gcd.size(); ++i) {
                const auto s = mr->stride(i);
                stride_gcd[i] = !is_dynamic_value(s) ? s : 1;
            }
        }

        return memref_info(alignment, size(mr->element_ty()), std::move(shape_gcd),
                           std::move(stride_gcd));
    };
    for (std::int32_t arg_no = 0; arg_no < fn.num_params(); ++arg_no) {
        auto ty = fn.params()[arg_no].ty();
        if (auto g = dyn_cast<group_data_type>(ty); g) {
            ty = g->ty();
        }
        if (auto mr = dyn_cast<memref_data_type>(ty); mr) {
            mr_.set(fn.params()[arg_no], known_memref_info(mr, fn.param_attr(arg_no)));
        }
    }
}

auto memref_analysis::run_on_function(function_node const &fn) -> memref_analysis_result {
    auto visitor = memref_helper{default_alignment_};

    visitor.set_from_attributes(fn);
    walk<walk_order::pre_order>(fn, [&visitor](inst_node const &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
