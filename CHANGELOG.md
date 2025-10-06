# Changelog

## [0.4.0] - 2025-10-06

### Major changes
* Support for complex numbers (c32 and c64) with optimized complex GEMM
* Support for half precision and bfloat16 numbers
* (Experimental) Support for accelerating low precision GEMMs with tensor cores (XMX)
* Compiler directly generates SPIR-V binaries (instead of OpenCL-C); JIT compilation speed-up of about 2x
* Removed dependency on clir and ocloc libraries
* Added preprocessor

### Tiny Tensor Language
* Clarified execution model
* Added align attribute
* Added unroll attribute
* Added boolean, f16 (half precision), bf16 (bfloat16), c32 (single complex), and c64 (double complex) type
* Added address space to memref type
* Added cooperative matrix type
* Added cooperative matrix instructions (prefix "cooperative_matrix_"): apply, atomic_load, atomic_store,
  atomic_update, construct, extract, insert, load, mul_add, reduce, scale, store
* Added new instructions: associated, barrier, constant, cumsum, foreach_tile, parallel, subgroup operations
* Extended foreach for iteration spaces with dim > 1
* For-loops can yield values
* Removed immediate operands in favour of constant instruction
* Instructions annotate the return type instead of operand types
* Change 1d group id to 3d group id; introduce 2d subgroup id

### Core API
* Introduced compiler_context class for compilations flags and control of error reporting
* Removed functions to create kernel bundle from source text
* Removed source_context in favour of compiler_context
* Added functions to compile tinytc prog to SPIR-V
* Added API to run function passes
* Added float <-> half and float <-> bfloat16 conversion functions
* Added array_view class in C++-API

### Builder API
* Changed ownership model for data type, inst, region, and func
* Normalized function names in builder API
* Instruction creation function always include return type

### Compiler
* Overhauled infrastructure for writing advanced compiler passes
* Introduced data flow analysis to properly insert barriers automatically when for-loops are present
* Added constant folding, constant propagation, and dead code elimination passes
* Added memref alignment analysis
* Added analysis to check applicability of tensor core acceleration
* Added SPIR-V support; Tiny Tensor Language to SPIR-V conversion and SPIR-V binary generation
* Introduced GEMM to cooperative matrix conversion (instead of direct GEMM to OpenCL-C conversion)
* Added fast cooperative matrix mul add implementation for complex numbers
* Added attributes
* Added "mochi" compiler-compiler to generate boilerplate for instructions, types, and enums

## [0.3.1] - 2024-05-22
* Bugfix: Add alias analysis for stack; needed to correctly insert barriers
* Bugfix: Disable block writes as alignment analysis is missing

## [0.3.0] - 2024-05-16

* Added offset parameter to group type
* Deploy FindOpenCL and FindLevelZero script for finding tinytc's dependencies via CMake
* Replaced concept by SFINAE such that tinytc.hpp can be used in C++17 projects
* Improved heuristic for subgroup size selection
* Refactored kernel compilation workflow such that libocloc is only used by libtinytc\_ze
  and libtinytc\_cl uses the built-in compiler; libtinytc and libtinytc\_cl can be built without libocloc
* Added functions to query device support level
* Added generic core info class for non-Intel devices
* cl\_mem object is passed by value instead of by pointer for recipes
* Removed tinytc\_mem\_t; memory object type is part of set\_args signature
* The index type was changed to i64 (from u32); attention: the calling convention is updated
* Add function to create tall and skinny recipe with additional specialization constants
* Fix uninitialized value bug in GEMM generator
* Use relaxed ordering for atomics
* Remove unsigned integer types
* Prefix arithmetic instructions with "arith."
* Add binary arithmetic (and, or, xor, not)

## [0.2.0] - 2024-04-26

* Refactored API and added C-API

## [0.1.0] - 2024-04-05

Initial release
