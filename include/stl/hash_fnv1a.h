#pragma once

#include <cstddef>
#include <cstdint>

namespace bluestl {

constexpr std::uint32_t fnv1a_hash(const void* data, std::size_t size) noexcept {
    constexpr std::uint32_t FNV_offset_basis = 2166136261u;
    constexpr std::uint32_t FNV_prime = 16777619u;
    std::uint32_t hash = FNV_offset_basis;
    const unsigned char* ptr = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= ptr[i];
        hash *= FNV_prime;
    }
    return hash;
}

template <typename T>
constexpr std::uint32_t hash_fnv1a(const T& value) noexcept {
    return fnv1a_hash(&value, sizeof(T));
}

constexpr std::uint64_t fnv1a_hash64(const void* data, std::size_t size) noexcept {
    constexpr std::uint64_t FNV_offset_basis = 14695981039346656037ull;
    constexpr std::uint64_t FNV_prime = 1099511628211ull;
    std::uint64_t hash = FNV_offset_basis;
    const unsigned char* ptr = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= ptr[i];
        hash *= FNV_prime;
    }
    return hash;
}

template <typename T>
constexpr std::uint64_t hash_fnv1a64(const T& value) noexcept {
    return fnv1a_hash64(&value, sizeof(T));
}

// C文字列（const char*）向けの特殊化
inline std::uint32_t hash_fnv1a(const char* str) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return fnv1a_hash(str, len);
}

// 64bit版も同様に特殊化
inline std::uint64_t hash_fnv1a64(const char* str) noexcept {
    if (!str) return 0;
    std::size_t len = 0;
    while (str[len]) ++len;
    return fnv1a_hash64(str, len);
}

#if defined(BLUESTL_USE_STD_STRING_HASH)

// std::string向けの特殊化
#include <string>
template<>
inline std::uint32_t hash_fnv1a<std::string>(const std::string& value) noexcept {
    return fnv1a_hash(value.data(), value.size());
}

// std::string_view向けの特殊化
#include <string_view>
template<>
inline std::uint32_t hash_fnv1a<std::string_view>(const std::string_view& value) noexcept {
    return fnv1a_hash(value.data(), value.size());
}

template<>
inline std::uint64_t hash_fnv1a64<std::string>(const std::string& value) noexcept {
    return fnv1a_hash64(value.data(), value.size());
}

template<>
inline std::uint64_t hash_fnv1a64<std::string_view>(const std::string_view& value) noexcept {
    return fnv1a_hash64(value.data(), value.size());
}

#endif // BLUESTL_USE_STD_STRING_HASH

} // namespace bluestl
