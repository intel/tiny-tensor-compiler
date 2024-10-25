// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DOCTEST_UTIL_20241023_HPP
#define DOCTEST_UTIL_20241023_HPP

#define DOCTEST_TENSOR1_TEST(MM)                                                                   \
    do {                                                                                           \
        for (auto mm : MM) {                                                                       \
            DOCTEST_SUBCASE((std::to_string(mm)).c_str()) { M = mm; }                              \
        }                                                                                          \
    } while (false)

#define DOCTEST_TENSOR2_TEST(MM, NN)                                                               \
    do {                                                                                           \
        for (auto nn : NN) {                                                                       \
            for (auto mm : MM) {                                                                   \
                DOCTEST_SUBCASE((std::to_string(mm) + "x" + std::to_string(nn)).c_str()) {         \
                    N = nn;                                                                        \
                    M = mm;                                                                        \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (false)

#define DOCTEST_TENSOR3_TEST(MM, NN, KK)                                                           \
    do {                                                                                           \
        for (auto kk : KK) {                                                                       \
            for (auto nn : NN) {                                                                   \
                for (auto mm : MM) {                                                               \
                    DOCTEST_SUBCASE(                                                               \
                        (std::to_string(mm) + "x" + std::to_string(nn) + "x" + std::to_string(kk)) \
                            .c_str()) {                                                            \
                        K = kk;                                                                    \
                        N = nn;                                                                    \
                        M = mm;                                                                    \
                    }                                                                              \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (false)

#define DOCTEST_TENSOR4_TEST(MM, NN, KK, HH)                                                       \
    do {                                                                                           \
        for (auto hh : HH) {                                                                       \
            for (auto kk : KK) {                                                                   \
                for (auto nn : NN) {                                                               \
                    for (auto mm : MM) {                                                           \
                        DOCTEST_SUBCASE((std::to_string(mm) + "x" + std::to_string(nn) + "x" +     \
                                         std::to_string(kk) + "*" + std::to_string(hh))            \
                                            .c_str()) {                                            \
                            howmany = hh;                                                          \
                            K = kk;                                                                \
                            N = nn;                                                                \
                            M = mm;                                                                \
                        }                                                                          \
                    }                                                                              \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (false)

#ifdef ENABLE_DOUBLE_PRECISION
#define TEST_PRECISIONS float, double
#else
#define TEST_PRECISIONS float
#endif

#endif // DOCTEST_UTIL_20241023_HPP
