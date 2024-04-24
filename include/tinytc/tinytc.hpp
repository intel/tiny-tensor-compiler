// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240403_HPP
#define TINYTC_20240403_HPP

#include "tinytc/tinytc.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

//! Convert error code to string
inline char const *error_string(status code) {
    return ::tinytc_error_string(static_cast<::tinytc_status_t>(code));
}

//! Throw exception for unsuccessful call to C-API
inline void CHECK_STATUS(tinytc_status_t code) {
    if (code != tinytc_status_success) {
        throw status{std::underlying_type_t<status>(code)};
    }
}

//! Builder exception enhanced with location
class builder_error : public std::exception {
  public:
    //! ctor; taking location and status code
    inline builder_error(status code, location const &loc) : code_(code), loc_(loc) {}
    //! Get status code
    inline auto code() const noexcept { return code_; }
    //! Get location
    inline auto loc() const noexcept -> location const & { return loc_; }
    //! Get explanatory string
    inline char const *what() const noexcept override { return error_string(code_); }

  private:
    status code_;
    location loc_;
};

//! Throw exception for unsuccessful call to C-API
inline void CHECK_STATUS_LOC(tinytc_status_t code, location const &loc) {
    if (code != tinytc_status_success) {
        throw builder_error(status{std::underlying_type_t<status>(code)}, loc);
    }
}

////////////////////////////
//////// Scalar type ///////
////////////////////////////

//! Convert scalar type to string
inline char const *to_string(scalar_type ty) {
    return ::tinytc_scalar_type_to_string(static_cast<tinytc_scalar_type_t>(ty));
}
//! Size of scalar type in bytes
inline std::size_t size(scalar_type ty) {
    return ::tinytc_scalar_type_size(static_cast<tinytc_scalar_type_t>(ty));
}

/**
 * Returns the scalar type corresponding to C++ type T
 *
 * Specializations exist for bool, (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, float, double.
 * The scalar_type is stored in the static constexpr member "value".
 */
