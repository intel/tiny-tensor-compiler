// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"

#include <cstdint>
#include <level_zero/ze_api.h>
#include <utility>

using tinytc::ZE_CHECK_STATUS;

level_zero_test_runtime::level_zero_test_runtime() {
    ZE_CHECK_STATUS(zeInit(0));

    std::uint32_t driver_count = 1;
    ze_driver_handle_t driver;
    ZE_CHECK_STATUS(zeDriverGet(&driver_count, &driver));

    std::uint32_t device_count = 1;
    ZE_CHECK_STATUS(zeDeviceGet(driver, &device_count, &dev_));

    auto ctx_desc = ze_context_desc_t{ZE_STRUCTURE_TYPE_CONTEXT_DESC, nullptr, 0};
    ZE_CHECK_STATUS(zeContextCreate(driver, &ctx_desc, &ctx_));

    auto queue_desc = ze_command_queue_desc_t{
        ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, nullptr, 0, 0, 0, ZE_COMMAND_QUEUE_MODE_DEFAULT,
        ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ZE_CHECK_STATUS(zeCommandListCreateImmediate(ctx_, dev_, &queue_desc, &list_));
}
level_zero_test_runtime::~level_zero_test_runtime() {
    zeCommandListDestroy(list_);
    zeContextDestroy(ctx_);
}

void level_zero_test_runtime::memcpy(void *dst, const void *src, std::size_t bytes) {
    ZE_CHECK_STATUS(zeCommandListAppendMemoryCopy(list_, dst, src, bytes, nullptr, 0, nullptr));
    synchronize();
}

auto level_zero_test_runtime::create_buffer(std::size_t bytes) const -> mem_t {
    void *ptr;
    auto device_desc =
        ze_device_mem_alloc_desc_t{ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, nullptr, 0, 0};
    ZE_CHECK_STATUS(zeMemAllocDevice(ctx_, &device_desc, bytes, 0, dev_, &ptr));
    return ptr;
}
void level_zero_test_runtime::free_buffer(mem_t buf) const {
    ZE_CHECK_STATUS(zeMemFree(ctx_, buf));
}
void level_zero_test_runtime::fill_buffer(mem_t buf, int value, std::size_t bytes) {
    ZE_CHECK_STATUS(
        zeCommandListAppendMemoryFill(list_, buf, &value, sizeof(int), bytes, nullptr, 0, nullptr));
    synchronize();
}
void level_zero_test_runtime::memcpy_h2d(mem_t dst, void const *src, std::size_t bytes) {
    this->memcpy(dst, src, bytes);
}
void level_zero_test_runtime::memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes) {
    this->memcpy(dst, src, bytes);
}

auto level_zero_test_runtime::get_core_info() const -> tinytc::core_info {
    return ::tinytc::make_core_info(dev_);
}
auto level_zero_test_runtime::get_device() -> device_t { return dev_; }
auto level_zero_test_runtime::get_context() -> context_t { return ctx_; }
auto level_zero_test_runtime::get_command_list() -> command_list_t { return list_; }
auto level_zero_test_runtime::get_recipe_handler(tinytc::recipe const &rec) -> recipe_handler_t {
    return tinytc::make_recipe_handler(ctx_, dev_, rec);
}
auto level_zero_test_runtime::get_kernel_bundle(tinytc::prog p,
                                                tinytc_core_feature_flags_t core_features)
    -> kernel_bundle_t {
    return ::tinytc::make_kernel_bundle(ctx_, dev_, std::move(p), core_features);
}
auto level_zero_test_runtime::get_kernel(kernel_bundle_t const &bundle, char const *name)
    -> kernel_t {
    return ::tinytc::make_kernel(bundle.get(), name);
}
void level_zero_test_runtime::set_arg(kernel_t &kernel, std::uint32_t arg_index,
                                      std::size_t arg_size, const void *arg_value) {
    ZE_CHECK_STATUS(zeKernelSetArgumentValue(kernel.get(), arg_index, arg_size, arg_value));
}
void level_zero_test_runtime::set_mem_arg(kernel_t &kernel, std::uint32_t arg_index,
                                          const void *arg_value, tinytc::mem_type) {
    set_arg(kernel, arg_index, sizeof(arg_value), &arg_value);
}
void level_zero_test_runtime::submit(kernel_t &kernel, std::int64_t howmany) {
    auto group_count = ::tinytc::get_group_count(howmany);
    ZE_CHECK_STATUS(
        zeCommandListAppendLaunchKernel(list_, kernel.get(), &group_count, nullptr, 0, nullptr));
}
void level_zero_test_runtime::synchronize() {
    ZE_CHECK_STATUS(zeCommandListHostSynchronize(list_, UINT64_MAX));
}

bool level_zero_test_runtime::supports_fp64() {
    auto props = ze_device_module_properties_t{};
    props.stype = ZE_STRUCTURE_TYPE_DEVICE_MODULE_PROPERTIES;
    props.pNext = nullptr;
    ZE_CHECK_STATUS(zeDeviceGetModuleProperties(dev_, &props));
    return bool(props.flags & ZE_DEVICE_MODULE_FLAG_FP64);
}
