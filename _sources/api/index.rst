.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

tinytc
======

The Tiny Tensor Compiler provides a C-API and a C++-API.
The C++-API is a header-only library built on top of the C-API that does not introduce
new symbols but only convience functionality for C++-users.

The C-API and C++-API provided by libtinytc is discussed in this section.
Applications should always include the header

.. code-block:: c

   #include "tinytc/tinytc.h"

for using the C-API or include the header

.. code-block:: cpp

   #include "tinytc/tinytc.hpp"

for using the C++-API and link against libtinytc.

.. toctree::
   :maxdepth: 1

   core_capi
   builder_capi
   core_cxxapi
   builder_cxxapi
   parser
   jit
   runtime
   recipe/index
   ir/index
