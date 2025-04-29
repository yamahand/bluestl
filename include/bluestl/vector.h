// -----------------------------------------------------------------------------
// Bluestl vector.h
// C++20準拠・STL風インターフェースの動的配列コンテナ
// -----------------------------------------------------------------------------
/**
 * @file vector.h
 * @brief Bluestlプロジェクトのvectorクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * vectorは、可変長の動的配列を提供するコンテナです。
 * - RTTIなし、例外なし、header-only設計
 * - STL std::vectorに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
 *   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
 *
 * 主な特徴:
 *   - C++20準拠、STL std::vector風のAPI
 *   - RTTI/例外なし
 *   - header-only、#pragma onceによるインクルードガード
 *   - push_back/pop_back/insert/erase/clear/resize/reserve/shrink_to_fit/fill などの操作
 *   - イテレータ(begin/end/rbegin/rend)
 *   - 比較演算子
 *   - allocatorは外部から渡すことが可能
 *
 * Bluestl全体の設計方針:
 *   - 高速なコンパイル・実行
 *   - RTTI/例外/ヒープ割り当てなし
 *   - header-only
 *   - STLに似たインターフェース
 *   - シンプルで明確なC++コード
 *   - 分離・粒度の細かい設計
 */

#pragma once

#include "allocator.h"
#include "log_interface.h"
#include "log_macros.h"

namespace bluestl {
/**
 * @class vector
 * @brief 動的配列コンテナ。STL std::vector風インターフェース。
 * @tparam T 要素型
 * @tparam Allocator アロケータ型
 *
 * - T型の動的配列を提供
 * - RTTI/例外なし
 * - STL std::vector風インターフェース
 * - allocatorは外部から渡す
 */
template <typename T, typename Allocator = allocator>
class vector {
   public:
    // 型定義
    using value_type = T;                                                  // 要素型
    using allocator_type = Allocator;                                      // アロケータ型
    using iterator = T*;                                                   // イテレータ型
    using const_iterator = const T*;                                       // constイテレータ型
    using reverse_iterator = std::reverse_iterator<iterator>;              // 逆イテレータ型
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;  // const逆イテレータ型

    /**
     * @brief デフォルトコンストラクタは禁止（アロケータ必須）
     */
    vector() = delete;

