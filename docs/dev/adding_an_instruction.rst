.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=====================
Adding an instruction
=====================

A new instruction requires us to update compiler passes, sanity checks, lexing and parsing, and code generation.
Here, we outline how one would add the :ref:`subview instruction <subview instruction>` if it would not exist.

Update the specification
========================

The first step is to update the language specification found in ``docs/manual/tensor-ir.rst``.
In this step, we define the instruction's syntax, we describe its semantics, we point out restrictions,
and give examples and counter-examples.

When adding a new instructions, the following points should be considered:

1. Do I need a new instruction or can an existing instruction be augmented?
2. May the instruction appear in collective regions, SPMD regions, or in both region kinds ("mixed")?
3. Does the new grammar lead to ambiguities while parsing? (If yes, then Bison will complain :-).)

During development it is helpful to view the documentation locally:
Create a python venv and install the packages in ``docs/requirements.txt``.
Configure TinyTC to build the documentation by passing ``-DBUILD_DOCUMENTATION=ON`` to CMake.
Then browse to the documentation's build folder (e.g. ``build/docs/_build``) and run
``python3 -m http.server``. View the documentation in a browser (``http://localhost:8000/``).

Add the instruction's scaffolding
=================================

In TinyTC, we generate an instruction's boilerplate code from an instruction specification file
located in ``include/tinytc/instructions.anko``.
The .anko file is later used as input in .mochi files that are run through mochi
(the **m**\ eta **o**\ bject **ch**\ omp\ **i**\ ler).
For example, in the file ``src/node/visit.hpp.mochi`` we find the placeholder

.. code::

    // もち visit_hpp "tinytc/instructions.anko"

that instructs mochi to generate the visitor boilerplate code, taking ``instructions.anko`` as input.
After build, the generated code is found in ``build/src/node/visit.hpp``.


For subview, we need to add the following lines to ``instructions.anko``:

.. code:: 

    inst @subview "Subview instruction" {
        prop* %static_offsets => i64 "static offsets (need to add value to dynamic offsets "
                                     "if static_offsets[i] == TINYTC_DYNAMIC)"
        prop* %static_sizes   => i64 "static sizes (need to add value to dynamic sizes "
                                     "if static_sizes[i] == TINYTC_DYNAMIC)"
        op %operand                  "operand"
        op* %offsets                 "dynamic offsets"
        op* %sizes                   "dynamic sizes"
        ret %result                  "resulting memref type"
    }

In the first line we declare an instruction named "subview" and immediately afterwards we add
a short description in quotes.
Inside the instructions body (``{...}``) we add properties, operands, return values, and child regions.
All four kinds may be augmented with the qualifier ``*`` or ``?``, where a star means we have zero or more
of the kind (vector), and a question mark indicates zero or one of the kind (optional).
Omitting the qualifier means we expect exactly one of the kind.

Properties
----------

A property is a compile-time constant modifier of an instruction.
It is initialized with the "prop" keyword, followed by the property's name, it's type (after ``=>``),
and a short description.
The type must be one of the builtin types (bool, i32, i64, type_t), an enum type defined in
``include/tinytc/enums.anko`` preceded with @ (e.g. @transpose), or a string in quotes that
gives an arbitrary C++ type name.

Here, we have the properties static_offsets and static_sizes, both coming with the "*" qualifier,
as we want to support arbitray dimensional shapes.
The properties encode either compile time slices or encode that an offset or a size must be passed
as operand (via the special value TINYTC_DYNAMIC).

Operands
--------

Operands are references to SSA values (i.e. ``tinytc_value_t``).
Note that at this level we just give names to the operands (and qualify them with "*" or "?").
Any kind of validation rules, e.g. the number of expected offset and size SSA values, 
must be implemented in the ``setup_and_check`` routine discussed futher below.

Results
-------

TinyTC instructions may return one or multiple values.
Similarly to operands, here we just give names to the results and qualify the number of returned values.

Child regions
-------------

A subview does not need child regions, but in the ``@if`` instruction we find

.. code::

    reg %then      "then block"
    reg %otherwise "else block"

which defines the two branch regions the if may take.

Add verification rules
======================

After adding the subview to ``instructions.anko`` and building we find the new function ``subview_inst::create``
in the generated file ``build/src/node/inst_view.cpp``.
The new function constructs a ``tinytc_inst_t`` object based on given properties, operands, and result types.
Note that a ``tinytc_inst_t`` is a generic container for all instructions;
the ``subview_inst`` class, found in ``build/src/node/inst_view.hpp`` is a view on a ``tinytc_inst_t`` but
does not hold any properties, operands, or results.

The last call in ``subview_inst::create`` is to ``subview_inst::setup_and_check``.
We must implement that function in ``src/node/inst_view_impl.cpp``.
One of the checks, e.g., is that ``static_offsets`` and ``static_sizes`` have the same dimension
as the dimension of the operand's memref.
The convention is that if a check fails we throw a ``compilation_error``, anotating the source code location:

.. code:: c++

    auto [ot, rt] = get_and_check_memref_type_addrspace(operand(), ty, loc());

    if (ot->dim() != static_cast<std::int64_t>(static_offsets().size()) ||
        ot->dim() != static_cast<std::int64_t>(static_sizes().size())) {
        throw compilation_error(loc(), status::ir_invalid_number_of_indices);
    }

Error codes are defined in the ``@status`` enum defined in ``include/tinytc/enums.anko``.

Update compiler passes
======================

When adding a new instruction we must check which compiler passes are affected and must be affected.
The ``src/pass/dump_ir.cpp`` is responsible for dumping an AST to a string, and must always be updated,
as there is no default implementation.

Moreover, a subview is a memory view operation and we must update the alias analysis found in
``src/analysis/alias.cpp`` and the GCD analysis in ``src/analysis/gcd.cpp``.
To identify compiler passes that need updating it is generally recommended to look for an instruction that is
conceptually similar (e.g. memory view instructions or arithmetic instructions) and then ``grep`` the codebase
for for passes or analysis where the similar instruction is touched.

Finally, for code generation we must update ``src/spv/converter.cpp`` that implements rules converting
the TinyTC instruction to a SPIR-V instruction.
Updating the SPIR-V converter is only needed if there is lowering implemented elsewhere.
For example, the ``foreach_tile`` instruction is converted to regular ``for`` loops in the
``lower_foreach_pass`` and thus is no longer part of the AST when converting to SPIR-V.

Update lexer and parser
=======================

Lastly, we want to update the lexer and parser to deal with the new instruction.
The lexer generator is re2c and the parser generator is good ol' bison.

We first update the lexer in ``src/parser/lexer.re`` and add new keywords.
As a minimum, the instruction's mnemonic must be added, here

.. code::

    "subview"            { adv_loc(); return parser::make_SUBVIEW(loc_); }

Next we open ``src/parser/parser_impl.yy``.
The SUBVIEW token must be added, which also makes bison provide the ``make_SUBVIEW`` function.

.. code::

    %token
        ...
        SUBVIEW "subview"
        ...
    ;

Then, we need to update the ``valued_inst`` rule for parsing the subview instruction:

.. code:: bison

    valued_inst:
        SUBVIEW var LSQBR optional_slice_list RSQBR COLON data_type[ty] {
            yytry(ctx, [&] {
                ...
                $$ = subview_inst::create(std::move(static_offsets), std::move(static_sizes), std::move($var),
                                          std::move(offsets), std::move(sizes), std::move($ty), @valued_inst); 
            });
        }

Note that we wrap the code in ``yytry`` that is going to catch exceptions thrown by ``subview_inst::create``
and properly reports the error.


















