#ifndef XE_CONSTANTS_20250219_HPP
#define XE_CONSTANTS_20250219_HPP

#include <cstdint>

namespace tinytc::spv::xe {
constexpr static std::int32_t grf_size = 64;
constexpr static std::int32_t exec_size = 16;
constexpr static std::int32_t channel_size = 4;
constexpr static std::int32_t sdepth = 8;
constexpr static std::int32_t rcount = 8;
constexpr static std::int32_t load_batch_size = 4;
constexpr static std::int32_t store_batch_size = 1;
} // namespace tinytc::spv::xe

#endif // XE_CONSTANTS_20250219_HPP