template <typename T> struct to_scalar_type;
//! to_scalar_type specialization
template <> struct to_scalar_type<bool> {
    static constexpr scalar_type value = scalar_type::bool_; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int8_t> {
    static constexpr scalar_type value = scalar_type::i8; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int16_t> {
    static constexpr scalar_type value = scalar_type::i16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int32_t> {
    static constexpr scalar_type value = scalar_type::i32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int64_t> {
    static constexpr scalar_type value = scalar_type::i64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint8_t> {
    static constexpr scalar_type value = scalar_type::u8; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint16_t> {
    static constexpr scalar_type value = scalar_type::u16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint32_t> {
    static constexpr scalar_type value = scalar_type::u32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint64_t> {
    static constexpr scalar_type value = scalar_type::u64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<float> {
    static constexpr scalar_type value = scalar_type::f32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<double> {
    static constexpr scalar_type value = scalar_type::f64; ///< value
};
/**
 * Convenience variable for to_scalar_type.
 *
 * Example: @code scalar_type ty = to_scalar_type_v<float>; @endcode
 */
template <typename T> inline constexpr scalar_type to_scalar_type_v = to_scalar_type<T>::value;

////////////////////////////
// Shared / unique handle //
////////////////////////////

//! Wraps retain / release calls for type T
template <typename T> struct shared_handle_traits {};

//! Wraps destroy calls for type T
template <typename T> struct unique_handle_traits {};

/**
 * @brief Wraps a C handle in a reference-counted object
 *
 * @tparam T C handle type (handle type = pointer to opaque struct)
 */
template <typename T> class shared_handle {
  public:
    //! Traits shortcut
    using traits = shared_handle_traits<T>;
    //! Typedef for native C handle
    using native_type = T;

    //! Create empty (invalid) handle
    shared_handle() : obj_{nullptr} {}
    //! Create handle from C handle
    shared_handle(T obj, bool needs_retain = false) : obj_(obj) {
        if (needs_retain) {
            CHECK_STATUS(c_retain());
        }
    }
    //! Decrease reference count
    ~shared_handle() { c_release(); }
    //! Copy ctor
    shared_handle(shared_handle const &other) : obj_(other.obj_) { CHECK_STATUS(c_retain()); }
    //! Move ctor
    shared_handle(shared_handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    shared_handle &operator=(shared_handle const &other) {
        if (obj_ != other.obj_) {
            CHECK_STATUS(c_release());
            obj_ = other.obj_;
            CHECK_STATUS(c_retain());
        }
        return *this;
    }
    //! Move operator
    shared_handle &operator=(shared_handle &&other) {
        if (obj_ != other.obj_) {
            CHECK_STATUS(c_release());
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Returns C handle
    auto get() const -> T { return obj_; }
    //! Returns C handle and releases the ownership of the managed object
    auto release() -> T {
        auto tmp = obj_;
        obj_ = nullptr;
        return tmp;
    }

    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(shared_handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(shared_handle<T> const &other) const { return !(*this == other); }

  protected:
    auto c_retain() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::retain(obj_);
        }
        return tinytc_status_success;
    }
    auto c_release() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::release(obj_);
        }
        return tinytc_status_success;
    }
    T obj_;
};

/**
 * @brief Wraps a C handle in a unique_ptr-alike object
 *
 * @tparam T C handle type (handle type = pointer to opaque struct)
 */
template <typename T> class unique_handle {
  public:
    //! Traits shortcut
    using traits = unique_handle_traits<T>;
    //! Typedef for native C handle
    using native_type = T;

    //! Create empty (invalid) handle
    unique_handle() : obj_{nullptr} {}
    //! Create handle from C handle
    unique_handle(T obj) : obj_(obj) {}
    //! Destroy object
    ~unique_handle() {
        if (obj_) {
            traits::destroy(obj_);
        }
    }
    //! Copy ctor
    unique_handle(unique_handle const &other) = delete;
    //! Move ctor
    unique_handle(unique_handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    unique_handle &operator=(unique_handle const &other) = delete;
    //! Move operator
    unique_handle &operator=(unique_handle &&other) {
        obj_ = other.obj_;
        other.obj_ = nullptr;
        return *this;
    }

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Returns C handle
    auto get() const -> T { return obj_; }
    //! Returns C handle and releases the ownership of the managed object
    auto release() -> T {
        auto tmp = obj_;
        obj_ = nullptr;
        return tmp;
    }

    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(unique_handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(unique_handle<T> const &other) const { return !(*this == other); }

  protected:
    T obj_;
};

////////////////////////////
///////// Data type ////////
////////////////////////////

//! Check if mode i is dynamic ('?')
inline bool is_dynamic_value(std::int64_t i) { return i == dynamic; }

template <> struct shared_handle_traits<tinytc_data_type_t> {
    static auto retain(tinytc_data_type_t handle) -> tinytc_status_t {
        return tinytc_data_type_retain(handle);
    }
    static auto release(tinytc_data_type_t handle) -> tinytc_status_t {
        return tinytc_data_type_release(handle);
    }
};

//! Reference-counted program handle
class data_type : public shared_handle<tinytc_data_type_t> {
  public:
    using shared_handle::shared_handle;

    data_type(scalar_type type, location const &loc = {}) {
        CHECK_STATUS_LOC(
            tinytc_scalar_type_create(&obj_, static_cast<tinytc_scalar_type_t>(type), &loc), loc);
    }
};

inline data_type create_memref(scalar_type scalar_ty, std::vector<std::int64_t> const &shape,
                               std::vector<std::int64_t> const &stride = {},
                               location const &loc = {}) {
    tinytc_data_type_t mt;
    CHECK_STATUS_LOC(tinytc_memref_type_create(&mt, static_cast<tinytc_scalar_type_t>(scalar_ty),
                                               shape.size(), shape.data(), stride.size(),
                                               stride.data(), &loc),
                     loc);
    return data_type(mt);
}
inline data_type create_group(data_type const &memref_ty, location const &loc = {}) {
    tinytc_data_type_t gt;
    CHECK_STATUS_LOC(tinytc_group_type_create(&gt, memref_ty.get(), &loc), loc);
    return data_type(gt);
}

////////////////////////////
/////////// Value //////////
////////////////////////////

template <> struct shared_handle_traits<tinytc_value_t> {
    static auto retain(tinytc_value_t handle) -> tinytc_status_t {
        return tinytc_value_retain(handle);
    }
    static auto release(tinytc_value_t handle) -> tinytc_status_t {
        return tinytc_value_release(handle);
    }
};

class value : public shared_handle<tinytc_value_t> {
  public:
    using shared_handle::shared_handle;
    //! Create value with data type ty
    value(data_type const &ty, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_value_create(&obj_, ty.get(), &loc), loc);
    }
    //! Create immediate value from float
    value(float imm, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_float_imm_create(&obj_, imm, tinytc_scalar_type_f32, &loc), loc);
    }
    //! Create immediate value from double
    value(double imm, scalar_type type = scalar_type::f64, location const &loc = {}) {
        CHECK_STATUS_LOC(
            tinytc_float_imm_create(&obj_, imm, static_cast<tinytc_scalar_type_t>(type), &loc),
            loc);
    }
    //! Create immediate value from int8_t
    value(std::int8_t imm, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i8, &loc), loc);
    }
    //! Create immediate value from int16_t
    value(std::int16_t imm, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i16, &loc), loc);
    }
    //! Create immediate value from int32_t
    value(std::int32_t imm, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i32, &loc), loc);
    }
    //! Create immediate value from int64_t
    value(std::int64_t imm, scalar_type type = scalar_type::i64, location const &loc = {}) {
        CHECK_STATUS_LOC(
            tinytc_int_imm_create(&obj_, imm, static_cast<tinytc_scalar_type_t>(type), &loc), loc);
    }
    //! Create immediate value from uint32_t (index type)
    value(std::uint32_t imm, location const &loc = {}) {
        CHECK_STATUS_LOC(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_index, &loc), loc);
    }

    inline auto get_name() const -> char const * {
        char const *name;
        CHECK_STATUS(tinytc_value_get_name(obj_, &name));
        return name;
    }
    inline void name(std::string const &name) {
        CHECK_STATUS(tinytc_value_set_name(obj_, name.c_str()));
    }
};

//! Is reinterpret_cast<tinytc_value_t*>(&v) allowed, where v has type value
constexpr bool value_reinterpret_allowed =
    std::is_standard_layout_v<value> && sizeof(value) == sizeof(tinytc_value_t);

////////////////////////////
/////////// Inst ///////////
////////////////////////////

//! Convert binary op to string
inline char const *to_string(binary_op op) {
    return ::tinytc_binary_op_to_string(static_cast<::tinytc_binary_op_t>(op));
}

//! Convert cmp condition to string
inline char const *to_string(cmp_condition cond) {
    return ::tinytc_cmp_condition_to_string(static_cast<::tinytc_cmp_condition_t>(cond));
}
//! Convert transpose to string
inline char const *to_string(transpose t) {
    return ::tinytc_transpose_to_string(static_cast<tinytc_transpose_t>(t));
}

template <> struct shared_handle_traits<tinytc_inst_t> {
    static auto retain(tinytc_inst_t handle) -> tinytc_status_t {
        return tinytc_inst_retain(handle);
    }
    static auto release(tinytc_inst_t handle) -> tinytc_status_t {
        return tinytc_inst_release(handle);
    }
};

class inst : public shared_handle<tinytc_inst_t> {
  public:
    using shared_handle::shared_handle;

    inline auto get_value() const -> value {
        tinytc_value_t result;
        CHECK_STATUS(tinytc_inst_get_value(obj_, &result));
        return value(result);
    }

    inline auto get_values() const -> std::vector<value> {
        static_assert(value_reinterpret_allowed);
        uint32_t result_list_size = 0;
        CHECK_STATUS(tinytc_inst_get_values(obj_, &result_list_size, nullptr));
        auto values = std::vector<value>(result_list_size);
        tinytc_value_t *result_list = reinterpret_cast<tinytc_value_t *>(values.data());
        CHECK_STATUS(tinytc_inst_get_values(obj_, &result_list_size, result_list));
        return values;
    }
};

//! Is reinterpret_cast<tinytc_inst_t*>(&i) allowed, where i has type inst
constexpr bool inst_reinterpret_allowed =
    std::is_standard_layout_v<inst> && sizeof(inst) == sizeof(tinytc_inst_t);

////////////////////////////
////////// Region //////////
////////////////////////////

template <> struct shared_handle_traits<tinytc_region_t> {
    static auto retain(tinytc_region_t handle) -> tinytc_status_t {
        return tinytc_region_retain(handle);
    }
    static auto release(tinytc_region_t handle) -> tinytc_status_t {
        return tinytc_region_release(handle);
    }
};

class region : public shared_handle<tinytc_region_t> {
  public:
    using shared_handle::shared_handle;

    region(std::vector<inst> &instructions, location const &loc = {}) {
        static_assert(inst_reinterpret_allowed);
        if (instructions.size() > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("Instruction list too long");
        }
        CHECK_STATUS_LOC(
            tinytc_region_create(&obj_, instructions.size(),
                                 reinterpret_cast<tinytc_inst_t *>(instructions.data()), &loc),
            loc);
    }
};

////////////////////////////
/////// Instructions ///////
////////////////////////////

inline inst create_binary_op(binary_op op, value const &a, value const &b,
                             location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_binary_op_inst_create(&instr, static_cast<tinytc_binary_op_t>(op),
                                                  a.get(), b.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_make_cast(value const &a, scalar_type to_ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_cast_inst_create(&instr, a.get(), static_cast<tinytc_scalar_type_t>(to_ty), &loc),
        loc);
    return inst(instr);
}

inline inst create_cmp(cmp_condition cond, value const &a, value const &b,
                       location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cmp_inst_create(&instr, static_cast<tinytc_cmp_condition_t>(cond),
                                            a.get(), b.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_neg(value const &a, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_neg_inst_create(&instr, a.get(), &loc), loc);
    return inst(instr);
}

inline inst create_alloca(data_type const &ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_alloca_inst_create(&instr, ty.get(), &loc), loc);
    return inst(instr);
}

inline inst create_axpby(transpose tA, bool atomic, value const &alpha, value const &A,
                         value const &beta, value const &B, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_axpby_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                              alpha.get(), A.get(), beta.get(), B.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_expand(value const &a, std::int64_t mode, std::vector<value> const &expand_shape,
                          location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = expand_shape.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("expand shape too large");
    }
    tinytc_value_t *eshape =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(expand_shape.data()));
    CHECK_STATUS_LOC(tinytc_expand_inst_create(&instr, a.get(), mode, len, eshape, &loc), loc);
    return inst(instr);
}

inline inst create_fuse(value const &a, std::int64_t from, std::int64_t to,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_fuse_inst_create(&instr, a.get(), from, to, &loc), loc);
    return inst(instr);
}

inline inst create_load(value const &a, std::vector<value> const &index_list,
                        location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    tinytc_value_t *il =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(index_list.data()));
    CHECK_STATUS_LOC(tinytc_load_inst_create(&instr, a.get(), len, il, &loc), loc);
    return inst(instr);
}

inline inst create_group_id(location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_group_id_inst_create(&instr, &loc), loc);
    return inst(instr);
}

