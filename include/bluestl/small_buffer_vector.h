// -----------------------------------------------------------------------------
// Bluestl small_buffer_vector.h
// C++20準拠・STL風インターフェースの小容量最適化可変長配列
// -----------------------------------------------------------------------------
/**
 * @file small_buffer_vector.h
 * @brief Bluestlプロジェクトのsmall_buffer_vectorクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * small_buffer_vectorは、少数要素はスタック上の固定バッファで管理し、
 * オーバーフロー時のみヒープ確保に切り替える可変長配列です。
 * - RTTIなし、例外なし、header-only設計
 * - STL std::vector風のインターフェース
 * - 固定バッファサイズはテンプレート引数で指定
 * - allocatorは外部から渡すことが可能
 * - #pragma onceによるインクルードガード
 */
#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>
#include <iterator>
#include "allocator.h"
#include "assert_handler.h"

namespace bluestl {

/**
 * @class small_buffer_vector
 * @brief 小容量最適化可変長配列。STL std::vector風インターフェース。
 * @tparam T 要素型
 * @tparam SmallCapacity 固定バッファ要素数
 * @tparam Allocator アロケータ型
 *
 * - 小容量時はスタック上のバッファを利用
 * - オーバーフロー時のみヒープ確保
 * - allocatorは外部から渡す
 */
template <typename T, std::size_t SmallCapacity, typename Allocator = allocator>
class small_buffer_vector {
   public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /**
     * @brief デフォルトコンストラクタは禁止（アロケータ必須）
     */
    small_buffer_vector() = delete;

    /**
     * @brief アロケータ指定コンストラクタ
     * @param alloc アロケータ
     */
    explicit small_buffer_vector(Allocator& alloc)
        : m_allocator(alloc),
          m_data(reinterpret_cast<T*>(m_small_buffer)),
          m_size(0),
          m_capacity(SmallCapacity),
          m_data_address(reinterpret_cast<uintptr_t>(m_small_buffer)) {}

    // コピー・ムーブコンストラクタ・代入演算子は省略（必要に応じて追加）

    /**
     * @brief デストラクタ
     */
    ~small_buffer_vector() {
        clear();
        deallocate();
    }

    /**
     * @brief 要素数を取得
     */
    [[nodiscard]] size_type size() const noexcept {
        return m_size;
    }
    /**
     * @brief 容量を取得
     */
    [[nodiscard]] size_type capacity() const noexcept {
        return m_capacity;
    }
    /**
     * @brief 空かどうか
     */
    [[nodiscard]] bool empty() const noexcept {
        return m_size == 0;
    }

    /**
     * @brief データポインタ取得
     */
    [[nodiscard]] T* data() noexcept {
        return m_data;
    }
    [[nodiscard]] const T* data() const noexcept {
        return m_data;
    }

    /**
     * @brief 先頭イテレータ
     */
    iterator begin() noexcept {
        return m_data;
    }
    const_iterator begin() const noexcept {
        return m_data;
    }
    iterator end() noexcept {
        return m_data + m_size;
    }
    const_iterator end() const noexcept {
        return m_data + m_size;
    }
    const_iterator cbegin() const noexcept {
        return begin();
    }
    const_iterator cend() const noexcept {
        return end();
    }
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }
    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    /**
     * @brief 添字アクセス
     */
    T& operator[](size_type i) noexcept {
        return m_data[i];
    }
    const T& operator[](size_type i) const noexcept {
        return m_data[i];
    }
    /**
     * @brief 範囲チェック付きアクセス
     */
    T& at(size_type i) {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }
    const T& at(size_type i) const {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }

