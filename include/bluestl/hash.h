#pragma once

#include "hash_fnv1a.h"
#include "hash_xx.h"
#include "hash_murmur.h"

// 使用するハッシュアルゴリズムを切り替えるマクロ
// BLUESTL_HASH_ALGO に "fnv1a", "xx", "murmur" のいずれかを指定
#ifndef BLUESTL_HASH_ALGO
#define BLUESTL_HASH_ALGO fnv1a
#endif

namespace bluestl {

#if BLUESTL_HASH_ALGO == fnv1a
    using hash_default_t = std::uint32_t;
    template <typename T>
    constexpr hash_default_t hash(const T& value) noexcept { return hash_fnv1a(value); }
#elif BLUESTL_HASH_ALGO == xx
    using hash_default_t = std::uint32_t;
    template <typename T>
    constexpr hash_default_t hash(const T& value) noexcept { return hash_xx(value); }
#elif BLUESTL_HASH_ALGO == murmur
    using hash_default_t = std::uint32_t;
    template <typename T>
    constexpr hash_default_t hash(const T& value) noexcept { return hash_murmur(value); }
#else
    #error "BLUESTL_HASH_ALGO must be fnv1a, xx, or murmur"
#endif

} // namespace bluestl