inline inst create_group_size(location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_group_size_inst_create(&instr, &loc), loc);
    return inst(instr);
}

inline inst create_gemm(transpose tA, transpose tB, bool atomic, value const &alpha, value const &A,
                        value const &B, value const &beta, value const &C,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_gemm_inst_create(&instr, static_cast<tinytc_transpose_t>(tA),
                                             static_cast<tinytc_transpose_t>(tB), atomic,
                                             alpha.get(), A.get(), B.get(), beta.get(), C.get(),
                                             &loc),
                     loc);
    return inst(instr);
}

inline inst create_gemv(transpose tA, bool atomic, value const &alpha, value const &A,
                        value const &B, value const &beta, value const &C,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_gemv_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                             alpha.get(), A.get(), B.get(), beta.get(), C.get(),
                                             &loc),
                     loc);
    return inst(instr);
}

inline inst create_ger(bool atomic, value const &alpha, value const &A, value const &B,
                       value const &beta, value const &C, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_ger_inst_create(&instr, atomic, alpha.get(), A.get(), B.get(),
                                            beta.get(), C.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_hadamard(bool atomic, value const &alpha, value const &A, value const &B,
                            value const &beta, value const &C, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_hadamard_inst_create(&instr, atomic, alpha.get(), A.get(), B.get(),
                                                 beta.get(), C.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_size(value const &a, std::int64_t mode, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_size_inst_create(&instr, a.get(), mode, &loc), loc);
    return inst(instr);
}

inline inst create_subview(value const &a, std::vector<value> const &offset_list,
                           std::vector<value> const &size_list, location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    if (offset_list.size() != size_list.size()) {
        throw std::invalid_argument("offset list must have the same length as the size list");
    }
    auto len = offset_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("slice list too long");
    }
    tinytc_value_t *ol =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(offset_list.data()));
    tinytc_value_t *sl =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(size_list.data()));
    CHECK_STATUS_LOC(tinytc_subview_inst_create(&instr, a.get(), len, ol, sl, &loc), loc);
    return inst(instr);
}

inline inst create_store(value const &val, value const &a, std::vector<value> const &index_list,
                         location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    tinytc_value_t *il =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(index_list.data()));
    CHECK_STATUS_LOC(tinytc_store_inst_create(&instr, val.get(), a.get(), len, il, &loc), loc);
    return inst(instr);
}

