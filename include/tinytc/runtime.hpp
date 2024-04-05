// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RUNTIME_20240308_HPP
#define RUNTIME_20240308_HPP

#include "tinytc/bundle_format.hpp"

#include <concepts>
#include <cstdint>
#include <vector>

namespace tinytc {

namespace internal {

template <typename T>
concept has_make_kernel_bundle =
    requires(std::uint8_t const *binary, std::size_t binary_size, bundle_format format,
             std::uint32_t core_features, typename T::context_t ctx, typename T::device_t dev) {
        {
            T::make_kernel_bundle(binary, binary_size, format, core_features, ctx, dev)
            } -> std::same_as<typename T::kernel_bundle_t>;
    };

template <typename T>
concept has_make_kernel = requires(typename T::native_kernel_bundle_t bundle, char const *name) {
                              {
                                  T::make_kernel(bundle, name)
                                  } -> std::same_as<typename T::kernel_t>;
                          };

template <typename T>
concept has_make_argument_handler = requires(T t, typename T::device_t dev) {
                                        {
                                            T::make_argument_handler(dev)
                                            } -> std::same_as<typename T::argument_handler_t>;
                                    };

template <typename T, typename Wrapped, typename Native>
concept has_get = requires(Wrapped w) {
                      { T::get(w) } -> std::convertible_to<Native>;
                  };

template <typename T>
concept has_submit_managed =
    requires(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
             typename T::native_kernel_t kernel, typename T::command_list_t q,
             std::vector<typename T::native_event_t> const &dep_events) {
        {
            T::submit(work_group_size, howmany, kernel, q, dep_events)
            } -> std::same_as<typename T::event_t>;
    };

template <typename T>
concept has_submit_unmanaged =
    requires(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany,
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
concept runtime =
    requires(T rt, std::uint8_t const *binary, std::size_t binary_size, bundle_format format,
             std::uint32_t core_features) {
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
        { T::is_event_managed } -> std::convertible_to<bool>;
        requires std::copyable<typename T::kernel_bundle_t>;
        requires std::copyable<typename T::kernel_t>;
        requires internal::has_make_kernel_bundle<T>;
        requires internal::has_make_kernel<T>;
        requires internal::has_make_argument_handler<T>;
        requires internal::has_get<T, typename T::kernel_bundle_t,
                                   typename T::native_kernel_bundle_t>;
        requires internal::has_get<T, typename T::kernel_t, typename T::native_kernel_t>;
        requires(T::is_event_managed && internal::has_submit_managed<T>) ||
                    (!T::is_event_managed && internal::has_submit_unmanaged<T>);
    };
} // namespace tinytc

#endif // RUNTIME_20240308_HPP
