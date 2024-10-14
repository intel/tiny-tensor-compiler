// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.h"

#include <stdint.h>

int main(void) {
    tinytc_scalar_type_t sty = tinytc_scalar_type_f32;
    int64_t M = 64;
    int64_t N = 32;

    char const *copy_fun_name = "copy";
    uint32_t num_results;
    uint32_t num_params;
    tinytc_compiler_context_t ctx;
    tinytc_prog_t program;
    tinytc_data_type_t element_ty, ty;
    tinytc_func_t copy_fun;
    tinytc_region_t copy_body;
    tinytc_inst_t tmp;
    tinytc_value_t params[2];
    tinytc_value_t alpha, beta;

    tinytc_compiler_context_create(&ctx);

    // Create program
    tinytc_prog_create(&program, ctx, NULL);

    // Get types
    tinytc_scalar_type_get(&element_ty, ctx, sty);
    int64_t shape[2] = {M, N};
    tinytc_memref_type_get(&ty, element_ty, 2, shape, 0, NULL, tinytc_address_space_global, NULL);

    // Create function
    tinytc_data_type_t param_types[2] = {ty, ty};
    tinytc_func_create(&copy_fun, sizeof(copy_fun_name) - 1, copy_fun_name, 2, param_types, NULL);
    tinytc_prog_add_function(program, copy_fun);

    // Get body
    tinytc_func_get_body(copy_fun, &copy_body);
    num_params = 2;
    tinytc_region_get_parameters(copy_body, &num_params, params);

    // Create instructions
    tinytc_constant_inst_create_one(&tmp, element_ty, NULL);
    num_results = 1;
    tinytc_inst_get_values(tmp, &num_results, &alpha);
    tinytc_region_add_instruction(copy_body, tmp);

    tinytc_constant_inst_create_zero(&tmp, element_ty, NULL);
    num_results = 1;
    tinytc_inst_get_values(tmp, &num_results, &beta);
    tinytc_region_add_instruction(copy_body, tmp);

    tinytc_axpby_inst_create(&tmp, tinytc_transpose_N, 0, alpha, params[0], beta, params[1], NULL);
    tinytc_region_add_instruction(copy_body, tmp);

    // Dump program
    tinytc_prog_dump(program);

    // Clean-up
    tinytc_prog_release(program);
    tinytc_compiler_context_release(ctx);

    return 0;
}