inline inst create_sum(transpose tA, bool atomic, value const &alpha, value const &A,
                       value const &beta, value const &B, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_sum_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                            alpha.get(), A.get(), beta.get(), B.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_for(value const &loop_var, value const &from, value const &to, value const &step,
                       region const &body, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_for_inst_create(&instr, loop_var.get(), from.get(), to.get(),
                                            step.get(), body.get(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_foreach(value const &loop_var, value const &from, value const &to,
                           region const &body, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_foreach_inst_create(&instr, loop_var.get(), from.get(), to.get(), body.get(), &loc),
        loc);
    return inst(instr);
}

inline inst create_if(value const &condition, region const &then,
                      region const &otherwise = region{},
                      std::vector<scalar_type> const &return_type_list = {},
                      location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = return_type_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("return type list too long");
    }
    auto rl_vec = std::vector<tinytc_scalar_type_t>();
    rl_vec.resize(len);
    for (auto const &rt : return_type_list) {
        rl_vec.emplace_back(static_cast<tinytc_scalar_type_t>(rt));
    }
    CHECK_STATUS_LOC(tinytc_if_inst_create(&instr, condition.get(), then.get(), otherwise.get(),
                                           len, rl_vec.data(), &loc),
                     loc);
    return inst(instr);
}

inline inst create_yield(std::vector<value> const &yield_list, location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = yield_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("slice list too long");
    }
    tinytc_value_t *yl =
        const_cast<tinytc_value_t *>(reinterpret_cast<tinytc_value_t const *>(yield_list.data()));
    CHECK_STATUS_LOC(tinytc_yield_inst_create(&instr, len, yl, &loc), loc);
    return inst(instr);
}

////////////////////////////
/////////// Func ///////////
////////////////////////////

template <> struct shared_handle_traits<tinytc_func_t> {
    static auto retain(tinytc_func_t handle) -> tinytc_status_t {
        return tinytc_func_retain(handle);
    }
    static auto release(tinytc_func_t handle) -> tinytc_status_t {
        return tinytc_func_release(handle);
    }
};

class func : public shared_handle<tinytc_func_t> {
  public:
    using shared_handle::shared_handle;
};

//! Is reinterpret_cast<tinytc_func_t*>(&f) allowed, where f has type func
constexpr bool func_reinterpret_allowed =
    std::is_standard_layout_v<func> && sizeof(func) == sizeof(tinytc_func_t);

inline func create_function_prototype(char const *name, std::vector<value> &arg_list,
                                      location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_func_t fun;
    auto len = arg_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("argument list too long");
    }
    tinytc_value_t *al = reinterpret_cast<tinytc_value_t *>(arg_list.data());
    CHECK_STATUS_LOC(tinytc_function_prototype_create(&fun, name, len, al, &loc), loc);
    return func(fun);
}

inline func create_function(func const &prototype, region const &body, location const &loc = {}) {
    tinytc_func_t fun;
    CHECK_STATUS_LOC(tinytc_function_create(&fun, prototype.get(), body.get(), &loc), loc);
    return func(fun);
}

inline void set_work_group_size(func &fun, uint32_t x, uint32_t y) {
    CHECK_STATUS(tinytc_function_set_work_group_size(fun.get(), x, y));
}

inline void set_subgroup_size(func &fun, uint32_t sgs) {
    CHECK_STATUS(tinytc_function_set_subgroup_size(fun.get(), sgs));
}

////////////////////////////
/////////// Prog ///////////
////////////////////////////

template <> struct shared_handle_traits<tinytc_prog_t> {
    static auto retain(tinytc_prog_t handle) -> tinytc_status_t {
        return tinytc_prog_retain(handle);
    }
    static auto release(tinytc_prog_t handle) -> tinytc_status_t {
        return tinytc_prog_release(handle);
    }
};

template <> struct unique_handle_traits<char *> {
    static void destroy(char *obj) { tinytc_string_destroy(obj); }
};

class prog : public shared_handle<tinytc_prog_t> {
  public:
    using shared_handle::shared_handle;

    inline prog(std::vector<func> &fun_list, location const &loc = {}) {
        static_assert(func_reinterpret_allowed);
        auto len = fun_list.size();
        if (len > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("function list too long");
        }
        tinytc_func_t *fl = reinterpret_cast<tinytc_func_t *>(fun_list.data());
        CHECK_STATUS_LOC(tinytc_program_create(&obj_, len, fl, &loc), loc);
    }

    void dump() const { CHECK_STATUS(tinytc_prog_dump(obj_)); }
    void print_to_file(char const *filename) const {
        CHECK_STATUS(tinytc_prog_print_to_file(obj_, filename));
    }
    auto print_to_string() const -> unique_handle<char *> {
        char *str;
        CHECK_STATUS(tinytc_prog_print_to_string(obj_, &str));
        return {str};
    }
};

////////////////////////////
////////// Builder /////////
////////////////////////////

//! Builder for regions
class region_builder {
  public:
    //! Returns built product
    inline auto get_product(location const &loc = {}) -> region {
        return region(instructions_, loc);
    }

    [[maybe_unused]] inline auto add(inst i, std::string const &name = "") -> value {
        auto result = i.get_value();
        if (result && name.size() > 0) {
            result.name(name);
        }
        instructions_.emplace_back(std::move(i));
        return result;
    }

    [[maybe_unused]] inline auto add_multivalued(inst i, std::string const &name = "")
        -> std::vector<value> {
        auto results = i.get_values();
        if (name.size() > 0) {
            for (auto &result : results) {
                result.name(name);
            }
        }
        instructions_.emplace_back(std::move(i));
        return results;
    }