    /**
     * @brief 末尾に要素を追加
     * @param value 追加する値
     * @return 追加後のサイズ
     */
    size_type push_back(const T& value) {
        reserve(m_size + 1);
        new (m_data + m_size) T(value);
        return ++m_size;
    }
    /**
     * @brief ムーブで末尾に要素を追加
     * @param value 追加する値
     * @return 追加後のサイズ
     */
    size_type push_back(T&& value) {
        reserve(m_size + 1);
        new (m_data + m_size) T(std::move(value));
        return ++m_size;
    }
    /**
     * @brief emplace_back
     */
    template <class... Args>
    size_type emplace_back(Args&&... args) {
        reserve(m_size + 1);
        new (m_data + m_size) T(std::forward<Args>(args)...);
        return ++m_size;
    }
    /**
     * @brief 末尾要素を削除
     */
    void pop_back() {
        if (m_size > 0) {
            --m_size;
            m_data[m_size].~T();
        }
    }
    /**
     * @brief 全要素削除
     */
    void clear() noexcept {
        for (size_type i = 0; i < m_size; ++i) {
            m_data[i].~T();
        }
        m_size = 0;
    }
    /**
     * @brief 容量確保
     * @param new_cap 新しい容量
     */
    void reserve(size_type new_cap) {
        if (new_cap <= m_capacity) return;
        size_type new_capacity = (m_capacity * 2 > new_cap) ? m_capacity * 2 : new_cap;
        T* new_data = static_cast<T*>(m_allocator.allocate(sizeof(T) * new_capacity));
        for (size_type i = 0; i < m_size; ++i) {
            new (new_data + i) T(std::move(m_data[i]));
            m_data[i].~T();
        }
        deallocate();
        m_data = new_data;
        m_capacity = new_capacity;
        m_data_address = reinterpret_cast<uintptr_t>(m_data);
    }
    /**
     * @brief shrink_to_fit: ヒープ確保時のみ容量縮小
     */
    void shrink_to_fit() {
        if (m_data == reinterpret_cast<T*>(m_small_buffer)) return;
        if (m_size <= SmallCapacity) {
            // スタックバッファに戻す
            T* new_data = reinterpret_cast<T*>(m_small_buffer);
            for (size_type i = 0; i < m_size; ++i) {
                new (new_data + i) T(std::move(m_data[i]));
                m_data[i].~T();
            }
            m_allocator.deallocate(m_data, sizeof(T) * m_capacity);
            m_data = new_data;
            m_capacity = SmallCapacity;
        } else if (m_size < m_capacity) {
            // ヒープ上で縮小
            T* new_data = static_cast<T*>(m_allocator.allocate(sizeof(T) * m_size));
            for (size_type i = 0; i < m_size; ++i) {
                new (new_data + i) T(std::move(m_data[i]));
                m_data[i].~T();
            }
            m_allocator.deallocate(m_data, sizeof(T) * m_capacity);
            m_data = new_data;
            m_capacity = m_size;
        }
        m_data_address = reinterpret_cast<uintptr_t>(m_data);
    }
    /**
     * @brief swap
     * @param other 入れ替え先small_buffer_vector
     * @details アロケータが異なる場合はアサートで停止します。
     */
    void swap(small_buffer_vector& other) noexcept {
        BLUESTL_ASSERT(&m_allocator == &other.m_allocator);
        if (this == &other) return;

        // 互いに相手の要素数分の容量を確保（必要ならヒープに移行）
        if (m_capacity < other.m_size) reserve(other.m_size);
        if (other.m_capacity < m_size) other.reserve(m_size);

        size_type min_size = m_size < other.m_size ? m_size : other.m_size;
        // 共通部分をswap
        for (size_type i = 0; i < min_size; ++i) {
            std::swap(m_data[i], other.m_data[i]);
        }
        // 余剰分をmove＋構築＆破棄
        if (m_size > other.m_size) {
            for (size_type i = other.m_size; i < m_size; ++i) {
                new (other.m_data + i) T(std::move(m_data[i]));
                m_data[i].~T();
            }
        } else if (other.m_size > m_size) {
            for (size_type i = m_size; i < other.m_size; ++i) {
                new (m_data + i) T(std::move(other.m_data[i]));
                other.m_data[i].~T();
            }
        }
        std::swap(m_size, other.m_size);
    }

   private:
    Allocator& m_allocator;
    T* m_data;
    size_type m_size;
    size_type m_capacity;
    alignas(T) unsigned char m_small_buffer[sizeof(T) * SmallCapacity];
    std::uintptr_t m_data_address;

    void deallocate() {
        if (m_data != reinterpret_cast<T*>(m_small_buffer)) {
            m_allocator.deallocate(m_data, sizeof(T) * m_capacity);
        }
        m_data = reinterpret_cast<T*>(m_small_buffer);
        m_capacity = SmallCapacity;
    }
};

}  // namespace bluestl
