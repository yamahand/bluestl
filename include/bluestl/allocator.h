#pragma once

#include "log_macros.h"
#include "assert_handler.h"
#include <cstddef> // size_t, ptrdiff_t
#include <cstdlib> // malloc, free
#include <cstdint> // uintptr_t
#include <limits>  // numeric_limits
#include <new>     // new
#include <type_traits> // false_type, true_type

#if defined(_WIN32) && defined(_MSC_VER)
#include <malloc.h> // _aligned_malloc, _aligned_free
#else
#include <cstdlib>  // posix_memalign, aligned_alloc
// POSIX環境でposix_memalignの宣言を確実にする
extern "C" int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif

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
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // アロケータ伝播特性（標準allocatorに合わせてfalse）
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using is_always_equal = std::true_type;

    // 異なる型 U のためのアロケータ型を提供するメタ関数
    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };

    // デフォルトコンストラクタ
    allocator() noexcept = default;
    // コピーコンストラクタ (他の型のアロケータからも構築可能)
    template <typename U>
    allocator(const allocator<U>&) noexcept {}    // メモリ確保
    [[nodiscard]] pointer allocate(size_type n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
             BLUESTL_LOG_ERROR("allocator: Allocation size overflow: %zu elements\\n", n);
             BLUESTL_ASSERT(false);
             return nullptr;
        }

        size_t bytes = n * sizeof(T);
        size_t alignment = alignof(T);

        // 標準アライメント（通常8バイト）を超える場合はアライメント指定メモリ確保を使用
        if (alignment > alignof(std::max_align_t)) {
            return allocate_aligned(n, alignment);
        }

        pointer p = static_cast<pointer>(std::malloc(bytes));
        if (!p) {
            BLUESTL_LOG_ERROR("allocator: Failed to allocate %zu bytes.\\n", bytes);
            BLUESTL_ASSERT(false);
            return nullptr;
        }
        BLUESTL_LOG_DEBUG("allocator: Allocated %zu bytes at %p (%zu elements).\\n", bytes, static_cast<void*>(p), n);
        return p;
    }

    // アライメント指定メモリ確保
    [[nodiscard]] pointer allocate_aligned(size_type n, size_type alignment) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
             BLUESTL_LOG_ERROR("allocator: Aligned allocation size overflow: %zu elements\\n", n);
             BLUESTL_ASSERT(false);
             return nullptr;
        }

        // アライメントが2の冪であることを確認
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            BLUESTL_LOG_ERROR("allocator: Invalid alignment: %zu (must be power of 2)\\n", alignment);
            BLUESTL_ASSERT(false);
            return nullptr;
        }        size_t bytes = n * sizeof(T);

        // アライメントがsizeof(T)未満の場合はデフォルトのアライメントを使用
        if (alignment < alignof(T)) {
            alignment = alignof(T);
        }

        // バイト数をアライメントの倍数に調整（一部のアライメント関数で必要）
        if (bytes % alignment != 0) {
            bytes = ((bytes + alignment - 1) / alignment) * alignment;
        }

#if defined(_WIN32) && defined(_MSC_VER)
        pointer p = static_cast<pointer>(_aligned_malloc(bytes, alignment));
#elif defined(__GNUC__) || defined(__clang__)
        pointer p = static_cast<pointer>(std::aligned_alloc(alignment, bytes));
#else
        // フォールバック: mallocとマニュアルアライメント調整
        void* raw_ptr = std::malloc(bytes + alignment + sizeof(void*));
        if (!raw_ptr) {
            p = nullptr;
        } else {
            // アライメント調整
            uintptr_t addr = reinterpret_cast<uintptr_t>(raw_ptr) + sizeof(void*);
            uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
            p = reinterpret_cast<pointer>(aligned_addr);
            // 元のポインタを保存（deallocate_alignedで使用）
            *(reinterpret_cast<void**>(aligned_addr) - 1) = raw_ptr;
        }
#endif

        if (!p) {
            BLUESTL_LOG_ERROR("allocator: Failed to allocate %zu aligned bytes (alignment=%zu).\\n", bytes, alignment);
            BLUESTL_ASSERT(false);
            return nullptr;
        }

        BLUESTL_LOG_DEBUG("allocator: Allocated %zu aligned bytes at %p (%zu elements, alignment=%zu).\\n",
                         bytes, static_cast<void*>(p), n, alignment);
        return p;
    }    // メモリ解放
    void deallocate(pointer p, size_type n) noexcept {
        if (!p) return;

        size_t bytes = n * sizeof(T); // デバッグ用にサイズを計算
        size_t alignment = alignof(T);

        BLUESTL_LOG_DEBUG("allocator: Deallocating %zu bytes at %p (%zu elements).\\n", bytes, static_cast<void*>(p), n);

        // 標準アライメントを超える場合はアライメント指定解放を使用
        if (alignment > alignof(std::max_align_t)) {
            deallocate_aligned(p, n);
        } else {
            std::free(p);
        }
    }

    // アライメント指定メモリ解放
    void deallocate_aligned(pointer p, size_type n) noexcept {
        if (!p) return;

        size_t bytes = n * sizeof(T); // デバッグ用にサイズを計算
        BLUESTL_LOG_DEBUG("allocator: Deallocating %zu aligned bytes at %p (%zu elements).\\n",
                         bytes, static_cast<void*>(p), n);

#if defined(_WIN32) && defined(_MSC_VER)
        _aligned_free(p);
#elif defined(__GNUC__) || defined(__clang__)
        std::free(p);
#else
        // フォールバック: 保存されている元のポインタを使用
        void* raw_ptr = *(reinterpret_cast<void**>(p) - 1);
        std::free(raw_ptr);
#endif
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
