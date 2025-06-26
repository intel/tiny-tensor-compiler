// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <tinytc/tinytc.h>
#include <tinytc/tinytc_cl.h>

#include <CL/cl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHECK(X)                                                                                   \
    do {                                                                                           \
        status = X;                                                                                \
        if (status != tinytc_status_success) {                                                     \
            printf("Error (%d): %s\n", status, tinytc_status_to_string(status));                   \
            printf("in %s:%d: \"%s\"\n", __FILE__, __LINE__, #X);                                  \
            goto err;                                                                              \
        }                                                                                          \
    } while (0)

#define CL_CHECK(X)                                                                                \
    do {                                                                                           \
        cl_int result = X;                                                                         \
        if (result != CL_SUCCESS) {                                                                \
            status = tinytc_cl_convert_status(result);                                             \
            printf("Error (%d): %s\n", status, tinytc_status_to_string(status));                   \
            printf("in %s:%d: \"%s\"\n", __FILE__, __LINE__, #X);                                  \
            goto err;                                                                              \
        }                                                                                          \
    } while (0)

#define STR_(X) #X
#define STR(X) STR_(X)
#define CHUNK_SIZE 64
#define CHUNK_SIZE_S STR(CHUNK_SIZE)

tinytc_status_t gemm(cl_context context, cl_device_id device, cl_command_queue queue) {
    tinytc_status_t status = tinytc_status_success;
    tinytc_core_info_t info = NULL;
    tinytc_recipe_t recipe = NULL;
    tinytc_recipe_handler_t handler = NULL;
    cl_mem A = NULL, B = NULL, C = NULL;
    float *Chost = NULL;
    cl_int err;

    CHECK(tinytc_cl_core_info_create(&info, device));

    const uint32_t M = 64, N = 64, K = 64, howmany = 1000;
    CHECK(tinytc_recipe_small_gemm_batched_create(&recipe, info, tinytc_scalar_type_f32,
                                                  tinytc_transpose_N, tinytc_transpose_N, M, N, K,
                                                  M, M * K, K, K * N, M, M * N, NULL));
    CHECK(tinytc_cl_recipe_handler_create(&handler, context, device, recipe));

    const size_t Abytes = M * K * howmany * sizeof(float);
    const size_t Bbytes = K * N * howmany * sizeof(float);
    const size_t Cbytes = M * N * howmany * sizeof(float);
    A = clCreateBuffer(context, CL_MEM_READ_ONLY, Abytes, NULL, &err);
    CL_CHECK(err);
    B = clCreateBuffer(context, CL_MEM_READ_ONLY, Bbytes, NULL, &err);
    CL_CHECK(err);
    C = clCreateBuffer(context, CL_MEM_READ_WRITE, Cbytes, NULL, &err);
    CL_CHECK(err);

    const float alpha = 1.0;
    const float beta = 0.0;

    CL_CHECK(clEnqueueFillBuffer(queue, A, &alpha, sizeof(alpha), 0, Abytes, 0, NULL, NULL));
    CL_CHECK(clEnqueueFillBuffer(queue, B, &alpha, sizeof(alpha), 0, Bbytes, 0, NULL, NULL));
    CL_CHECK(clFinish(queue));

    CHECK(tinytc_recipe_small_gemm_batched_set_args(
        handler, howmany, sizeof(alpha), &alpha, tinytc_mem_type_buffer, A, tinytc_mem_type_buffer,
        B, sizeof(beta), &beta, tinytc_mem_type_buffer, C));

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    CHECK(tinytc_cl_recipe_handler_submit(handler, queue, 0, NULL, NULL));
    CL_CHECK(clFinish(queue));
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("Matmul computation time: %ld ns\n",
           1000000000L * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_nsec -
               start_time.tv_nsec);

    Chost = (float *)malloc(Cbytes);
    if (!Chost) {
        goto err;
    }
    CL_CHECK(clEnqueueReadBuffer(queue, C, CL_TRUE, 0, Cbytes, Chost, 0, NULL, NULL));

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
        clReleaseMemObject(C);
    }
    if (B) {
        clReleaseMemObject(B);
    }
    if (A) {
        clReleaseMemObject(A);
    }
    tinytc_recipe_handler_release(handler);
    tinytc_recipe_release(recipe);
    tinytc_core_info_release(info);

    return status;
}

tinytc_status_t custom_kernel(cl_context context, cl_device_id device, cl_command_queue queue) {
    tinytc_status_t status = tinytc_status_success;
    int32_t *host = NULL;
    cl_mem A = NULL, B = NULL;
    tinytc_core_info_t info = NULL;
    tinytc_prog_t program = NULL;
    cl_program module = NULL;
    cl_kernel kernel = NULL;
    cl_int err;

    const int64_t howmany = 1000;
    const int64_t elements = CHUNK_SIZE * howmany;
    const size_t bytes = elements * sizeof(float);
    host = (int32_t *)malloc(bytes);
    if (!host) {
        goto err;
    }
    for (int32_t i = 0; i < elements; ++i) {
        host[i] = i;
    }

    A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bytes, host, &err);
    CL_CHECK(err);
    B = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, &err);
    CL_CHECK(err);

    CHECK(tinytc_cl_core_info_create(&info, device));

    static const char source_text[] =
        "func @copy(%A: memref<i32x" CHUNK_SIZE_S "x?>, %B: memref<i32x" CHUNK_SIZE_S "x?>) {\n"
        "    %gid = builtin.group_id.x : index\n"
        "    %a = subview %A[0:" CHUNK_SIZE_S ",%gid] : memref<i32x" CHUNK_SIZE_S ">\n"
        "    %b = subview %B[0:" CHUNK_SIZE_S ",%gid] : memref<i32x" CHUNK_SIZE_S ">\n"
        "    %c0 = constant 0 : i32\n"
        "    %c1 = constant 1 : i32\n"
        "    axpby.n %c1, %a, %c0, %b\n"
        "}\n";

    CHECK(tinytc_parse_string(&program, sizeof(source_text), source_text, NULL));
    CHECK(tinytc_cl_kernel_bundle_create_with_program(&module, context, device, program, 0u));
    kernel = clCreateKernel(module, "copy", &err);
    CL_CHECK(err);

    CL_CHECK(clSetKernelArg(kernel, 0, sizeof(A), &A));
    CL_CHECK(clSetKernelArg(kernel, 1, sizeof(howmany), &howmany));
    CL_CHECK(clSetKernelArg(kernel, 2, sizeof(B), &B));
    CL_CHECK(clSetKernelArg(kernel, 3, sizeof(howmany), &howmany));
    size_t ng[3] = {howmany, 1, 1};
    size_t ls[3], gs[3];
    CHECK(tinytc_cl_get_group_size(kernel, ls));
    tinytc_cl_get_global_size(ng, ls, gs);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, 3u, NULL, gs, ls, 0, NULL, NULL));
    CL_CHECK(clFinish(queue));
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("Custom kernel computation time: %ld ns\n",
           1000000000L * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_nsec -
               start_time.tv_nsec);

    CL_CHECK(clEnqueueReadBuffer(queue, B, CL_TRUE, 0, bytes, host, 0, NULL, NULL));

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
        clReleaseKernel(kernel);
    }
    if (module) {
        clReleaseProgram(module);
    }
    tinytc_prog_release(program);
    tinytc_core_info_release(info);
    if (B) {
        clReleaseMemObject(B);
    }
    if (A) {
        clReleaseMemObject(A);
    }
    free(host);

    return status;
}

