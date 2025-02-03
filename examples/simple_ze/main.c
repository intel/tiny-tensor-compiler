// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <tinytc/tinytc.h>
#include <tinytc/tinytc_ze.h>

#include <level_zero/ze_api.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(X)                                                                                   \
    do {                                                                                           \
        status = X;                                                                                \
        if (status != tinytc_status_success) {                                                     \
            printf("Error (%d): %s\n", status, tinytc_error_string(status));                       \
            printf("in %s:%d: \"%s\"\n", __FILE__, __LINE__, #X);                                  \
            goto err;                                                                              \
        }                                                                                          \
    } while (0)

#define ZE_CHECK(X)                                                                                \
    do {                                                                                           \
        ze_result_t result = X;                                                                    \
        if (result != ZE_RESULT_SUCCESS) {                                                         \
            status = tinytc_ze_convert_status(result);                                             \
            printf("Error (%d): %s\n", status, tinytc_error_string(status));                       \
            printf("in %s:%d: \"%s\"\n", __FILE__, __LINE__, #X);                                  \
            goto err;                                                                              \
        }                                                                                          \
    } while (0)

#define TIMEOUT 1000000000

#define STR_(X) #X
#define STR(X) STR_(X)
#define CHUNK_SIZE 64
#define CHUNK_SIZE_S STR(CHUNK_SIZE)

tinytc_status_t gemm(ze_context_handle_t context, ze_device_handle_t device,
                     ze_command_list_handle_t list) {
    tinytc_status_t status = tinytc_status_success;
    tinytc_core_info_t info = NULL;
    tinytc_recipe_t recipe = NULL;
    tinytc_recipe_handler_t handler = NULL;
    void *A = NULL, *B = NULL, *C = NULL;
    float *Chost = NULL;

    CHECK(tinytc_ze_core_info_create(&info, device));

    const uint32_t M = 64, N = 64, K = 64, howmany = 1000;
    CHECK(tinytc_recipe_small_gemm_batched_create(&recipe, info, tinytc_scalar_type_f32,
                                                  tinytc_transpose_N, tinytc_transpose_N, M, N, K,
                                                  M, M * K, K, K * N, M, M * N, NULL));
    CHECK(tinytc_ze_recipe_handler_create(&handler, context, device, recipe));

    const size_t Abytes = M * K * howmany * sizeof(float);
    const size_t Bbytes = K * N * howmany * sizeof(float);
    const size_t Cbytes = M * N * howmany * sizeof(float);
    ze_device_mem_alloc_desc_t mem_desc = {ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, NULL, 0, 0};
    ZE_CHECK(zeMemAllocDevice(context, &mem_desc, Abytes, 64, device, &A));
    ZE_CHECK(zeMemAllocDevice(context, &mem_desc, Bbytes, 64, device, &B));
    ZE_CHECK(zeMemAllocDevice(context, &mem_desc, Cbytes, 64, device, &C));

    const float alpha = 1.0;
    const float beta = 0.0;

    ZE_CHECK(zeCommandListAppendMemoryFill(list, A, &alpha, sizeof(alpha),
                                           M * K * howmany * sizeof(float), NULL, 0, NULL));
    ZE_CHECK(zeCommandListAppendMemoryFill(list, B, &alpha, sizeof(alpha),
                                           K * N * howmany * sizeof(float), NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    CHECK(tinytc_recipe_small_gemm_batched_set_args(
        handler, howmany, sizeof(alpha), &alpha, tinytc_mem_type_usm_pointer, A,
        tinytc_mem_type_usm_pointer, B, sizeof(beta), &beta, tinytc_mem_type_usm_pointer, C));

    CHECK(tinytc_ze_recipe_handler_submit(handler, list, NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    Chost = (float *)malloc(Cbytes);
    if (!Chost) {
        goto err;
    }
    ZE_CHECK(zeCommandListAppendMemoryCopy(list, Chost, C, Cbytes, NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    uint32_t ok = 0;
    for (size_t i = 0; i < M * N * howmany; ++i) {
        ok += ((uint32_t)Chost[i]) == K ? 1 : 0;
    }
    if (ok == M * N * howmany) {
        printf("Matmul was successful\n");
    } else {
        printf("Matmul failed\n");
    }

err:
    free(Chost);
    if (C) {
        zeMemFree(context, C);
    }
    if (B) {
        zeMemFree(context, B);
    }
    if (A) {
        zeMemFree(context, A);
    }
    tinytc_recipe_handler_release(handler);
    tinytc_recipe_release(recipe);
    tinytc_core_info_release(info);

    return status;
}

tinytc_status_t custom_kernel(ze_context_handle_t context, ze_device_handle_t device,
                              ze_command_list_handle_t list) {
    tinytc_status_t status = tinytc_status_success;
    int32_t *host = NULL;
    void *A = NULL, *B = NULL;
    tinytc_core_info_t info = NULL;
    tinytc_prog_t program = NULL;
    ze_module_handle_t module = NULL;
    ze_kernel_handle_t kernel = NULL;

    const uint32_t howmany = 1000;
    const int32_t elements = CHUNK_SIZE * howmany;
    const size_t bytes = elements * sizeof(float);
    host = (int32_t *)malloc(bytes);
    if (!host) {
        goto err;
    }
    for (int32_t i = 0; i < elements; ++i) {
        host[i] = i;
    }

    ze_device_mem_alloc_desc_t mem_desc = {ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, NULL, 0, 0};
    ZE_CHECK(zeMemAllocDevice(context, &mem_desc, bytes, 64, device, &A));
    ZE_CHECK(zeMemAllocDevice(context, &mem_desc, bytes, 64, device, &B));

    ZE_CHECK(zeCommandListAppendMemoryCopy(list, A, host, bytes, NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    CHECK(tinytc_ze_core_info_create(&info, device));

    static const char source_text[] =
        "func @copy(%A: memref<i32x" CHUNK_SIZE_S "x?>, %B: memref<i32x" CHUNK_SIZE_S "x?>) {\n"
        "    %gid = builtin.group_id : index\n"
        "    %a = subview %A[0:" CHUNK_SIZE_S ",%gid] : memref<i32x" CHUNK_SIZE_S ">\n"
        "    %b = subview %B[0:" CHUNK_SIZE_S ",%gid] : memref<i32x" CHUNK_SIZE_S ">\n"
        "    %c0 = constant 0 : i32\n"
        "    %c1 = constant 1 : i32\n"
        "    axpby.n %c1, %a, %c0, %b\n"
        "}\n";

    CHECK(tinytc_parse_string(&program, sizeof(source_text), source_text, NULL));
    CHECK(tinytc_ze_kernel_bundle_create_with_program(&module, context, device, program, 0u));
    CHECK(tinytc_ze_kernel_create(&kernel, module, "copy"));

    ZE_CHECK(zeKernelSetArgumentValue(kernel, 0, sizeof(A), &A));
    ZE_CHECK(zeKernelSetArgumentValue(kernel, 1, sizeof(howmany), &howmany));
    ZE_CHECK(zeKernelSetArgumentValue(kernel, 2, sizeof(B), &B));
    ZE_CHECK(zeKernelSetArgumentValue(kernel, 3, sizeof(howmany), &howmany));
    ze_group_count_t group_count = tinytc_ze_get_group_count(howmany);
    ZE_CHECK(zeCommandListAppendLaunchKernel(list, kernel, &group_count, NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    ZE_CHECK(zeCommandListAppendMemoryCopy(list, host, B, bytes, NULL, 0, NULL));
    ZE_CHECK(zeCommandListHostSynchronize(list, TIMEOUT));

    uint32_t ok = 0;
    for (int32_t i = 0; i < elements; ++i) {
        if (host[i] == i) {
            ++ok;
        }
    }
    if (ok == (uint32_t)elements) {
        printf("Custom kernel was successful\n");
    } else {
        printf("Custom kernel failed\n");
    }

err:
    if (kernel) {
        zeKernelDestroy(kernel);
    }
    if (module) {
        zeModuleDestroy(module);
    }
    tinytc_prog_release(program);
    tinytc_core_info_release(info);
    if (B) {
        zeMemFree(context, B);
    }
    if (A) {
        zeMemFree(context, A);
    }
    free(host);

    return status;
}

int main(void) {
    tinytc_status_t status = tinytc_status_success;
    ze_driver_handle_t driver = NULL;
    ze_device_handle_t device = NULL;
    ze_context_handle_t context = NULL;
    ze_command_list_handle_t list = NULL;

    ZE_CHECK(zeInit(0));

    uint32_t driver_count = 1;
    ZE_CHECK(zeDriverGet(&driver_count, &driver));

    uint32_t device_count = 1;
    ZE_CHECK(zeDeviceGet(driver, &device_count, &device));

    tinytc_support_level_t level;
    CHECK(tinytc_ze_get_support_level(device, &level));
    printf("Device support level: %d\n", level);
    if (level == tinytc_support_level_none) {
        printf("Device is not supported\n");
        status = tinytc_status_unsupported_device;
        goto err;
    }

    ze_context_desc_t context_desc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC, NULL, 0};
    ZE_CHECK(zeContextCreate(driver, &context_desc, &context));

    ze_command_queue_desc_t queue_desc = {
        ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, NULL, 0, 0, 0, ZE_COMMAND_QUEUE_MODE_DEFAULT,
        ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ZE_CHECK(zeCommandListCreateImmediate(context, device, &queue_desc, &list));

    status = gemm(context, device, list);
    if (status != tinytc_status_success) {
        goto err;
    }
    status = custom_kernel(context, device, list);

err:
    if (list) {
        zeCommandListDestroy(list);
    }
    if (context) {
        zeContextDestroy(context);
    }

    return status == tinytc_status_success ? 0 : -1;
}
