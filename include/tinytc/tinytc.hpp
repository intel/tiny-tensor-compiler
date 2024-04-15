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

////////////////////////////
////////// Macros //////////
////////////////////////////

//! Capture error code and throw error code if unsuccessful
#define TINYTC_CHECK(X)                                                                            \
    [](tinytc_status_t code) {                                                                     \
        if (code != tinytc_status_success) {                                                       \
            auto code_ec = static_cast<status>(code);                                              \
            throw code_ec;                                                                         \
        }                                                                                          \
    }(X)

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

//! Convert error code to string
inline char const *error_string(status code) {
    return ::tinytc_error_string(static_cast<::tinytc_status_t>(code));
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

    //! Create empty (invalid) handle
    shared_handle() : obj_{nullptr} {}
    //! Create handle from C handle
    shared_handle(T obj, bool needs_retain = false) : obj_(obj) {
        if (needs_retain) {
            TINYTC_CHECK(c_retain());
        }
    }
    //! Decrease reference count
    ~shared_handle() { c_release(); }
    //! Copy ctor
    shared_handle(shared_handle const &other) : obj_(other.obj_) { TINYTC_CHECK(c_retain()); }
    //! Move ctor
    shared_handle(shared_handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    shared_handle &operator=(shared_handle const &other) {
        if (obj_ != other.obj_) {
            TINYTC_CHECK(c_release());
            obj_ = other.obj_;
            TINYTC_CHECK(c_retain());
        }
        return *this;
    }
    //! Move operator
    shared_handle &operator=(shared_handle &&other) {
        if (obj_ != other.obj_) {
            TINYTC_CHECK(c_release());
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
        TINYTC_CHECK(
            tinytc_scalar_type_create(&obj_, static_cast<tinytc_scalar_type_t>(type), &loc));
    }
};

inline data_type create_memref(scalar_type scalar_ty, std::vector<std::int64_t> const &shape,
                               std::vector<std::int64_t> const &stride = {},
                               location const &loc = {}) {
    tinytc_data_type_t mt;
    TINYTC_CHECK(tinytc_memref_type_create(&mt, static_cast<tinytc_scalar_type_t>(scalar_ty),
                                           shape.size(), shape.data(), stride.size(), stride.data(),
                                           &loc));
    return data_type(mt);
}
inline data_type create_group(data_type memref_ty, location const &loc = {}) {
    tinytc_data_type_t gt;
    TINYTC_CHECK(tinytc_group_type_create(&gt, memref_ty.get(), &loc));
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
    value(data_type ty) { TINYTC_CHECK(tinytc_value_create(&obj_, ty.get())); }
    //! Create immediate value from float
    value(float imm) { TINYTC_CHECK(tinytc_float_imm_create(&obj_, imm, tinytc_scalar_type_f32)); }
    //! Create immediate value from double
    value(double imm, scalar_type type = scalar_type::f64) {
        TINYTC_CHECK(tinytc_float_imm_create(&obj_, imm, static_cast<tinytc_scalar_type_t>(type)));
    }
    //! Create immediate value from int8_t
    value(std::int8_t imm) {
        TINYTC_CHECK(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i8));
    }
    //! Create immediate value from int16_t
    value(std::int16_t imm) {
        TINYTC_CHECK(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i16));
    }
    //! Create immediate value from int32_t
    value(std::int32_t imm) {
        TINYTC_CHECK(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_i32));
    }
    //! Create immediate value from int64_t
    value(std::int64_t imm, scalar_type type = scalar_type::i64) {
        TINYTC_CHECK(tinytc_int_imm_create(&obj_, imm, static_cast<tinytc_scalar_type_t>(type)));
    }
    //! Create immediate value from uint32_t (index type)
    value(std::uint32_t imm) {
        TINYTC_CHECK(tinytc_int_imm_create(&obj_, imm, tinytc_scalar_type_index));
    }

    inline auto get_name() const -> char const * {
        char const *name;
        TINYTC_CHECK(tinytc_value_get_name(obj_, &name));
        return name;
    }
    inline void name(std::string const &name) {
        TINYTC_CHECK(tinytc_value_set_name(obj_, name.c_str()));
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
        TINYTC_CHECK(tinytc_inst_get_value(obj_, &result));
        return value(result);
    }

    inline auto get_values() const -> std::vector<value> {
        static_assert(value_reinterpret_allowed);
        uint32_t result_list_size = 0;
        TINYTC_CHECK(tinytc_inst_get_values(obj_, &result_list_size, nullptr));
        auto values = std::vector<value>(result_list_size);
        tinytc_value_t *result_list = reinterpret_cast<tinytc_value_t *>(values.data());
        TINYTC_CHECK(tinytc_inst_get_values(obj_, &result_list_size, result_list));
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
        TINYTC_CHECK(tinytc_region_create(&obj_, instructions.size(),
                                          reinterpret_cast<tinytc_inst_t *>(instructions.data()),
                                          &loc));
    }
};

////////////////////////////
/////// Instructions ///////
////////////////////////////

