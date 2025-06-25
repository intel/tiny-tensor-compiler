// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/attr_node.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/iterator.hpp"
#include "util/overloaded.hpp"
#include "util/visit.hpp"

#include <cstdlib> // IWYU pragma: keep
#include <functional>
#include <numeric>
#include <utility>
#include <variant>

namespace tinytc {

memref_info::memref_info(std::int64_t offset_gcd, std::vector<std::int64_t> shape_gcd,
                         std::vector<std::int64_t> stride_gcd)
    : offset_gcd_(offset_gcd), shape_gcd_(std::move(shape_gcd)),
      stride_gcd_(std::move(stride_gcd)) {}

auto gcd_analysis_result::get(::const_tinytc_value_t a) const -> std::int64_t {
    const auto g = get_if(a);
    return g ? *g : 1;
}
auto gcd_analysis_result::get(::tinytc_value const &a) const -> std::int64_t { return get(&a); }
auto gcd_analysis_result::get_if(::const_tinytc_value_t a) const -> std::optional<std::int64_t> {
    if (auto it = gcd_.find(a); it != gcd_.end()) {
        return it->second;
    }
    return std::nullopt;
}
auto gcd_analysis_result::get_if(::tinytc_value const &a) const -> std::optional<std::int64_t> {
    return get_if(&a);
}
void gcd_analysis_result::set(::tinytc_value const &a, std::int64_t g) { gcd_[&a] = g; }

auto gcd_analysis_result::get_memref_if(::const_tinytc_value_t a) const -> memref_info const * {
    if (auto it = memref_info_.find(a); it != memref_info_.end()) {
        return &it->second;
    }
    return nullptr;
}
auto gcd_analysis_result::get_memref_if(::tinytc_value const &a) const -> memref_info const * {
    return get_memref_if(&a);
}
void gcd_analysis_result::set_memref(::tinytc_value const &a, memref_info g) {
    memref_info_[&a] = std::move(g);
}

class gcd_helper {
  public:
    inline gcd_helper(std::int32_t default_alignment) : default_alignment_{default_alignment} {}

    void operator()(inst_view in);
    void operator()(alloca_inst in);
    void operator()(arith_inst in);
    void operator()(arith_unary_inst in);
    void operator()(cast_inst in);
    void operator()(constant_inst in);
    void operator()(expand_inst in);
    void operator()(for_inst in);
    void operator()(fuse_inst in);
    void operator()(load_inst in);
    void operator()(size_inst in);
    void operator()(subgroup_broadcast_inst in);
    void operator()(subview_inst in);

    void set_from_attributes(function_node &fn);

    auto get_result() && { return std::move(gcd_); }

