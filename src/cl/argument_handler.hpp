// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_ARGUMENT_HANDLER_20240424_HPP
#define CL_ARGUMENT_HANDLER_20240424_HPP

#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cstdint>

namespace tinytc {

class opencl_argument_handler {
  public:
    //! Signature of clSetKernelArgMemPointerINTEL function
    using clSetKernelArgMemPointerINTEL_t = cl_int (*)(cl_kernel kernel, cl_uint arg_index,
                                                       const void *arg_value);
    //! ctor
    inline opencl_argument_handler() = default;
    //! ctor; checks whether cl_intel_unified_shared_memory is available and gets
    //! clSetKernelArgMemPointerINTEL
    inline opencl_argument_handler(cl_platform_id plat) { set_platform(plat); }

    inline void set_platform(cl_platform_id plat) {
        clSetKernelArgMemPointerINTEL_ =
            (clSetKernelArgMemPointerINTEL_t)clGetExtensionFunctionAddressForPlatform(
                plat, "clSetKernelArgMemPointerINTEL");
    }

    /**
     * @brief Set single kernel argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param arg_size Size of argument
     * @param arg_value Pointer to argument value
     */
    inline void set_arg(cl_kernel kernel, std::uint32_t arg_index, std::size_t arg_size,
                        void const *arg_value) const {
        CL_CHECK_STATUS(clSetKernelArg(kernel, arg_index, arg_size, arg_value));
    }

    /**
     * @brief Set memory argument
     *
     * @param kernel Kernel handle
     * @param arg_index Argument index
     * @param mem Memory object
     */
    inline void set_mem_arg(cl_kernel kernel, std::uint32_t arg_index, const void *value,
                            tinytc_mem_type_t ty) const {
        switch (ty) {
        case tinytc_mem_type_buffer:
            set_arg(kernel, arg_index, sizeof(value), &value);
            return;
        case tinytc_mem_type_usm_pointer:
            if (clSetKernelArgMemPointerINTEL_ == nullptr) {
                throw status::unavailable_extension;
            }
            CL_CHECK_STATUS(clSetKernelArgMemPointerINTEL_(kernel, arg_index, value));
            return;
        case tinytc_mem_type_svm_pointer:
            CL_CHECK_STATUS(clSetKernelArgSVMPointer(kernel, arg_index, value));
            return;
        default:
            break;
        }
        throw status::invalid_arguments;
    }

  private:
    clSetKernelArgMemPointerINTEL_t clSetKernelArgMemPointerINTEL_ = nullptr;
};

} // namespace tinytc

#endif // CL_ARGUMENT_HANDLER_20240424_HPP
