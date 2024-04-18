// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"

#include <cstdint>

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
    ZE_CHECK_STATUS(zeMemAllocDevice(ctx_, &device_desc, bytes, 64, dev_, &ptr));
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
    return ::tinytc::create_core_info(dev_);
}
auto level_zero_test_runtime::get_device() -> device_t { return dev_; }
auto level_zero_test_runtime::get_context() -> context_t { return ctx_; }
auto level_zero_test_runtime::get_command_list() -> command_list_t { return list_; }
void level_zero_test_runtime::synchronize() {
    ZE_CHECK_STATUS(zeCommandListHostSynchronize(list_, UINT64_MAX));
}
