#pragma once

#include "log_macros.h"
#include "assert_handler.h"
#include <cstddef> // size_t, ptrdiff_t
#include <cstdlib> // malloc, free
#include <limits>  // numeric_limits
#include <new>     // new

namespace bluestl {

/**
 * @class allocator
 * @brief デフォルトのアロケータクラス。malloc/freeを使用。
 * @tparam T 要素型
 */
template <typename T>
class allocator {
   public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // 異なる型 U のためのアロケータ型を提供するメタ関数
    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };

    // デフォルトコンストラクタ
    allocator() noexcept = default;
    // コピーコンストラクタ (他の型のアロケータからも構築可能)
    template <typename U>
    allocator(const allocator<U>&) noexcept {}

    // メモリ確保
    [[nodiscard]] pointer allocate(size_type n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
             BLUESTL_LOG_ERROR("allocator: Allocation size overflow: %zu elements\\n", n);
             BLUESTL_ASSERT(false);
             return nullptr;
        }
        size_t bytes = n * sizeof(T);
        pointer p = static_cast<pointer>(std::malloc(bytes));
        if (!p) {
            BLUESTL_LOG_ERROR("allocator: Failed to allocate %zu bytes.\\n", bytes);
            BLUESTL_ASSERT(false);
            return nullptr;
        }
        BLUESTL_LOG_DEBUG("allocator: Allocated %zu bytes at %p (%zu elements).\\n", bytes, static_cast<void*>(p), n);
        return p;
    }

    // メモリ解放
    void deallocate(pointer p, size_type n) noexcept {
        if (!p) return;
        size_t bytes = n * sizeof(T); // デバッグ用にサイズを計算
        BLUESTL_LOG_DEBUG("allocator: Deallocating %zu bytes at %p (%zu elements).\\n", bytes, static_cast<void*>(p), n);
        std::free(p);
    }

    // アロケータ同士の比較 (ステートレスなので常に等しい)
    bool operator==(const allocator&) const noexcept { return true; }
    bool operator!=(const allocator&) const noexcept { return false; }

    // construct と destroy は allocator_traits がデフォルトを提供するため、ここでは不要
};

// void 用の特殊化 (必須ではないが、標準に合わせておく)
template <>
class allocator<void> {
public:
    using value_type = void;
    using pointer = void*;
    using const_pointer = const void*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };
};


}  // namespace bluestl