    /**
     * @brief アロケータのみを受け取るコンストラクタ
     * @param allocator 使用するアロケータ
     */
    explicit vector(Allocator& allocator) : m_allocator(allocator), m_data(nullptr), m_size(0), m_capacity(0) {}

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元vector
     */
    vector(const vector& other) : m_allocator(other.m_allocator), m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(other.m_size);
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(other.m_data[i]);
        }
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元vector
     */
    vector(vector&& other) noexcept
        : m_allocator(std::move(other.m_allocator)),
          m_data(other.m_data),
          m_size(other.m_size),
          m_capacity(other.m_capacity) {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    /**
     * @brief コピーコンストラクタ（アロケータ指定）
     * @param other コピー元vector
     * @param allocator 使用するアロケータ
     */
    vector(const vector& other, Allocator& allocator)
        : m_allocator(allocator), m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(other.m_size);
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(other.m_data[i]);
        }
    }

    /**
     * @brief ムーブコンストラクタ（アロケータ指定）
     * @param other ムーブ元vector
     * @param allocator 使用するアロケータ
     */
    vector(vector&& other, Allocator& allocator) noexcept
        : m_allocator(allocator), m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(other.m_size);
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(std::move(other.m_data[i]));
        }
        other.clear();
    }

    /**
     * @brief コピー代入演算子
     * @param other コピー元vector
     * @return *this
     */
    vector& operator=(const vector& other) {
        if (this != &other) {
            clear();
            m_allocator = other.m_allocator;
            reserve(other.m_size);
            for (size_t i = 0; i < other.m_size; ++i) {
                push_back(other.m_data[i]);
            }
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元vector
     * @return *this
     */
    vector& operator=(vector&& other) noexcept {
        if (this != &other) {
            clear();
            m_allocator = std::move(other.m_allocator);
            reserve(other.m_size);
            for (size_t i = 0; i < other.m_size; ++i) {
                push_back(std::move(other.m_data[i]));
            }
            other.clear();
        }
        return *this;
    }

    /**
     * @brief イニシャライザリストコンストラクタ
     * @param ilist 初期化リスト
     * @param allocator 使用するアロケータ
     */
    vector(std::initializer_list<T> ilist, Allocator& allocator)
        : m_allocator(allocator), m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(ilist.size());
        for (const auto& v : ilist) {
            push_back(v);
        }
    }

    /**
     * @brief デストラクタ
     */
    ~vector() {
        clear();
        if (m_data) m_allocator.deallocate(m_data, m_capacity * sizeof(T));
    }

    /**
     * @brief 要素を末尾に追加
     * @param value 追加する値
     */
    void push_back(const T& value) {
        if (m_size >= m_capacity) reserve((m_capacity == 0) ? 4 : m_capacity * 2);
        new (&m_data[m_size]) T(value);
        ++m_size;
    }
    /**
     * @brief 末尾の要素を削除
     */
    void pop_back() {
        BLUESTL_ASSERT(m_size > 0);
        m_data[--m_size].~T();
    }

    /**
     * @brief 全要素を削除
     */
    void clear() {
        for (size_t i = 0; i < m_size; ++i) m_data[i].~T();
        m_size = 0;
    }

    /**
     * @brief 容量を拡張
     * @param new_capacity 新しい容量
     */
    void reserve(size_t new_capacity) {
        if (new_capacity <= m_capacity) return;
        T* new_data = static_cast<T*>(m_allocator.allocate(new_capacity * sizeof(T)));
        for (size_t i = 0; i < m_size; ++i) new (&new_data[i]) T(std::move(m_data[i]));
        for (size_t i = 0; i < m_size; ++i) m_data[i].~T();
        if (m_data) m_allocator.deallocate(m_data, m_capacity * sizeof(T));
        m_data = new_data;
        m_capacity = new_capacity;
    }

    /**
     * @brief 現在の要素数を取得
     * @return 要素数
     */
    size_t size() const {
        return m_size;
    }
    /**
     * @brief 現在の容量を取得
     * @return 容量
     */
    size_t capacity() const {
        return m_capacity;
    }

    /**
     * @brief 指定インデックスの要素参照
     * @param i インデックス
     * @return 要素参照
     */
    T& operator[](size_t i) {
        return m_data[i];
    }
    const T& operator[](size_t i) const {
        return m_data[i];
    }

    /**
     * @brief 先頭イテレータ取得
     */
    iterator begin() noexcept {
        return m_data;
    }
    const_iterator begin() const noexcept {
        return m_data;
    }
    /**
     * @brief 末尾イテレータ取得
     */
    iterator end() noexcept {
        return m_data + m_size;
    }
    const_iterator end() const noexcept {
        return m_data + m_size;
    }
    /**
     * @brief constイテレータの先頭・末尾
     */
    const_iterator cbegin() const noexcept {
        return begin();
    }
    const_iterator cend() const noexcept {
        return end();
    }

    /**
     * @brief 逆イテレータ取得
     */
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
     * @brief 先頭・末尾要素参照
     */
    T& front() {
        return m_data[0];
    }
    const T& front() const {
        return m_data[0];
    }
    T& back() {
        return m_data[m_size - 1];
    }
    const T& back() const {
        return m_data[m_size - 1];
    }

    /**
     * @brief 空かどうか判定
     * @return 空ならtrue
     */
    bool empty() const noexcept {
        return m_size == 0;
    }
    /**
     * @brief 範囲チェック付きアクセス
     * @param i インデックス
     * @return 要素参照
     */
    T& at(size_t i) {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }
    const T& at(size_t i) const {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }

    /**
     * @brief 要素数を変更
     * @param new_size 新しい要素数
     * @param value 追加時の初期値
     */
    void resize(size_t new_size, const T& value = T()) {
        if (new_size < m_size) {
            for (size_t i = new_size; i < m_size; ++i) {
                m_data[i].~T();
            }
        } else if (new_size > m_size) {
            reserve(new_size);
            for (size_t i = m_size; i < new_size; ++i) {
                new (&m_data[i]) T(value);
            }
        }
        m_size = new_size;
    }

    /**
     * @brief vector同士の内容を入れ替え
     * @param other 入れ替え先vector
     * @details アロケータが異なる場合はアサートで停止します。
     */
    void swap(vector& other) noexcept {
        BLUESTL_ASSERT(&m_allocator == &other.m_allocator);
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
        swap(m_capacity, other.m_capacity);
    }

    /**
     * @brief 指定数だけvalueで埋める
     * @param count 埋める数
     * @param value 埋める値
     */
    void assign(size_t count, const T& value) {
        clear();
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            new (&m_data[i]) T(value);
        }
        m_size = count;
    }
    /**
     * @brief 指定位置に要素を挿入
     * @param pos 挿入位置
     * @param value 挿入する値
     * @return 挿入位置のイテレータ
     */
    iterator insert(iterator pos, const T& value) {
        size_t idx = pos - m_data;
        BLUESTL_ASSERT(idx <= m_size);
        if (m_size >= m_capacity) reserve((m_capacity == 0) ? 4 : m_capacity * 2);
        for (size_t i = m_size; i > idx; --i) {
            new (&m_data[i]) T(std::move(m_data[i - 1]));
            m_data[i - 1].~T();
        }
        new (&m_data[idx]) T(value);
        ++m_size;
        return m_data + idx;
    }

    /**
     * @brief 指定位置にcount個のvalueを挿入
     * @param pos 挿入位置
     * @param count 挿入数
     * @param value 挿入する値
     * @return 挿入位置のイテレータ
     */
    iterator insert(iterator pos, size_t count, const T& value) {
        size_t idx = pos - m_data;
        BLUESTL_ASSERT(idx <= m_size);
        reserve(m_size + count);
        for (size_t i = m_size + count - 1; i >= idx + count; --i) {
            new (&m_data[i]) T(std::move(m_data[i - count]));
            m_data[i - count].~T();
        }
        for (size_t i = 0; i < count; ++i) {
            new (&m_data[idx + i]) T(value);
        }
        m_size += count;
        return m_data + idx;
    }

    /**
     * @brief 指定位置の要素を削除
     * @param pos 削除位置
     * @return 削除後のイテレータ
     */
    iterator erase(iterator pos) {
        size_t idx = pos - m_data;
        BLUESTL_ASSERT(idx < m_size);
        m_data[idx].~T();
        for (size_t i = idx; i < m_size - 1; ++i) {
            new (&m_data[i]) T(std::move(m_data[i + 1]));
            m_data[i + 1].~T();
        }
        --m_size;
        return m_data + idx;
    }

    /**
     * @brief 指定範囲の要素を削除
     * @param first 範囲開始
     * @param last 範囲終端
     * @return 削除後のイテレータ
     */
    iterator erase(iterator first, iterator last) {
        size_t idx_first = first - m_data;
        size_t idx_last = last - m_data;
        BLUESTL_ASSERT(idx_first <= idx_last && idx_last <= m_size);
        size_t count = idx_last - idx_first;
        for (size_t i = idx_first; i < idx_last; ++i) {
            m_data[i].~T();
        }
        for (size_t i = idx_last; i < m_size; ++i) {
            new (&m_data[i - count]) T(std::move(m_data[i]));
            m_data[i].~T();
        }
        m_size -= count;
        return m_data + idx_first;
    }

    /**
     * @brief 内部データへのポインタ取得
     * @return データポインタ
     */
    T* data() noexcept {
        return m_data;
    }
    const T* data() const noexcept {
        return m_data;
    }

    /**
     * @brief 容量を要素数に合わせて縮小
     */
    void shrink_to_fit() {
        if (m_size < m_capacity) {
            T* new_data = static_cast<T*>(m_allocator.allocate(m_size * sizeof(T)));
            for (size_t i = 0; i < m_size; ++i) {
                new (&new_data[i]) T(std::move(m_data[i]));
                m_data[i].~T();
            }
            if (m_data) m_allocator.deallocate(m_data, m_capacity * sizeof(T));
            m_data = new_data;
            m_capacity = m_size;
        }
    }

    /**
     * @brief 全要素をvalueで埋める
     * @param value 埋める値
     */
    void fill(const T& value) {
        for (size_t i = 0; i < m_size; ++i) {
            m_data[i] = value;
        }
    }

    // 比較演算子
    // 2つのvectorが等しいか判定します。
    friend bool operator==(const vector& lhs, const vector& rhs) {
        if (lhs.m_size != rhs.m_size) return false;
        for (size_t i = 0; i < lhs.m_size; ++i) {
            if (!(lhs.m_data[i] == rhs.m_data[i])) return false;
        }
        return true;
    }
    friend bool operator!=(const vector& lhs, const vector& rhs) {
        return !(lhs == rhs);
    }
    friend bool operator<(const vector& lhs, const vector& rhs) {
        size_t n = lhs.m_size < rhs.m_size ? lhs.m_size : rhs.m_size;
        for (size_t i = 0; i < n; ++i) {
            if (lhs.m_data[i] < rhs.m_data[i]) return true;
            if (rhs.m_data[i] < lhs.m_data[i]) return false;
        }
        return lhs.m_size < rhs.m_size;
    }
    friend bool operator>(const vector& lhs, const vector& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(const vector& lhs, const vector& rhs) {
        return !(rhs < lhs);
    }
    friend bool operator>=(const vector& lhs, const vector& rhs) {
        return !(lhs < rhs);
    }

   private:
    Allocator& m_allocator;  // アロケータ参照
    T* m_data;               // データ領域
    size_t m_size;           // 現在の要素数
    size_t m_capacity;       // 現在の容量
};
}  // namespace bluestl