    //! Build for-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_for(data_type const &loop_var_ty, value const &from, value const &to, F &&f,
                    std::string const &name = "", location const &loc = {}) {
        create_for<F>(std::move(loop_var_ty), std::move(from), std::move(to), nullptr,
                      std::forward<F>(f), name, loc);
    }
    //! Build for-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_for(data_type const &loop_var_ty, value const &from, value const &to,
                    value const &step, F &&f, std::string const &name = "",
                    location const &loc = {}) {
        auto loop_var = value(loop_var_ty);
        if (name.size() > 0) {
            loop_var.name(name);
        }
        auto bb = region_builder{};
        f(bb);
        add(::tinytc::create_for(std::move(loop_var), from, to, step, bb.get_product(), loc));
    }
    //! Build foreach-loop with functor f(region_builder&) -> void
    template <typename F>
    void create_foreach(data_type const &loop_var_ty, value const &from, value const &to, F &&f,
                        std::string const &name = "", location const &loc = {}) {
        auto loop_var = value(loop_var_ty);
        if (name.size() > 0) {
            loop_var.name(name);
        }
        auto bb = region_builder{};
        f(bb);
        add(::tinytc::create_foreach(std::move(loop_var), from, to, bb.get_product(), loc));
    }

    //! Build if with functor then(region_builder&) -> void
    template <typename F>
    auto create_if(value const &condition, F &&then,
                   std::vector<scalar_type> const &return_type_list = {}, location const &loc = {})
        -> std::vector<value> {
        auto bb = region_builder{};
        then(bb);
        return add_multivalued(::tinytc::create_if(std::move(condition), bb.get_product(), region{},
                                                   return_type_list, loc));
    }
    //! Build if/else with functors then(region_builder&) -> void and otherwise(region_builder&) ->
    //! void
    template <typename F, typename G>
    auto create_ifelse(value const &condition, F &&then, G &&otherwise,
                       std::vector<scalar_type> const &return_type_list = {},
                       location const &loc = {}) -> std::vector<value> {
        auto bb1 = region_builder{};
        then(bb1);
        auto bb2 = region_builder{};
        otherwise(bb2);
        return add_multivalued(::tinytc::create_if(std::move(condition), bb1.get_product(),
                                                   bb2.get_product(), return_type_list, loc));
    }

  private:
    std::vector<inst> instructions_;
};

//! Builder for functions
class function_builder {
  public:
    //! ctor; creates function \@name
    inline function_builder(std::string name) : name_(std::move(name)), body_{nullptr} {}

    //! Returns built product
    inline func get_product(location const &loc = {}) {
        auto proto = create_function_prototype(name_.c_str(), arguments_, loc);
        auto fun = create_function(proto, body_);
        if (x_ > 0 && y_ > 0) {
            set_work_group_size(fun, x_, y_);
        }
        if (sgs_ > 0) {
            set_subgroup_size(fun, sgs_);
        }
        return fun;
    }

    //! @code %name: %ty @endcode
    inline value argument(data_type const &ty, std::string const &name = "",
                          location const &loc = {}) {
        auto v = value(ty, loc);
        if (name.size() > 0) {
            v.name(name);
        }
        arguments_.emplace_back(std::move(v));
        return arguments_.back();
    }

    //! @code work_group_size(%m, %n) @endcode
    inline void work_group_size(std::uint32_t x, std::uint32_t y) {
        x_ = x;
        y_ = y;
    }
    //! @code subgroup_size(%subgroup_size) @endcode
    inline void subgroup_size(std::uint32_t subgroup_size) { sgs_ = subgroup_size; }

    //! Build function body with functor f(region_builder&) -> void
    template <typename F> void body(F &&f, location const &loc = {}) {
        auto bb = region_builder{};
        f(bb);
        body_ = bb.get_product(loc);
    }

  private:
    std::string name_;
    region body_;
    std::vector<value> arguments_;
    std::uint32_t x_ = 0, y_ = 0, sgs_ = 0;
};

//! Builder for programs
class program_builder {
  public:
    //! create function \@name with functor f(function_builder&) -> void
    template <typename F> void create(std::string name, F &&f, location const &loc = {}) {
        auto fb = function_builder(std::move(name));
        f(fb);
        add(fb.get_product(loc));
    }
    //! Add function
    inline void add(func f) { functions_.emplace_back(std::move(f)); }
    //! Returns built product
    inline prog get_product(location const &loc = {}) { return prog(functions_, loc); }

  private:
    std::vector<func> functions_;
};

////////////////////////////
//////// Device info ///////
////////////////////////////

template <> struct unique_handle_traits<tinytc_core_info_t> {
    static void destroy(tinytc_core_info_t obj) { tinytc_core_info_destroy(obj); }
};

class core_info : public unique_handle<tinytc_core_info_t> {
  public:
    using unique_handle::unique_handle;

    auto get_ip_version() -> std::uint32_t {
        std::uint32_t ip_version;
        CHECK_STATUS(tinytc_core_info_get_ip_version(obj_, &ip_version));
        return ip_version;
    }

    void get_subgroup_sizes(uint32_t *sgs_size, uint32_t const **sgs) {
        CHECK_STATUS(tinytc_core_info_get_subgroup_sizes(obj_, sgs_size, sgs));
    }

    auto get_register_size() -> std::uint32_t {
        std::uint32_t size;
        CHECK_STATUS(tinytc_core_info_get_register_size(obj_, &size));
        return size;
    }
    auto get_num_registers_per_thread() -> std::uint32_t {
        std::uint32_t num;
        CHECK_STATUS(tinytc_core_info_get_num_registers_per_thread(obj_, &num));
        return num;
    }

    void set_core_feature(core_feature_flag flag) {
        CHECK_STATUS(tinytc_core_info_set_core_feature(
            obj_, static_cast<::tinytc_core_feature_flag_t>(flag)));
    }

    void clear_core_feature(core_feature_flag flag) {
        CHECK_STATUS(tinytc_core_info_clear_core_feature(
            obj_, static_cast<::tinytc_core_feature_flag_t>(flag)));
    }
};

