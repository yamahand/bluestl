#pragma once

#include <cstddef>
#include <cstdint>

namespace bluestl {

// xxHash32の簡易実装（参考用、十分な分布と速度を持つ）
constexpr std::uint32_t xxhash32(const void* data, std::size_t len, std::uint32_t seed = 0) noexcept {
    constexpr std::uint32_t PRIME1 = 2654435761U;
    constexpr std::uint32_t PRIME2 = 2246822519U;
    constexpr std::uint32_t PRIME3 = 3266489917U;
    constexpr std::uint32_t PRIME4 = 668265263U;
    constexpr std::uint32_t PRIME5 = 374761393U;

    const unsigned char* p = static_cast<const unsigned char*>(data);
    const unsigned char* const bEnd = p + len;
    std::uint32_t h32 = seed + PRIME5;
    h32 += static_cast<std::uint32_t>(len);

    while (p + 4 <= bEnd) {
        std::uint32_t k1 = static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
                           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
        k1 *= PRIME3;
        k1 = (k1 << 17) | (k1 >> (32 - 17));
        k1 *= PRIME4;
        h32 ^= k1;
        h32 = (h32 << 19) | (h32 >> (32 - 19));
        h32 = h32 * 5 + 0xe6546b64U;
        p += 4;
    }
    while (p < bEnd) {
        h32 ^= (*p) * PRIME5;
        h32 = (h32 << 11) | (h32 >> (32 - 11));
        h32 *= PRIME1;
        ++p;
    }
    h32 ^= h32 >> 15;
    h32 *= PRIME2;
    h32 ^= h32 >> 13;
    h32 *= PRIME3;
    h32 ^= h32 >> 16;
    return h32;
}

template <typename T>
constexpr std::uint32_t hash_xx(const T& value, std::uint32_t seed = 0) noexcept {
    return xxhash32(&value, sizeof(T), seed);
}

// xxHash64の簡易実装
constexpr std::uint64_t xxhash64(const void* data, std::size_t len, std::uint64_t seed = 0) noexcept {
    constexpr std::uint64_t PRIME1 = 11400714785074694791ULL;
    constexpr std::uint64_t PRIME2 = 14029467366897019727ULL;
    constexpr std::uint64_t PRIME3 = 1609587929392839161ULL;
    constexpr std::uint64_t PRIME4 = 9650029242287828579ULL;
    constexpr std::uint64_t PRIME5 = 2870177450012600261ULL;
    const unsigned char* p = static_cast<const unsigned char*>(data);
    const unsigned char* const bEnd = p + len;
    std::uint64_t h64 = seed + PRIME5 + len;
    while (p + 8 <= bEnd) {
        std::uint64_t k1 = static_cast<std::uint64_t>(p[0]) | (static_cast<std::uint64_t>(p[1]) << 8) |
                           (static_cast<std::uint64_t>(p[2]) << 16) | (static_cast<std::uint64_t>(p[3]) << 24) |
                           (static_cast<std::uint64_t>(p[4]) << 32) | (static_cast<std::uint64_t>(p[5]) << 40) |
                           (static_cast<std::uint64_t>(p[6]) << 48) | (static_cast<std::uint64_t>(p[7]) << 56);
        k1 *= PRIME2;
        k1 = (k1 << 31) | (k1 >> (64 - 31));
        k1 *= PRIME1;
        h64 ^= k1;
        h64 = (h64 << 27) | (h64 >> (64 - 27));
        h64 = h64 * PRIME1 + PRIME4;
        p += 8;
    }
    while (p < bEnd) {
        h64 ^= (*p) * PRIME5;
        h64 = (h64 << 11) | (h64 >> (64 - 11));
        h64 *= PRIME1;
        ++p;
    }
    h64 ^= h64 >> 33;
    h64 *= PRIME2;
    h64 ^= h64 >> 29;
    h64 *= PRIME3;
    h64 ^= h64 >> 32;
    return h64;
}

template <typename T>
constexpr std::uint64_t hash_xx64(const T& value, std::uint64_t seed = 0) noexcept {
    return xxhash64(&value, sizeof(T), seed);
}

// C文字列（const char*）向けの特殊化
inline std::uint32_t hash_xx(const char* str, std::uint32_t seed = 0) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return xxhash32(str, len, seed);
}

// 64bit版も同様に特殊化
inline std::uint64_t hash_xx64(const char* str, std::uint64_t seed = 0) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return xxhash64(str, len, seed);
}

#if defined(BLUESTL_USE_STD_STRING_HASH)

// std::string向けの特殊化
#include <string>
template <>
inline std::uint32_t hash_xx<std::string>(const std::string& value, std::uint32_t seed) noexcept {
    return xxhash32(value.data(), value.size(), seed);
}

// std::string_view向けの特殊化
#include <string_view>
template <>
inline std::uint32_t hash_xx<std::string_view>(const std::string_view& value, std::uint32_t seed) noexcept {
    return xxhash32(value.data(), value.size(), seed);
}

template <>
inline std::uint64_t hash_xx64<std::string>(const std::string& value, std::uint64_t seed) noexcept {
    return xxhash64(value.data(), value.size(), seed);
}

template <>
inline std::uint64_t hash_xx64<std::string_view>(const std::string_view& value, std::uint64_t seed) noexcept {
    return xxhash64(value.data(), value.size(), seed);
}

#endif  // BLUESTL_USE_STD_STRING_HASH

}  // namespace bluestl
