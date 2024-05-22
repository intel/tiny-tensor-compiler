# Changelog

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