  private:
    std::int32_t default_alignment_;
    gcd_analysis_result gcd_;
};

void gcd_helper::operator()(inst_view) {}
void gcd_helper::operator()(alloca_inst in) {
    if (in.stack_ptr() >= 0) {
        const auto rt = get_memref_type(in.result().ty());
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
        gcd_.set_memref(in.result(),
                        memref_info(i / size(rt->element_ty()), rt->shape(), rt->stride()));
    }
}
void gcd_helper::operator()(arith_inst in) {
    auto compute_gcd = [&]() -> std::optional<std::int64_t> {
        const auto ga = gcd_.get(in.a());
        const auto gb = gcd_.get(in.b());
        switch (in.operation()) {
        case arithmetic::add:
            return std::gcd(ga, gb);
        case arithmetic::mul:
            return ga * gb;
        case arithmetic::div: {
            return ga % gb == 0 ? ga / gb : 1;
        }
        default:
            break;
        }
        return std::nullopt;
    };
    auto g = compute_gcd();
    if (g) {
        gcd_.set(in.result(), *g);
    }
}
void gcd_helper::operator()(arith_unary_inst in) {
    auto compute_gcd = [&]() -> std::optional<std::int64_t> {
        switch (in.operation()) {
        case arithmetic_unary::abs:
        case arithmetic_unary::not_:
            return gcd_.get(in.a());
        default:
            break;
        }
        return std::nullopt;
    };
    auto g = compute_gcd();
    if (g) {
        gcd_.set(in.result(), *g);
    }
}
void gcd_helper::operator()(cast_inst in) {
    auto g = gcd_.get_if(in.a());
    if (g) {
        gcd_.set(in.result(), *g);
    }
}
void gcd_helper::operator()(constant_inst in) {
    if (std::holds_alternative<std::int64_t>(in.value())) {
        gcd_.set(in.result(), std::abs(std::get<std::int64_t>(in.value())));
    }
}
void gcd_helper::operator()(expand_inst in) {
    if (auto mi = gcd_.get_memref_if(in.operand()); mi) {
        const auto mt = get_memref_type(in.operand());
        const auto offset_gcd = mi->offset_gcd();
        auto shape_gcd = std::vector<std::int64_t>{};
        auto stride_gcd = std::vector<std::int64_t>{};

        auto static_shape = in.static_expand_shape();
        auto dyn_shape = in.expand_shape();

        shape_gcd.reserve(mt->dim() + static_shape.size() - 1);
        stride_gcd.reserve(mt->dim() + static_shape.size() - 1);

        for (std::int64_t i = 0; i < in.expanded_mode(); ++i) {
            shape_gcd.push_back(mi->shape_gcd(i));
            stride_gcd.push_back(mi->stride_gcd(i));
        }

        auto get_shape = [&, j = std::size_t{0}](std::int64_t s) mutable {
            if (is_dynamic_value(s)) {
                return gcd_.get(dyn_shape[j++]);
            }
            return s;
        };
        stride_gcd.push_back(mi->stride_gcd(in.expanded_mode()));
        shape_gcd.push_back(get_shape(static_shape[0]));
        for (std::size_t j = 1; j < static_shape.size(); ++j) {
            stride_gcd.push_back(stride_gcd.back() * shape_gcd.back());
            shape_gcd.push_back(get_shape(static_shape[j]));
        }

        for (std::int64_t i = in.expanded_mode() + 1; i < mt->dim(); ++i) {
            shape_gcd.push_back(mi->shape_gcd(i));
            stride_gcd.push_back(mi->stride_gcd(i));
        }

        gcd_.set_memref(in.result(), memref_info(offset_gcd, shape_gcd, stride_gcd));
    }
}
void gcd_helper::operator()(for_inst in) {
    if (in.has_step()) {
        auto g = std::gcd(gcd_.get(in.from()), gcd_.get(in.step()));
        gcd_.set(in.loop_var(), g);
    }
}
void gcd_helper::operator()(fuse_inst in) {
    if (auto mi = gcd_.get_memref_if(in.operand()); mi) {
        const auto mt = get_memref_type(in.operand());
        const auto offset_gcd = mi->offset_gcd();
        auto shape_gcd = std::vector<std::int64_t>{};
        auto stride_gcd = std::vector<std::int64_t>{};

        shape_gcd.reserve(mt->dim());
        stride_gcd.reserve(mt->dim());

        std::int64_t i = 0;
        for (; i < in.from(); ++i) {
            shape_gcd.push_back(mi->shape_gcd(i));
            stride_gcd.push_back(mi->stride_gcd(i));
        }
        std::int64_t prod = mi->shape_gcd(i++);
        for (; i <= in.to(); ++i) {
            prod *= mi->shape_gcd(i);
        }
        shape_gcd.push_back(prod);
        stride_gcd.push_back(mi->stride_gcd(in.from()));
        for (i = in.to() + 1; i < mt->dim(); ++i) {
            shape_gcd.push_back(mi->shape_gcd(i));
            stride_gcd.push_back(mi->stride_gcd(i));
        }

        gcd_.set_memref(in.result(), memref_info(offset_gcd, shape_gcd, stride_gcd));
    }
}
void gcd_helper::operator()(load_inst in) {
    if (auto mi = gcd_.get_memref_if(in.operand());
        mi && isa<group_data_type>(*in.operand().ty())) {
        gcd_.set_memref(in.result(), *mi);
    }
}
void gcd_helper::operator()(size_inst in) {
    const auto size =
        visit(overloaded{[&](group_data_type &g) -> std::int64_t {
                             return !is_dynamic_value(g.size()) ? g.size() : 1;
                         },
                         [&](memref_data_type &m) -> std::int64_t {
                             const auto s_i = m.shape(in.mode());
                             if (is_dynamic_value(s_i)) {
                                 if (auto mi = gcd_.get_memref_if(in.operand()); mi) {
                                     return mi->shape_gcd(in.mode());
                                 }
                                 return 1;
                             }
                             return s_i;
                         },
                         [&](auto &) -> std::int64_t {
                             throw compilation_error(in.loc(), status::ir_expected_memref_or_group);
                         }},
              *in.operand().ty());

    gcd_.set(in.result(), size);
}
void gcd_helper::operator()(subgroup_broadcast_inst in) {
    auto g = gcd_.get_if(in.a());
    if (g) {
        gcd_.set(in.result(), *g);
    }
}
void gcd_helper::operator()(subview_inst in) {
    if (auto mi = gcd_.get_memref_if(in.operand()); mi) {
        const auto mt = get_memref_type(in.operand());

        auto shape_gcd = std::vector<std::int64_t>{};
        auto stride_gcd = std::vector<std::int64_t>{};

        shape_gcd.reserve(mt->dim());
        stride_gcd.reserve(mt->dim());
        auto dyn_offsets = in.offsets();
        auto dyn_sizes = in.sizes();
        std::int64_t offset_gcd = mi->offset_gcd();
        for (std::int64_t i = 0, joffset = 0, jsize = 0; i < mt->dim(); ++i) {
            const std::int64_t offset = in.static_offsets()[i];

            auto const get_offset = [&]() -> std::int64_t {
                if (is_dynamic_value(offset)) {
                    return gcd_.get(dyn_offsets[joffset++]);
                }
                return offset;
            };
            offset_gcd = std::gcd(offset_gcd, get_offset() * mi->stride_gcd(i));

            const std::int64_t size = in.static_sizes()[i];
            if (size > 0 || is_dynamic_value(size)) {
                auto const get_size = [&]() -> std::int64_t {
                    if (is_dynamic_value(size)) {
                        return gcd_.get(dyn_sizes[jsize++]);
                    }
                    return size;
                };
                shape_gcd.emplace_back(get_size());
                stride_gcd.emplace_back(mi->stride_gcd(i));
            }
        }

        gcd_.set_memref(in.result(), memref_info(offset_gcd, shape_gcd, stride_gcd));
    }
}

void gcd_helper::set_from_attributes(function_node &fn) {
    auto known_memref_info = [&](memref_data_type *mr, tinytc_attr_t dict) -> memref_info {
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
        auto const dim = static_cast<std::size_t>(mr->dim());
        if (shape_gcd.size() > dim) {
            shape_gcd.resize(dim);
        } else if (shape_gcd.size() < dim) {
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
        if (stride_gcd.size() > dim) {
            stride_gcd.resize(dim);
        } else if (stride_gcd.size() < dim) {
            std::size_t i = stride_gcd.size();
            stride_gcd.resize(mr->dim());
            for (; i < stride_gcd.size(); ++i) {
                const auto s = mr->stride(i);
                stride_gcd[i] = !is_dynamic_value(s) ? s : 1;
            }
        }

        return memref_info(alignment / size(mr->element_ty()), std::move(shape_gcd),
                           std::move(stride_gcd));
    };
    for (std::int32_t arg_no = 0; arg_no < fn.num_params(); ++arg_no) {
        auto ty = fn.params()[arg_no].ty();
        if (auto g = dyn_cast<group_data_type>(ty); g) {
            ty = g->ty();
        }
        if (auto mr = dyn_cast<memref_data_type>(ty); mr) {
            gcd_.set_memref(fn.params()[arg_no], known_memref_info(mr, fn.param_attr(arg_no)));
        }
    }
}

auto gcd_analysis::run_on_function(function_node &fn) -> gcd_analysis_result {
    auto visitor = gcd_helper{default_alignment_};
    visitor.set_from_attributes(fn);

    walk<walk_order::pre_order>(fn, [&visitor](inst_node &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
