#pragma once

#include <cstddef>
#include <cstdint>

namespace bluestl {

// MurmurHash3 32bit版の簡易実装
constexpr std::uint32_t murmur3_32(const void* key, std::size_t len, std::uint32_t seed = 0) noexcept {
    const std::uint8_t* data = static_cast<const std::uint8_t*>(key);
    std::uint32_t h = seed;
    constexpr std::uint32_t c1 = 0xcc9e2d51;
    constexpr std::uint32_t c2 = 0x1b873593;
    std::size_t nblocks = len / 4;

    // 本体
    for (std::size_t i = 0; i < nblocks; ++i) {
        std::uint32_t k =
            static_cast<std::uint32_t>(data[i * 4 + 0]) | (static_cast<std::uint32_t>(data[i * 4 + 1]) << 8) |
            (static_cast<std::uint32_t>(data[i * 4 + 2]) << 16) | (static_cast<std::uint32_t>(data[i * 4 + 3]) << 24);
        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;
        h ^= k;
        h = (h << 13) | (h >> (32 - 13));
        h = h * 5 + 0xe6546b64;
    }
    // 残り
    std::uint32_t k1 = 0;
    switch (len & 3) {
        case 3:
            k1 ^= data[len - 3] << 16;
            [[fallthrough]];
        case 2:
            k1 ^= data[len - 2] << 8;
            [[fallthrough]];
        case 1:
            k1 ^= data[len - 1];
            k1 *= c1;
            k1 = (k1 << 15) | (k1 >> (32 - 15));
            k1 *= c2;
            h ^= k1;
    }
    // 終端処理
    h ^= static_cast<std::uint32_t>(len);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

template <typename T>
constexpr std::uint32_t hash_murmur(const T& value, std::uint32_t seed = 0) noexcept {
    return murmur3_32(&value, sizeof(T), seed);
}

// C文字列（const char*）向けの特殊化
inline std::uint32_t hash_murmur(const char* str, std::uint32_t seed = 0) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return murmur3_32(str, len, seed);
}

// MurmurHash3 64bit版の簡易実装
constexpr std::uint64_t murmur3_64(const void* key, std::size_t len, std::uint64_t seed = 0) noexcept {
    const std::uint8_t* data = static_cast<const std::uint8_t*>(key);
    std::uint64_t h = seed;
    constexpr std::uint64_t c1 = 0x87c37b91114253d5ULL;
    constexpr std::uint64_t c2 = 0x4cf5ad432745937fULL;
    std::size_t nblocks = len / 8;
    for (std::size_t i = 0; i < nblocks; ++i) {
        std::uint64_t k =
            static_cast<std::uint64_t>(data[i * 8 + 0]) | (static_cast<std::uint64_t>(data[i * 8 + 1]) << 8) |
            (static_cast<std::uint64_t>(data[i * 8 + 2]) << 16) | (static_cast<std::uint64_t>(data[i * 8 + 3]) << 24) |
            (static_cast<std::uint64_t>(data[i * 8 + 4]) << 32) | (static_cast<std::uint64_t>(data[i * 8 + 5]) << 40) |
            (static_cast<std::uint64_t>(data[i * 8 + 6]) << 48) | (static_cast<std::uint64_t>(data[i * 8 + 7]) << 56);
        k *= c1;
        k = (k << 31) | (k >> (64 - 31));
        k *= c2;
        h ^= k;
        h = (h << 27) | (h >> (64 - 27));
        h = h * 5 + 0x52dce729;
    }
    // 残り
    std::uint64_t k1 = 0;
    switch (len & 7) {
        case 7:
            k1 ^= static_cast<std::uint64_t>(data[len - 7]) << 48;
            [[fallthrough]];
        case 6:
            k1 ^= static_cast<std::uint64_t>(data[len - 6]) << 40;
            [[fallthrough]];
        case 5:
            k1 ^= static_cast<std::uint64_t>(data[len - 5]) << 32;
            [[fallthrough]];
        case 4:
            k1 ^= static_cast<std::uint64_t>(data[len - 4]) << 24;
            [[fallthrough]];
        case 3:
            k1 ^= static_cast<std::uint64_t>(data[len - 3]) << 16;
            [[fallthrough]];
        case 2:
            k1 ^= static_cast<std::uint64_t>(data[len - 2]) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= static_cast<std::uint64_t>(data[len - 1]);
            k1 *= c1;
            k1 = (k1 << 31) | (k1 >> (64 - 31));
            k1 *= c2;
            h ^= k1;
    }
    // 終端処理
    h ^= static_cast<std::uint64_t>(len);
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

template <typename T>
constexpr std::uint64_t hash_murmur64(const T& value, std::uint64_t seed = 0) noexcept {
    return murmur3_64(&value, sizeof(T), seed);
}

// 64bit版も同様に特殊化
inline std::uint64_t hash_murmur64(const char* str, std::uint64_t seed = 0) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return murmur3_64(str, len, seed);
}

#if defined(BLUESTL_USE_STD_STRING_HASH)

// std::string向けの特殊化
#include <string>
template <>
inline std::uint32_t hash_murmur<std::string>(const std::string& value, std::uint32_t seed) noexcept {
    return murmur3_32(value.data(), value.size(), seed);
}

// std::string_view向けの特殊化
#include <string_view>
template <>
inline std::uint32_t hash_murmur<std::string_view>(const std::string_view& value, std::uint32_t seed) noexcept {
    return murmur3_32(value.data(), value.size(), seed);
}

template <>
inline std::uint64_t hash_murmur64<std::string>(const std::string& value, std::uint64_t seed) noexcept {
    return murmur3_64(value.data(), value.size(), seed);
}

template <>
inline std::uint64_t hash_murmur64<std::string_view>(const std::string_view& value, std::uint64_t seed) noexcept {
    return murmur3_64(value.data(), value.size(), seed);
}

#endif  // BLUESTL_USE_STD_STRING_HASH

}  // namespace bluestl
