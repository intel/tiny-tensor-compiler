.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

============
Introduction
============

The API provided by libtinytc is discussed in this section.
Applications should always include the header

.. code-block:: cpp

   #include <tinytc/tinytc.hpp>

and link against libtinytc.

The library follows the `Semantic Versioning <https://semver.org/>`_ scheme
and the current version is provided via the following constants:

.. doxygenvariable:: major_version
.. doxygenvariable:: minor_version
.. doxygenvariable:: patch_version
.. doxygenvariable:: version

Additionally, the output of git describe is available for development builds:

.. doxygenvariable:: number_of_commits_since_release
.. doxygenvariable:: git_commit