inline auto create_core_info_intel_from_arch(intel_gpu_architecture arch) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_intel_create_from_arch(
        &info, static_cast<tinytc_intel_gpu_architecture_t>(arch)));
    return core_info{info};
}

inline auto create_core_info_intel(std::uint32_t ip_version, std::uint32_t num_eus_per_subslice,
                                   std::uint32_t num_threads_per_eu,
                                   std::uint32_t local_memory_size, std::vector<std::uint32_t> sgs)
    -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_intel_create(&info, ip_version, num_eus_per_subslice,
                                               num_threads_per_eu, local_memory_size, sgs.size(),
                                               sgs.data()));
    return core_info{info};
}

////////////////////////////
////////// Parser //////////
////////////////////////////

inline auto parse_file(char const *filename, tinytc_source_context_t source_ctx = nullptr) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_file(&prg, filename, source_ctx));
    return prog(prg);
}
inline auto parse_stdin(tinytc_source_context_t source_ctx = nullptr) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_stdin(&prg, source_ctx));
    return prog(prg);
}
inline auto parse_string(std::string const &src, tinytc_source_context_t source_ctx = nullptr)
    -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_string(&prg, src.size(), src.c_str(), source_ctx));
    return prog(prg);
}

template <> struct unique_handle_traits<tinytc_source_context_t> {
    static void destroy(tinytc_source_context_t obj) { tinytc_source_context_destroy(obj); }
};

class source_context : public unique_handle<tinytc_source_context_t> {
  public:
    using unique_handle::unique_handle;

    source_context() { CHECK_STATUS(tinytc_source_context_create(&obj_)); }

    inline auto parse_file(const char *filename) -> prog {
        return ::tinytc::parse_file(filename, obj_);
    }
    inline auto parse_stdin() -> prog { return ::tinytc::parse_stdin(obj_); }
    inline auto parse_string(std::string const &src) -> prog {
        return ::tinytc::parse_string(src, obj_);
    }

    inline auto add_source(char const *name, char const *text) -> std::int32_t {
        std::int32_t source_id;
        CHECK_STATUS(tinytc_source_context_add_source(obj_, name, text, &source_id));
        return source_id;
    }
    inline auto get_error_log() const -> char const * {
        char const *log;
        CHECK_STATUS(tinytc_source_context_get_error_log(obj_, &log));
        return log;
    }
    inline void report_error(location const &loc, char const *what, bool append = true) {
        CHECK_STATUS(tinytc_source_context_report_error(obj_, &loc, what,
                                                        static_cast<tinytc_bool_t>(append)));
    }
};

////////////////////////////
///////// Compiler /////////
////////////////////////////

template <> struct unique_handle_traits<tinytc_source_t> {
    static void destroy(tinytc_source_t obj) { tinytc_source_destroy(obj); }
};
template <> struct shared_handle_traits<tinytc_binary_t> {
    static auto retain(tinytc_binary_t handle) -> tinytc_status_t {
        return tinytc_binary_retain(handle);
    }
    static auto release(tinytc_binary_t handle) -> tinytc_status_t {
        return tinytc_binary_release(handle);
    }
};

class source : public unique_handle<tinytc_source_t> {
  public:
    using unique_handle::unique_handle;

    inline auto get_code() -> char const * {
        char const *code = nullptr;
        CHECK_STATUS(tinytc_source_get_code(obj_, &code));
        return code;
    }
};

class binary : public shared_handle<tinytc_binary_t> {
  public:
    using shared_handle::shared_handle;

    struct raw {
        bundle_format format;
        std::uint64_t data_size;
        std::uint8_t const *data;
    };

    inline auto get_raw() -> raw {
        raw r;
        tinytc_bundle_format_t f;
        CHECK_STATUS(tinytc_binary_get_raw(obj_, &f, &r.data_size, &r.data));
        r.format = bundle_format{std::underlying_type_t<bundle_format>(f)};
        return r;
    }
    inline auto get_core_features() -> std::uint32_t {
        std::uint32_t cf;
        CHECK_STATUS(tinytc_binary_get_core_features(obj_, &cf));
        return cf;
    }
};

inline auto compile_to_opencl(prog prg, core_info const &info,
                              tinytc_source_context_t ctx = nullptr) -> source {
    tinytc_source_t src;
    CHECK_STATUS(tinytc_prog_compile_to_opencl(&src, prg.get(), info.get(), ctx));
    return source{src};
}

inline auto compile_to_binary(source const &src, core_info const &info, bundle_format format,
                              tinytc_source_context_t ctx = nullptr) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_source_compile_to_binary(&bin, src.get(), info.get(),
                                                 static_cast<tinytc_bundle_format_t>(format), ctx));
    return binary{bin};
}

inline auto compile_to_binary(prog prg, core_info const &info, bundle_format format,
                              tinytc_source_context_t ctx = nullptr) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_prog_compile_to_binary(&bin, prg.get(), info.get(),
                                               static_cast<tinytc_bundle_format_t>(format), ctx));
    return binary{bin};
}

////////////////////////////
////////// Runtime /////////
////////////////////////////

