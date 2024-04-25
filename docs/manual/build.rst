.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

====================
Building and linking
====================

Dependencies
============

- `CMake <https://www.cmake.org>`_ >= 3.23
- `Intel oneAPI Base Toolkit <https://www.intel.com/content/www/us/en/developer/tools/oneapi/toolkits.html>`_
- OpenCL
- Level Zero
- `ocloc <https://github.com/intel/compute-runtime>`_ (OpenCL offline compiler from the Intel Compute Runtime)
- `Double-Batched FFT Library <https://github.com/intel/double-batched-fft-library>`_ >= 0.5.1
- `re2c <http://re2c.org>`_ >= 3.0
- `bison <https://www.gnu.org/software/bison/>`_ >= 3.8.2

Build from source using oneAPI
==============================

Install CMake, the oneAPI Base Toolkit, the Intel compute runtime, re2c, and bison using your system
package manager.

Initialize the oneAPI environment.

.. code:: console

    . /opt/intel/oneapi/setvars.sh

Clone the Double-Batched FFT library to your filesystem.

.. code:: console

   git clone https://github.com/intel/double-batched-fft-library.git

We only need libclir.so that we build and install using the following steps.

.. code:: console

   cd double-batched-fft-library/clir/
   cmake -Bbuild -GNinja -DCMAKE_CXX_COMPILER=icpx -DCMAKE_INSTALL_PREFIX=$(pwd)/../../install/ \
         -DCMAKE_CXX_FLAGS="-ffp-model=precise" -DBUILD_SHARED_LIBS=YES
   cmake --build build
   cmake --install build
   cd ../..

Then, build and install Tiny Tensor Compiler with the following steps

.. code:: console

   git clone https://github.com/intel/tiny-tensor-compiler.git tinytc
   cd tinytc
   cmake -Bbuild -GNinja -DCMAKE_CXX_COMPILER=icpx -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
         -DCMAKE_PREFIX_PATH=$(pwd)/../install -DBUILD_SHARED_LIBS=YES
   cmake --build build
   cmake --install build
   cd ..

If you need a static library, set `-DBUILD_SHARED_LIBS=NO` when compiling libclir and the Tiny Tensor Compiler.

Build options
=============

The following CMake option options are supported.

====================== ============
Option                 Description
====================== ============
BUILD_DOCUMENTATION    Build the documentation
BUILD_TESTING          Build unit tests
BUILD_LEVEL_ZERO       Build libtinytc_ze for Level Zero support (enforced if BUILD_SYCL=ON)
BUILD_OPENCL           Build libtinytc_cl for OpenCL support (enforced if BUILD_SYCL=ON)
BUILD_SYCL             Build libtinytc_sycl for SYCL support
====================== ============

Linking in a CMake project
==========================

CMake targets are exported, therefore you only need

.. code:: cmake

    find_package(tinytc REQUIRED)

in your CMakeLists.txt to find the Tiny Tensor Compiler.

.. note::

   For non-standard installation directories add -DCMAKE_PREFIX_PATH=/path/to/installation
   when invoking cmake.

Runtime support is split in the three library libtinytc_ze, libtinytc_cl, and libtinytc_sycl.
The BUILD_(LEVEL_ZERO, OPENCL, SYCL) options control which libraries are built, respectively.
For example, when using OpenCL only, you can set BUILD_SYCL=OFF such that you do
not need a C++ compiler with SYCL support.

For runtime support you have to add one of the following `find_package` calls to your CMakeLists.txt:

.. code:: cmake

    # For SYCL
    find_package(tinytc_sycl REQUIRED)
    # For Level Zero
    find_package(tinytc_ze REQUIRED)
    # For OpenCL
    find_package(tinytc_cl REQUIRED)

.. note::

   You can add "static" or "shared" after "REQUIRED" to explicitly request the static or shared library version.

For linking and setting up include directories you only need

.. code:: cmake

    target_link_libraries(your-target PRIVATE tinytc::tinytc tinytc::tinytc_sycl)
    # or
    target_link_libraries(your-target PRIVATE tinytc::tinytc tinytc::tinytc_ze)
    # or
    target_link_libraries(your-target PRIVATE tinytc::tinytc tinytc::tinytc_cl)
