# Ozaki scheme example

Example demonstrates the implementation of the Ozaki scheme following

DGEMM on Integer Matrix Multiplication Unit, Hiroyuki Ootomo and Katsuhisa Ozaki and Rio Yokota,
2024, arXiv:arXiv:2306.11975v4.

The implementation splits a f32 or f64 matrix into i8 matrices, the number of splits controlling the accuracy.
The i8 matrices are multiplied with i32 accumulation and mapped to fast DPAS instructions.

Matrices are currently limited to be square and a multiple of 256x256.
The size is given by -N.
The number of splits is controlled by the -s option and one may choose SGEMM with -ff32 and DGEMM with -ff64.
Moreover, the distribution of the exponent can be controlled by the --phi option
(higher phi means more splits are needed to retain the accuracy).

Please set
```
export IGC_EnableSelectiveScalarizer=1
export NEO_CACHE_PERSISTENT=0
```
for best performance. 
