// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DOPE_VECTOR_20241213_HPP
#define DOPE_VECTOR_20241213_HPP

#include <cstdint>
#include <vector>

namespace tinytc::spv {

class spv_inst;

class dope_vector {
  public:
    dope_vector() = default;
    dope_vector(spv_inst *ty, std::vector<std::int64_t> static_shape,
                std::vector<std::int64_t> static_stride, spv_inst *size_ty = nullptr,
                std::int64_t static_size = 0, spv_inst *offset_ty = nullptr,
                std::int64_t static_offset = 0);

    inline auto dim() const -> std::int64_t { return static_shape_.size(); }
    inline auto ty() const -> spv_inst * { return ty_; }
    inline auto static_shape(std::int64_t i) const -> std::int64_t { return static_shape_[i]; }
    inline auto static_stride(std::int64_t i) const -> std::int64_t { return static_stride_[i]; }
    inline auto shape(std::int64_t i) const -> spv_inst * { return shape_[i]; }
    inline auto stride(std::int64_t i) const -> spv_inst * { return stride_[i]; }
    inline void shape(std::int64_t i, spv_inst *s) { shape_[i] = s; }
    inline void stride(std::int64_t i, spv_inst *s) { stride_[i] = s; }

    inline auto size_ty() const -> spv_inst * { return size_ty_; }
    inline auto static_size() const -> std::int64_t { return static_size_; }
    inline auto size() -> spv_inst * { return size_; }
    inline void size(spv_inst *size) { size_ = size; }

    inline auto offset_ty() const -> spv_inst * { return offset_ty_; }
    inline auto static_offset() const -> std::int64_t { return static_offset_; }
    inline auto offset() -> spv_inst * { return offset_; }
    inline void offset(spv_inst *offset) { offset_ = offset; }

    auto num_dynamic() const -> std::int64_t;

  private:
    spv_inst *ty_ = nullptr;
    std::vector<std::int64_t> static_shape_, static_stride_;
    std::vector<spv_inst *> shape_, stride_;
    spv_inst *size_ty_ = nullptr, *offset_ty_ = nullptr;
    std::int64_t static_size_, static_offset_;
    spv_inst *size_ = nullptr;
    spv_inst *offset_ = nullptr;
};

} // namespace tinytc::spv

#endif // DOPE_VECTOR_20241213_HPP
