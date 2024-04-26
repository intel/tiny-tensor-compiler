// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.h"

#include <stdint.h>

int main(void) {
    tinytc_scalar_type_t type = tinytc_scalar_type_f32;
    int64_t M = 64;
    int64_t N = 32;

    tinytc_data_type_t dt;
    int64_t shape[2] = {M, N};
    tinytc_memref_type_create(&dt, type, 2, shape, 0, NULL, NULL);

    tinytc_value_t A, B, alpha, beta;
    tinytc_value_create(&A, dt, NULL);
    tinytc_value_create(&B, dt, NULL);
    tinytc_float_imm_create(&alpha, 1.0, type, NULL);
    tinytc_float_imm_create(&beta, 0.0, type, NULL);
    tinytc_data_type_release(dt);

    tinytc_inst_t copy_inst;
    tinytc_axpby_inst_create(&copy_inst, tinytc_transpose_N, 0, alpha, A, beta, B, NULL);
    tinytc_value_release(alpha);
    tinytc_value_release(beta);

    tinytc_func_t copy_proto;
    tinytc_value_t args[2] = {A, B};
    tinytc_function_prototype_create(&copy_proto, "copy", 2, args, NULL);
    tinytc_value_release(A);
    tinytc_value_release(B);

    tinytc_region_t copy_body;
    tinytc_region_create(&copy_body, 1, &copy_inst, NULL);
    tinytc_inst_release(copy_inst);

    tinytc_func_t copy_fun;
    tinytc_function_create(&copy_fun, copy_proto, copy_body, NULL);
    tinytc_func_release(copy_proto);
    tinytc_region_release(copy_body);

    tinytc_prog_t program;
    tinytc_program_create(&program, 1, &copy_fun, NULL);
    tinytc_func_release(copy_fun);

    tinytc_prog_dump(program);

    tinytc_prog_release(program);

    return 0;
}