namespace internal {
template <typename T>
concept has_make_kernel_bundle =
    requires(typename T::context_t ctx, typename T::device_t dev, binary const &bin) {
        { T::make_kernel_bundle(ctx, dev, bin) } -> std::same_as<typename T::kernel_bundle_t>;
    };

template <typename T>
concept has_make_kernel = requires(typename T::native_kernel_bundle_t bundle, char const *name) {
    { T::make_kernel(bundle, name) } -> std::same_as<typename T::kernel_t>;
};

template <typename T>
concept has_make_argument_handler = requires(T t, typename T::device_t dev) {
    { T::make_argument_handler(dev) } -> std::same_as<typename T::argument_handler_t>;
};

template <typename T>
concept has_work_group_size =
    requires(T t, typename T::native_kernel_t kernel, typename T::device_t dev) {
        { T::work_group_size(kernel, dev) } -> std::same_as<typename T::work_group_size_t>;
    };

template <typename T, typename Wrapped, typename Native>
concept has_get = requires(Wrapped w) {
    { T::get(w) } -> std::convertible_to<Native>;
};

template <typename T>
concept has_submit_managed =
    requires(typename T::work_group_size_t const &work_group_size, std::size_t howmany,
             typename T::native_kernel_t kernel, typename T::command_list_t q,
             std::vector<typename T::native_event_t> const &dep_events) {
        {
            T::submit(work_group_size, howmany, kernel, q, dep_events)
        } -> std::same_as<typename T::event_t>;
    };

template <typename T>
concept has_submit_unmanaged =
    requires(typename T::work_group_size_t const &work_group_size, std::size_t howmany,
             typename T::native_kernel_t kernel, typename T::command_list_t q,
             typename T::native_event_t signal_event, std::uint32_t num_wait_events,
             typename T::native_event_t *wait_events) {
        T::submit(work_group_size, howmany, kernel, q, signal_event, num_wait_events, wait_events);
    };
} // namespace internal

/**
 * @brief Defines functions and members a runtime class has to provide
 */
template <typename T>
concept runtime = requires(T rt, std::uint8_t const *binary, std::size_t binary_size,
                           bundle_format format, std::uint32_t core_features) {
    typename T::context_t;
    typename T::device_t;
    typename T::kernel_bundle_t;
    typename T::kernel_t;
    typename T::native_kernel_bundle_t;
    typename T::native_kernel_t;
    typename T::argument_handler_t;
    typename T::command_list_t;
    typename T::event_t;
    typename T::native_event_t;
    typename T::mem_t;
    typename T::const_mem_t;
    typename T::work_group_size_t;
    { T::is_event_managed } -> std::convertible_to<bool>;
    requires std::movable<typename T::kernel_bundle_t>;
    requires std::movable<typename T::kernel_t>;
    requires internal::has_make_kernel_bundle<T>;
    requires internal::has_make_kernel<T>;
    requires internal::has_make_argument_handler<T>;
    requires internal::has_work_group_size<T>;
    requires internal::has_get<T, typename T::kernel_bundle_t, typename T::native_kernel_bundle_t>;
    requires internal::has_get<T, typename T::kernel_t, typename T::native_kernel_t>;
    requires(T::is_event_managed && internal::has_submit_managed<T>) ||
                (!T::is_event_managed && internal::has_submit_unmanaged<T>);
};

/**
 * @brief Encapsulates a tensor compute kernel for a runtime of type T
 */
