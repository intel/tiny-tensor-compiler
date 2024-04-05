.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==============
Error handling
==============

All calls to the Level Zero API are wrapped in the :c:macro:`ZE_CHECK` macro.
The macro checks the return code and throws a :cpp:class:`tinytc::level_zero_error`
should the return code not equal `ZE_RESULT_SUCCESS`.

.. doxygendefine:: ZE_CHECK
.. doxygenclass:: tinytc::level_zero_error
.. doxygenfunction:: tinytc::ze_result_to_string