inline inst create_binary_op(binary_op op, value const &a, value const &b,
                             location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_binary_op_inst_create(&instr, static_cast<tinytc_binary_op_t>(op), a.get(),
                                              b.get(), &loc));
    return inst(instr);
}

inline inst create_make_cast(value const &a, scalar_type to_ty, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(
        tinytc_cast_inst_create(&instr, a.get(), static_cast<tinytc_scalar_type_t>(to_ty), &loc));
    return inst(instr);
}

inline inst create_cmp(cmp_condition cond, value const &a, value const &b,
                       location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_cmp_inst_create(&instr, static_cast<tinytc_cmp_condition_t>(cond), a.get(),
                                        b.get(), &loc));
    return inst(instr);
}

inline inst create_neg(value const &a, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_neg_inst_create(&instr, a.get(), &loc));
    return inst(instr);
}

inline inst create_alloca(data_type const &ty, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_alloca_inst_create(&instr, ty.get(), &loc));
    return inst(instr);
}

inline inst create_axpby(transpose tA, bool atomic, value const &alpha, value const &A,
                         value const &beta, value const &B, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_axpby_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                          alpha.get(), A.get(), beta.get(), B.get(), &loc));
    return inst(instr);
}

inline inst create_expand(value const &a, std::int64_t mode, std::vector<value> &expand_shape,
                          location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = expand_shape.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("expand shape too large");
    }
    tinytc_value_t *eshape = reinterpret_cast<tinytc_value_t *>(expand_shape.data());
    TINYTC_CHECK(tinytc_expand_inst_create(&instr, a.get(), mode, len, eshape, &loc));
    return inst(instr);
}

inline inst create_fuse(value const &a, std::int64_t from, std::int64_t to,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_fuse_inst_create(&instr, a.get(), from, to, &loc));
    return inst(instr);
}

inline inst create_load(value const &a, std::vector<value> &index_list, location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    tinytc_value_t *il = reinterpret_cast<tinytc_value_t *>(index_list.data());
    TINYTC_CHECK(tinytc_load_inst_create(&instr, a.get(), len, il, &loc));
    return inst(instr);
}

inline inst create_group_id(location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_group_id_inst_create(&instr, &loc));
    return inst(instr);
}

inline inst create_group_size(location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_group_size_inst_create(&instr, &loc));
    return inst(instr);
}

inline inst create_gemm(transpose tA, transpose tB, bool atomic, value const &alpha, value const &A,
                        value const &B, value const &beta, value const &C,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_gemm_inst_create(&instr, static_cast<tinytc_transpose_t>(tA),
                                         static_cast<tinytc_transpose_t>(tB), atomic, alpha.get(),
                                         A.get(), B.get(), beta.get(), C.get(), &loc));
    return inst(instr);
}

inline inst create_gemv(transpose tA, bool atomic, value const &alpha, value const &A,
                        value const &B, value const &beta, value const &C,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_gemv_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                         alpha.get(), A.get(), B.get(), beta.get(), C.get(), &loc));
    return inst(instr);
}

inline inst create_ger(bool atomic, value const &alpha, value const &A, value const &B,
                       value const &beta, value const &C, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_ger_inst_create(&instr, atomic, alpha.get(), A.get(), B.get(), beta.get(),
                                        C.get(), &loc));
    return inst(instr);
}

inline inst create_hadamard(bool atomic, value const &alpha, value const &A, value const &B,
                            value const &beta, value const &C, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_hadamard_inst_create(&instr, atomic, alpha.get(), A.get(), B.get(),
                                             beta.get(), C.get(), &loc));
    return inst(instr);
}

inline inst create_size(value const &a, std::int64_t mode, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_size_inst_create(&instr, a.get(), mode, &loc));
    return inst(instr);
}

inline inst create_subview(value const &a, std::vector<value> &offset_list,
                           std::vector<value> &size_list, location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    if (offset_list.size() != size_list.size()) {
        throw std::invalid_argument("offset list must have the same length as the size list");
    }
    auto len = offset_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("slice list too long");
    }
    tinytc_value_t *ol = reinterpret_cast<tinytc_value_t *>(offset_list.data());
    tinytc_value_t *sl = reinterpret_cast<tinytc_value_t *>(size_list.data());
    TINYTC_CHECK(tinytc_subview_inst_create(&instr, a.get(), len, ol, sl, &loc));
    return inst(instr);
}

inline inst create_store(value const &val, value const &a, std::vector<value> &index_list,
                         location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    tinytc_value_t *il = reinterpret_cast<tinytc_value_t *>(index_list.data());
    TINYTC_CHECK(tinytc_store_inst_create(&instr, val.get(), a.get(), len, il, &loc));
    return inst(instr);
}

inline inst create_sum(transpose tA, bool atomic, value const &alpha, value const &A,
                       value const &beta, value const &B, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_sum_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                        alpha.get(), A.get(), beta.get(), B.get(), &loc));
    return inst(instr);
}

inline inst create_for(value const &loop_var, value const &from, value const &to, value const &step,
                       region const &body, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(tinytc_for_inst_create(&instr, loop_var.get(), from.get(), to.get(), step.get(),
                                        body.get(), &loc));
    return inst(instr);
}