int main(void) {
    tinytc_status_t status = tinytc_status_success;
    cl_platform_id *platforms = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_int err;

    cl_uint platform_count = 0;
    CL_CHECK(clGetPlatformIDs(platform_count, NULL, &platform_count));
    platforms = (cl_platform_id *)malloc(platform_count * sizeof(cl_platform_id));
    CL_CHECK(clGetPlatformIDs(platform_count, platforms, &platform_count));

    cl_uint device_count = 0;
    for (cl_uint p = 0; p < platform_count; ++p) {
        err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, NULL, &device_count);
        if (err == CL_SUCCESS && device_count > 0) {
            device_count = 1;
            CL_CHECK(clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, &device, NULL));
            char name[256] = {0};
            clGetPlatformInfo(platforms[p], CL_PLATFORM_NAME, sizeof(name) - 1, name, NULL);
            printf("Platform: %s\n", name);
            clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name) - 1, name, NULL);
            printf("Device: %s\n", name);
            break;
        }
    }
    if (device_count == 0) {
        CL_CHECK(CL_DEVICE_NOT_FOUND);
    }

    tinytc_support_level_t level;
    CHECK(tinytc_cl_get_support_level(device, &level));
    printf("Device support level: %d\n", level);
    if (level == tinytc_support_level_none) {
        printf("Device is not supported\n");
        status = tinytc_status_unsupported_device;
        goto err;
    }

    context = clCreateContext(NULL, device_count, &device, NULL, NULL, &err);
    CL_CHECK(err);

    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    CL_CHECK(err);

    status = gemm(context, device, queue);
    if (status != tinytc_status_success) {
        goto err;
    }
    status = custom_kernel(context, device, queue);

err:
    if (queue) {
        clReleaseCommandQueue(queue);
    }
    if (context) {
        clReleaseContext(context);
    }
    free(platforms);

    return status == tinytc_status_success ? 0 : -1;
}
