#pragma once

#include <cstdlib>  // malloc, free
#include <new>      // std::nothrow
#include <string>
#include <type_traits>  // std::true_type, std::false_type
#include <limits>       // std::numeric_limits
#include "bluestl/log_macros.h"

// テスト用のアロケータ
// どのくらいメモリが確保・解放されたかを追跡する
template <typename T>
class TestAllocator {
   public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using void_pointer = void*;              // 必要な型を追加
    using const_void_pointer = const void*;  // 必要な型を追加

    // コンテナ操作時にアロケータインスタンスを伝播させるか
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    // アロケータのインスタンスが常に等しいか (ステートフルなのでfalse)
    using is_always_equal = std::false_type;

    // 異なる型 U のためのアロケータ型を提供するメタ関数
    template <typename U>
    struct rebind {
        using other = TestAllocator<U>;
    };

    explicit TestAllocator(const char* name = "TestAllocator") noexcept
        : m_name(name), m_allocated_bytes(0), m_allocation_count(0), m_deallocation_count(0) {
        BLUESTL_LOG_INFO("TestAllocator '{}' created.\n", m_name.c_str());
    }

    // コピーコンストラクタ
    TestAllocator(const TestAllocator& other) noexcept
        : m_name(other.m_name),
          m_allocated_bytes(0),  // 状態はコピーしない (各インスタンスで追跡)
          m_allocation_count(0),
          m_deallocation_count(0) {
        BLUESTL_LOG_INFO("TestAllocator '{}' copied (state not copied).\n", m_name.c_str());
    }

    // ムーブコンストラクタ
    TestAllocator(TestAllocator&& other) noexcept
        : m_name(std::move(other.m_name)),
          m_allocated_bytes(other.m_allocated_bytes),
          m_allocation_count(other.m_allocation_count),
          m_deallocation_count(other.m_deallocation_count) {
        BLUESTL_LOG_INFO("TestAllocator '{}' moved.\n", m_name.c_str());
        // ムーブ元はリセット
        other.m_allocated_bytes = 0;
        other.m_allocation_count = 0;
        other.m_deallocation_count = 0;
    }

    // 他の型からのrebindコンストラクタ
    template <typename U>
    TestAllocator(const TestAllocator<U>& other) noexcept
        : m_name(other.get_name()), m_allocated_bytes(0), m_allocation_count(0), m_deallocation_count(0) {
        BLUESTL_LOG_INFO("TestAllocator '{}' rebind constructed.\n", m_name.c_str());
    }

    // コピー代入
    TestAllocator& operator=(const TestAllocator& other) noexcept {
        if (this != &other) {
            m_name = other.m_name;
            // 状態はコピーしない
            m_allocated_bytes = 0;
            m_allocation_count = 0;
            m_deallocation_count = 0;
            BLUESTL_LOG_INFO("TestAllocator '{}' copy assigned (state not copied).\n", m_name.c_str());
        }
        return *this;
    }

    // ムーブ代入
    TestAllocator& operator=(TestAllocator&& other) noexcept {
        if (this != &other) {
            m_name = std::move(other.m_name);
            m_allocated_bytes = other.m_allocated_bytes;
            m_allocation_count = other.m_allocation_count;
            m_deallocation_count = other.m_deallocation_count;
            BLUESTL_LOG_INFO("TestAllocator '{}' move assigned.\n", m_name.c_str());
            // ムーブ元はリセット
            other.m_allocated_bytes = 0;
            other.m_allocation_count = 0;
            other.m_deallocation_count = 0;
        }
        return *this;
    }

    ~TestAllocator() {
        BLUESTL_LOG_INFO(
            "TestAllocator '{}' destroyed. Final allocated bytes: {}, Allocations: {}, Deallocations: {}\n",
            m_name.c_str(), m_allocated_bytes, m_allocation_count, m_deallocation_count);
        // 解放漏れチェック (テストによっては解放漏れを意図する場合もあるため、アサートはしない)
        // BLUESTL_ASSERT(m_allocated_bytes == 0);
        // BLUESTL_ASSERT(m_allocation_count == m_deallocation_count);
    }

    // メモリ確保
    [[nodiscard]] pointer allocate(size_type n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
            BLUESTL_LOG_ERROR("TestAllocator '{}': Allocation size overflow: {} elements\n", m_name.c_str(), n);
            // Bluestlは例外を投げないので、ヌルポインタを返すかアサートする
            BLUESTL_ASSERT(false);  // ここで停止させるのが安全か
            return nullptr;
        }
        size_t bytes = n * sizeof(T);
        pointer p = static_cast<pointer>(std::malloc(bytes));
        if (!p) {
            BLUESTL_LOG_ERROR("TestAllocator '{}': Failed to allocate {} bytes.\n", m_name.c_str(), bytes);
            // Bluestlは例外を投げないので、ヌルポインタを返すかアサートする
            BLUESTL_ASSERT(false);  // ここで停止させるのが安全か
            return nullptr;
        }
        m_allocated_bytes += bytes;
        m_allocation_count++;
        BLUESTL_LOG_INFO("TestAllocator '{}': Allocated {} bytes at %p ({} elements). Total allocated: {}\n",
                         m_name.c_str(), bytes, static_cast<void*>(p), n, m_allocated_bytes);
        return p;
    }

    // メモリ解放
    void deallocate(pointer p, size_type n) noexcept {
        if (!p) return;
        size_t bytes = n * sizeof(T);
        m_allocated_bytes -= bytes;
        m_deallocation_count++;
        BLUESTL_LOG_INFO("TestAllocator '{}': Deallocating {} bytes at %p ({} elements). Total allocated: {}\n",
                         m_name.c_str(), bytes, static_cast<void*>(p), n, m_allocated_bytes);
        std::free(p);
    }

    // 統計情報
    size_t get_allocated_bytes() const noexcept {
        return m_allocated_bytes;
    }
    size_t get_allocation_count() const noexcept {
        return m_allocation_count;
    }
    size_t get_deallocation_count() const noexcept {
        return m_deallocation_count;
    }
    const std::string& get_name() const noexcept {
        return m_name;
    }

    // 比較演算子 (インスタンスのアドレスを比較)
    bool operator==(const TestAllocator& other) const noexcept {
        return this == &other;
    }

    bool operator!=(const TestAllocator& other) const noexcept {
        return !(*this == other);
    }

   private:
    std::string m_name;
    size_t m_allocated_bytes;
    size_t m_allocation_count;
    size_t m_deallocation_count;
};