inline inst create_foreach(value const &loop_var, value const &from, value const &to,
                           region const &body, location const &loc = {}) {
    tinytc_inst_t instr;
    TINYTC_CHECK(
        tinytc_foreach_inst_create(&instr, loop_var.get(), from.get(), to.get(), body.get(), &loc));
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
    TINYTC_CHECK(tinytc_if_inst_create(&instr, condition.get(), then.get(), otherwise.get(), len,
                                       rl_vec.data(), &loc));
    return inst(instr);
}

inline inst create_yield(std::vector<value> &yield_list, location const &loc = {}) {
    static_assert(value_reinterpret_allowed);
    tinytc_inst_t instr;
    auto len = yield_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("slice list too long");
    }
    tinytc_value_t *yl = reinterpret_cast<tinytc_value_t *>(yield_list.data());
    TINYTC_CHECK(tinytc_yield_inst_create(&instr, len, yl, &loc));
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
    TINYTC_CHECK(tinytc_function_prototype_create(&fun, name, len, al, &loc));
    return func(fun);
}

inline func create_function(func const &prototype, region const &body, location const &loc = {}) {
    tinytc_func_t fun;
    TINYTC_CHECK(tinytc_function_create(&fun, prototype.get(), body.get(), &loc));
    return func(fun);
}

inline void set_work_group_size(func &fun, uint32_t x, uint32_t y) {
    TINYTC_CHECK(tinytc_function_set_work_group_size(fun.get(), x, y));
}

inline void set_subgroup_size(func &fun, uint32_t sgs) {
    TINYTC_CHECK(tinytc_function_set_subgroup_size(fun.get(), sgs));
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

class prog : public shared_handle<tinytc_prog_t> {
  public:
    using shared_handle::shared_handle;
};

inline prog create_program(std::vector<func> &fun_list, location const &loc = {}) {
    static_assert(func_reinterpret_allowed);
    tinytc_prog_t prg;
    auto len = fun_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("function list too long");
    }
    tinytc_func_t *fl = reinterpret_cast<tinytc_func_t *>(fun_list.data());
    TINYTC_CHECK(tinytc_program_create(&prg, len, fl, &loc));
    return prog(prg);
}

////////////////////////////
//////// Device info ///////
////////////////////////////

template <> struct unique_handle_traits<tinytc_core_info_t> {
    static void destroy(tinytc_core_info_t obj) { tinytc_core_info_destroy(obj); }
};

class core_info : public unique_handle<tinytc_core_info_t> {
  public:
    using unique_handle::unique_handle;
};

inline auto get_core_info_intel_gpu(intel_gpu_architecture arch) -> core_info {
    tinytc_core_info_t info;
    TINYTC_CHECK(tinytc_core_info_intel_gpu_create(
        &info, static_cast<tinytc_intel_gpu_architecture_t>(arch)));
    return core_info{info};
}

////////////////////////////
///////// Compiler /////////
////////////////////////////

template <> struct unique_handle_traits<tinytc_source_t> {
    static void destroy(tinytc_source_t obj) { tinytc_source_destroy(obj); }
};
template <> struct unique_handle_traits<tinytc_binary_t> {
    static void destroy(tinytc_binary_t obj) { tinytc_binary_destroy(obj); }
};

class source : public unique_handle<tinytc_source_t> {
  public:
    using unique_handle::unique_handle;

    inline auto get_code() -> char const * {
        char const *code = nullptr;
        TINYTC_CHECK(tinytc_source_get_code(obj_, &code));
        return code;
    }
};

class binary : public unique_handle<tinytc_binary_t> {
  public:
    using unique_handle::unique_handle;
};

inline auto compile_to_opencl(prog &prg, core_info const &info, error_handler err_handler = nullptr,
                              void *err_handler_data = nullptr) -> source {
    tinytc_source_t src;
    TINYTC_CHECK(
        tinytc_prog_compile_to_opencl(&src, prg.get(), info.get(), err_handler, err_handler_data));
    return source{src};
}

inline auto compile_to_binary(source const &src, core_info const &info, bundle_format format,
                              error_handler err_handler = nullptr, void *err_handler_data = nullptr)
    -> binary {
    tinytc_binary_t bin;
    TINYTC_CHECK(tinytc_source_compile_to_binary(&bin, src.get(), info.get(),
                                                 static_cast<tinytc_bundle_format_t>(format),
                                                 err_handler, err_handler_data));
    return binary{bin};
}

inline auto compile_to_binary(prog &prg, core_info const &info, bundle_format format,
                              error_handler err_handler = nullptr, void *err_handler_data = nullptr)
    -> binary {
    tinytc_binary_t bin;
    TINYTC_CHECK(tinytc_prog_compile_to_binary(&bin, prg.get(), info.get(),
                                               static_cast<tinytc_bundle_format_t>(format),
                                               err_handler, err_handler_data));
    return binary{bin};
}

} // namespace tinytc

#endif // TINYTC_20240403_HPP
