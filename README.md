<h1 align="center">
  Tiny Tensor Compiler
</h1>

<h4 align="center">
  A compiler for tensor computations on GPUs and other devices, supporting the OpenCL, Level Zero, and SYCL runtime.
</h4>

<p align="center">
  <a href="#introduction">Introduction</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#license">License</a>
</p>

## Introduction

The Tiny Tensor Compiler compiles programs written in a domain-specific tensor language to
OpenCL-C and provides methods to run these programs on GPU devices and other devices via OpenCL,
Level Zero, or SYCL.

The tensor language allows writing device-agnostic tensor programs and a large class of tensor
operations can be implemented in terms of loop constructs, subviews, and BLAS-like instructions
(e.g. GEMM, GER, or AXPBY).
High performance is achieved by providing highly optimized BLAS primitives, in particular
through a GEMM code generator, that can be specialized for small GEMMs or uncommon shapes.

## Documentation

The online documentation is available at https://intel.github.io/tiny-tensor-compiler/

## License

[BSD 3-Clause License](LICENSE.md)
