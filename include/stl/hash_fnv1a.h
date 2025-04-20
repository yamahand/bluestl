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

} // namespace bluestl