template <runtime T> class tensor_kernel {
  public:
    using kernel_t = typename T::kernel_t;                     ///< Kernel type
    using argument_handler_t = typename T::argument_handler_t; ///< Argument handler type
    using command_list_t = typename T::command_list_t;         ///< Command list / queue type
    using event_t = typename T::event_t;                       ///< Event type
    using native_event_t = typename T::native_event_t;         ///< Native event type
    using work_group_size_t = typename T::work_group_size_t;   ///< Work group size type

    /**
     * @brief ctor
     *
     * The constructor is usally not invoked directly but a tensor_kernel<T> object
     * is obtained via the tensor_kernel_bundle<T>::get function.
     *
     * @param kernel Wrapped kernel object
     * @param arg_handler Runtime-specific argument handler
     * @param metadata Kernel attributes like work-group size and subgroup size
     */
    tensor_kernel(kernel_t kernel, argument_handler_t arg_handler,
                  work_group_size_t work_group_size)
        : kernel_(std::move(kernel)), arg_handler_(std::move(arg_handler)),
          work_group_size_(std::move(work_group_size)) {}

    /**
     * @brief Set kernel argument
     *
     * Calls the runtime's argument setter function, e.g. zeKernelSetArgumentValue
     * or clSetKernelArg.
     *
     * @tparam Arg Type of argument
     * @param arg_index Argument position in kernel prototype
     * @param arg Argument value
     */
    template <typename Arg> void set_arg(std::uint32_t arg_index, Arg const &arg) {
        arg_handler_.set_arg(T::get(kernel_), arg_index, arg);
    }
    /**
     * @brief Convenience wrapper for set_arg
     *
     * set_args forwards each argument to set_arg.
     * The argument index increases from left to right, that is, for
     * @code set_args(arg_0, ..., arg_N) @endcode
     * arg_0 has argument index 0 and arg_N has argument index N.
     *
     * @tparam Arg Argument types
     * @param ...args arguments
     */
    template <typename... Arg> void set_args(Arg const &...args) {
        arg_handler_.set_args(T::get(kernel_), args...);
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * This submit prototype is only available if the runtime's native event
     * type supports reference counting, such as the sycl::event or cl_event type.
     *
     * @param howmany Group size
     * @param q Runtime's queue type
     * @param dep_events Vector of events that need to be waited on before execution
     */
    auto submit(std::size_t howmany, command_list_t q,
                std::vector<native_event_t> const &dep_events = {}) -> event_t
    requires(T::is_event_managed)
    {
        return T::submit(work_group_size_, howmany, T::get(kernel_), std::move(q), dep_events);
    }

    /**
     * @brief Submits a kernel to the runtime for execution on the device.
     *
     * This submit prototype is only available if the lifetime of the runtime's native event type
     * is user-managed, such as the ze_event_handle_t type.
     *
     * @param howmany Group size
     * @param q Runtime's command list type
     * @param signal_event Event that is signalled on kernel completion
     * @param num_wait_events Number of events that need to be waited on before execution
     * @param wait_events Pointer to num_wait_events event handles
     */
    void submit(std::size_t howmany, command_list_t q, native_event_t signal_event = nullptr,
                std::uint32_t num_wait_events = 0, native_event_t *wait_events = nullptr)
    requires(!T::is_event_managed)
    {
        T::submit(work_group_size_, howmany, T::get(kernel_), q, signal_event, num_wait_events,
                  wait_events);
    }

  private:
    kernel_t kernel_;
    argument_handler_t arg_handler_;
    work_group_size_t work_group_size_;
};

/**
 * @brief Encapsulates a compiled tensor program for a runtime of type T
 */
template <runtime T> class tensor_kernel_bundle {
  public:
    using context_t = typename T::context_t;                   ///< Context type
    using device_t = typename T::device_t;                     ///< Device type
    using kernel_bundle_t = typename T::kernel_bundle_t;       ///< Kernel bundle type
    using argument_handler_t = typename T::argument_handler_t; ///< Argument handler type

    /**
     * @brief ctor
     *
     * @param bin Binary
     * @param ctx Context
     * @param dev Device
     */
    tensor_kernel_bundle(binary const &bin, context_t ctx, device_t dev)
        : bundle_(T::make_kernel_bundle(std::move(ctx), dev, bin)),
          arg_handler_(T::make_argument_handler(std::move(dev))), dev_{dev} {}

    /**
     * @brief Get a kernel by name from the kernel bundle
     *
     * @param name Kernel name
     *
     * @return Tensor kernel object
     */
    auto get(char const *name) -> tensor_kernel<T> {
        auto krnl = T::make_kernel(T::get(bundle_), name);
        auto wgs = T::work_group_size(T::get(krnl), dev_);
        return {std::move(krnl), arg_handler_, wgs};
    }

  private:
    kernel_bundle_t bundle_;
    argument_handler_t arg_handler_;
    device_t dev_;
};

////////////////////////////
////////// Recipe //////////
////////////////////////////

template <typename T> struct auto_mem_type;

template <typename T>
concept pointer_to_scalar = std::is_pointer_v<T> && !std::is_class_v<std::remove_pointer_t<T>>;

template <pointer_to_scalar T> struct auto_mem_type<T> {
    constexpr static mem_type value = mem_type::usm_pointer;
};

template <typename T> inline constexpr auto auto_mem_type_v = auto_mem_type<T>::value;

struct mem : ::tinytc_mem {
    template <typename T>
    inline mem(T value, mem_type type = auto_mem_type_v<T>)
        : ::tinytc_mem{value, static_cast<tinytc_mem_type_t>(type)} {}
};

template <> struct shared_handle_traits<tinytc_recipe_t> {
    static auto retain(tinytc_recipe_t handle) -> tinytc_status_t {
        return tinytc_recipe_retain(handle);
    }
    static auto release(tinytc_recipe_t handle) -> tinytc_status_t {
        return tinytc_recipe_release(handle);
    }
};

template <> struct shared_handle_traits<tinytc_recipe_handler_t> {
    static auto retain(tinytc_recipe_handler_t handle) -> tinytc_status_t {
        return tinytc_recipe_handler_retain(handle);
    }
    static auto release(tinytc_recipe_handler_t handle) -> tinytc_status_t {
        return tinytc_recipe_handler_release(handle);
    }
};

class recipe : public shared_handle<tinytc_recipe_t> {
  public:
    using shared_handle::shared_handle;

    auto get_prog() const -> prog {
        tinytc_prog_t prg;
        CHECK_STATUS(tinytc_recipe_get_prog(obj_, &prg));
        return {prg};
    }

    auto get_binary() const -> binary {
        tinytc_binary_t bin;
        CHECK_STATUS(tinytc_recipe_get_binary(obj_, &bin));
        return {bin};
    }
};

class recipe_handler : public shared_handle<tinytc_recipe_handler_t> {
  public:
    using shared_handle::shared_handle;

    auto get_recipe() const -> recipe {
        tinytc_recipe_t rec;
        CHECK_STATUS(tinytc_recipe_handler_get_recipe(obj_, &rec));
        return {rec};
    }
};

class small_gemm_batched : public recipe {
  public:
    using recipe::recipe;

    inline small_gemm_batched(core_info const &info, scalar_type ty, transpose tA, transpose tB,
                              uint32_t M, uint32_t N, uint32_t K, uint32_t ldA, uint32_t strideA,
                              uint32_t ldB, uint32_t strideB, uint32_t ldC, uint32_t strideC,
                              tinytc_source_context_t ctx = nullptr) {
        CHECK_STATUS(tinytc_recipe_small_gemm_batched_create(
            &obj_, info.get(), static_cast<tinytc_scalar_type_t>(ty),
            static_cast<tinytc_transpose_t>(tA), static_cast<tinytc_transpose_t>(tB), M, N, K, ldA,
            strideA, ldB, strideB, ldC, strideC, ctx));
    }

    template <typename T>
    static void set_args(recipe_handler &handler, uint32_t howmany, T alpha, mem A, mem B, T beta,
                         mem C) {
        CHECK_STATUS(tinytc_recipe_small_gemm_batched_set_args(
            handler.get(), howmany, sizeof(alpha), &alpha, A, B, sizeof(beta), &beta, C));
    }
};

class tall_and_skinny : public recipe {
  public:
    using recipe::recipe;

    inline tall_and_skinny(core_info const &info, scalar_type ty, std::uint32_t N, std::uint32_t K,
                           std::uint32_t M_block_size = 0, tinytc_source_context_t ctx = nullptr) {
        CHECK_STATUS(tinytc_recipe_tall_and_skinny_create(
            &obj_, info.get(), static_cast<tinytc_scalar_type_t>(ty), N, K, M_block_size, ctx));
    }

    template <typename T>
    static void set_args(recipe_handler &handler, std::uint32_t M, T alpha, mem A,
                         std::uint32_t ldA, mem B, std::uint32_t ldB, T beta, mem C,
                         std::uint32_t ldC) {
        CHECK_STATUS(tinytc_recipe_tall_and_skinny_set_args(
            handler.get(), M, sizeof(alpha), &alpha, A, ldA, B, ldB, sizeof(beta), &beta, C, ldC));
    }
};

} // namespace tinytc

#endif // TINYTC_20240403_HPP
