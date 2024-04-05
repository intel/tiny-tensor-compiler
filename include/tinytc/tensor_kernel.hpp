// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef KERNEL_20240308_HPP
#define KERNEL_20240308_HPP

#include "tinytc/binary.hpp"
#include "tinytc/runtime.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace tinytc {

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
    tensor_kernel(kernel_t kernel, argument_handler_t arg_handler, kernel_metadata const &metadata)
        : kernel_(std::move(kernel)), arg_handler_(std::move(arg_handler)), metadata_(metadata) {}

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
        return T::submit(metadata_.work_group_size, howmany, T::get(kernel_), std::move(q),
                         dep_events);
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
        T::submit(metadata_.work_group_size, howmany, T::get(kernel_), q, signal_event,
                  num_wait_events, wait_events);
    }

    //! Get kernel metadata
    inline auto metadata() const -> kernel_metadata const & { return metadata_; }

  private:
    kernel_t kernel_;
    argument_handler_t arg_handler_;
    kernel_metadata metadata_;
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
    tensor_kernel_bundle(std::shared_ptr<binary> bin, context_t ctx, device_t dev)
        : bin_(std::move(bin)),
          bundle_(T::make_kernel_bundle(bin_->data(), bin_->size(), bin_->format(),
                                        bin_->core_features(), std::move(ctx), dev)),
          arg_handler_(T::make_argument_handler(std::move(dev))) {}

    /**
     * @brief Get a kernel by name from the kernel bundle
     *
     * @param name Kernel name
     *
     * @return Tensor kernel object
     */
    auto get(char const *name) -> tensor_kernel<T> {
        return {T::make_kernel(T::get(bundle_), name), arg_handler_, bin_->metadata(name)};
    }

  private:
    std::shared_ptr<binary> bin_;
    kernel_bundle_t bundle_;
    argument_handler_t arg_handler_;
};

} // namespace tinytc

#endif // KERNEL_20240308_HPP
