#pragma once

#include "hash_fnv1a.h"
#include "hash_xx.h"
#include "hash_murmur.h"
#include <cstdint>
#include <type_traits>

// 使用するハッシュアルゴリズムを切り替えるマクロ
// BLUESTL_HASH_ALGO に "fnv1a", "xx", "murmur" のいずれかを指定
#ifndef BLUESTL_HASH_ALGO
#define BLUESTL_HASH_ALGO fnv1a
#endif

namespace bluestl {

/**
 * @brief 小さな整数値のハッシュを改善するためのミキサー関数
 * Thomas Wang's 32-bit integer hash function
 */
constexpr std::uint32_t mix_small_hash(std::uint32_t key) noexcept {
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
}

/**
 * @brief 64bit版のミキサー関数
 */
constexpr std::uint64_t mix_small_hash(std::uint64_t key) noexcept {
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

/**
 * @brief 型特性に基づく最適化されたハッシュ関数
 */
template <typename T>
constexpr auto hash_optimized(const T& value) noexcept {
    if constexpr (std::is_integral_v<T> && sizeof(T) <= 4) {
        // 小さな整数値には特別な処理を適用
        auto basic_hash =
#if BLUESTL_HASH_ALGO == fnv1a
            hash_fnv1a(value);
#elif BLUESTL_HASH_ALGO == xx
            hash_xx(value);
#elif BLUESTL_HASH_ALGO == murmur
            hash_murmur(value);
#endif
        return mix_small_hash(static_cast<std::uint32_t>(basic_hash));
    } else if constexpr (std::is_integral_v<T> && sizeof(T) == 8) {
        // 64bit整数値
        auto basic_hash =
#if BLUESTL_HASH_ALGO == fnv1a
            hash_fnv1a(value);
#elif BLUESTL_HASH_ALGO == xx
            hash_xx(value);
#elif BLUESTL_HASH_ALGO == murmur
            hash_murmur(value);
#endif
        return static_cast<std::uint32_t>(mix_small_hash(static_cast<std::uint64_t>(basic_hash)));
    } else {
        // その他の型は通常のハッシュ関数を使用
#if BLUESTL_HASH_ALGO == fnv1a
        return hash_fnv1a(value);
#elif BLUESTL_HASH_ALGO == xx
        return hash_xx(value);
#elif BLUESTL_HASH_ALGO == murmur
        return hash_murmur(value);
#endif
    }
}

#if BLUESTL_HASH_ALGO == fnv1a
using hash_default_t = std::uint32_t;
template <typename T>
constexpr hash_default_t hash(const T& value) noexcept {
    return hash_optimized(value);
}
#elif BLUESTL_HASH_ALGO == xx
using hash_default_t = std::uint32_t;
template <typename T>
constexpr hash_default_t hash(const T& value) noexcept {
    return hash_optimized(value);
}
#elif BLUESTL_HASH_ALGO == murmur
using hash_default_t = std::uint32_t;
template <typename T>
constexpr hash_default_t hash(const T& value) noexcept {
    return hash_optimized(value);
}
#else
#error "BLUESTL_HASH_ALGO must be fnv1a, xx, or murmur"
#endif

}  // namespace bluestl
