// -----------------------------------------------------------------------------
// Bluestl string.h
// C++20準拠・STL風インターフェースの可変長文字列
// -----------------------------------------------------------------------------
/**
 * @file string.h
 * @brief Bluestlプロジェクトのstringクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * stringは、可変長文字列コンテナです。
 * - RTTIなし、例外なし、header-only設計
 * - STL std::stringに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
 *   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
 *
 * 主な特徴:
 *   - C++20準拠、STL std::string風のAPI
 *   - RTTI/例外なし
 *   - header-only、#pragma onceによるインクルードガード
 *   - append/substr/find/c_str/clear/resize などの操作
 *   - イテレータ(begin/end/rbegin/rend)
 *   - 比較演算子（==, !=, <, >, <=, >=）
 *   - 可変長でヒープ確保による動的拡張
 *   - allocatorを外部から渡すことが可能
 *
 * Bluestl全体の設計方針:
 *   - 高速なコンパイル・実行
 *   - RTTI/例外/最小限のヒープ割り当て
 *   - header-only
 *   - STLに似たインターフェース
 *   - シンプルで明確なC++コード
 *   - 分離・粒度の細かい設計
 *   - STL std::stringとの違い: 例外非対応、RTTI非使用、最小限の実装
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>
#include <type_traits>
#include <iterator>
#include <concepts>
#include <algorithm>
#include "allocator.h"
#include "allocator_traits.h"
#include "assert_handler.h"

namespace bluestl {

/**
 * @class string
 * @brief 可変長文字列コンテナ。STL std::string風インターフェース。
 * @tparam Allocator アロケータ型
 *
 * - 可変長でヒープ割り当てによる動的拡張
 * - RTTI/例外なし
 * - STL std::string風インターフェース
 * - allocatorは外部から渡すことが可能
 */
template <typename Allocator = allocator<char>>
class string {
   public:
    using value_type = char;
    using allocator_type = Allocator;
    using traits_type = allocator_traits<Allocator>;
    using reference = char&;
    using const_reference = const char&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = char*;
    using const_iterator = const char*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos = static_cast<size_type>(-1);

    /**
     * @brief デフォルトコンストラクタ
     * @param alloc アロケータ
     */
    explicit string(const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        ensure_capacity(1);  // null終端のための最小容量
        m_data[0] = '\0';
    }

    /**
     * @brief C文字列からの構築
     * @param str C文字列
     * @param alloc アロケータ
     */
    string(const char* str, const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (str) {
            size_type len = get_cstring_length(str);
            ensure_capacity(len + 1);
            for (size_type i = 0; i < len; ++i) {
                m_data[i] = str[i];
            }
            m_size = len;
            m_data[m_size] = '\0';
        } else {
            ensure_capacity(1);
            m_data[0] = '\0';
        }
    }

    /**
     * @brief string_viewからの構築
     * @param sv string_view
     * @param alloc アロケータ
     */
    string(std::string_view sv, const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        ensure_capacity(sv.size() + 1);
        for (size_type i = 0; i < sv.size(); ++i) {
            m_data[i] = sv[i];
        }
        m_size = sv.size();
        m_data[m_size] = '\0';
    } /**
       * @brief 他の文字列から部分文字列を構築
       * @tparam OtherAllocator 元の文字列のアロケータ型
       * @param other 元の文字列
       * @param pos 開始位置
       * @param len 長さ（nposの場合は文字列の終わりまで）
       * @param alloc アロケータ
       */
    template <typename OtherAllocator>
    string(const string<OtherAllocator>& other, size_type pos, size_type len = npos,
           const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (pos <= other.size()) {
            size_type actual_len = (len == npos || pos + len > other.size()) ? other.size() - pos : len;
            ensure_capacity(actual_len + 1);
            for (size_type i = 0; i < actual_len; ++i) {
                m_data[i] = other[pos + i];
            }
            m_size = actual_len;
            m_data[m_size] = '\0';
        } else {
            ensure_capacity(1);
            m_data[0] = '\0';
        }
    }

    /**
     * @brief C文字列から指定長さで構築
     * @param str C文字列
     * @param count 使用する文字数
     * @param alloc アロケータ
     */
    string(const char* str, size_type count, const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (str) {
            ensure_capacity(count + 1);
            for (size_type i = 0; i < count && str[i]; ++i) {
                m_data[i] = str[i];
                m_size = i + 1;
            }
            m_data[m_size] = '\0';
        } else {
            ensure_capacity(1);
            m_data[0] = '\0';
        }
    }

    /**
     * @brief 文字を指定回数繰り返して構築
     * @param count 文字数
     * @param ch 文字
     * @param alloc アロケータ
     */
    string(size_type count, char ch, const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        ensure_capacity(count + 1);
        for (size_type i = 0; i < count; ++i) {
            m_data[i] = ch;
        }
        m_size = count;
        m_data[m_size] = '\0';
    }

    /**
     * @brief イテレータから構築
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     * @param alloc アロケータ
     */
    template <std::input_iterator InputIt>
    string(InputIt first, InputIt last, const Allocator& alloc = Allocator()) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        ensure_capacity(1);
        m_data[0] = '\0';
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元
     */
    string(const string& other)
        : m_allocator(traits_type::select_on_container_copy_construction(other.m_allocator)),
          m_data(nullptr),
          m_size(0),
          m_capacity(0) {
        if (other.m_size > 0) {
            ensure_capacity(other.m_size + 1);
            for (size_type i = 0; i < other.m_size; ++i) {
                m_data[i] = other.m_data[i];
            }
            m_size = other.m_size;
            m_data[m_size] = '\0';
        } else {
            ensure_capacity(1);
            m_data[0] = '\0';
        }
    }

    /**
     * @brief コピーコンストラクタ（アロケータ指定）
     * @param other コピー元
     * @param alloc アロケータ
     */
    string(const string& other, const Allocator& alloc)
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (other.m_size > 0) {
            ensure_capacity(other.m_size + 1);
            for (size_type i = 0; i < other.m_size; ++i) {
                m_data[i] = other.m_data[i];
            }
            m_size = other.m_size;
            m_data[m_size] = '\0';
        } else {
            ensure_capacity(1);
            m_data[0] = '\0';
        }
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元
     */
    string(string&& other) noexcept
        : m_allocator(std::move(other.m_allocator)),
          m_data(other.m_data),
          m_size(other.m_size),
          m_capacity(other.m_capacity) {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    /**
     * @brief ムーブコンストラクタ（アロケータ指定）
     * @param other ムーブ元
     * @param alloc アロケータ
     */
    string(string&& other, const Allocator& alloc) : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (m_allocator == other.m_allocator) {
            // アロケータが同じならリソースをムーブ
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        } else {
            // アロケータが異なるなら要素をムーブ
            if (other.m_size > 0) {
                ensure_capacity(other.m_size + 1);
                for (size_type i = 0; i < other.m_size; ++i) {
                    m_data[i] = std::move(other.m_data[i]);
                }
                m_size = other.m_size;
                m_data[m_size] = '\0';
            } else {
                ensure_capacity(1);
                m_data[0] = '\0';
            }
        }
    }

    /**
     * @brief デストラクタ
     */
    ~string() {
        deallocate_memory();
    }

    /**
     * @brief コピー代入演算子
     * @param other コピー元
     * @return *this
     */
    string& operator=(const string& other) {
        if (this != &other) {
            if constexpr (traits_type::propagate_on_container_copy_assignment::value) {
                if (m_allocator != other.m_allocator) {
                    // アロケータが異なり、伝播が必要な場合、既存のメモリを解放
                    deallocate_memory();
                    m_allocator = other.m_allocator;
                    m_data = nullptr;
                    m_size = 0;
                    m_capacity = 0;
                }
            }
            assign(other);
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元
     * @return *this
     */
    string& operator=(string&& other) noexcept(std::is_nothrow_move_assignable_v<Allocator> ||
                                               !traits_type::propagate_on_container_move_assignment::value) {
        if (this == &other) {
            return *this;
        }

        if constexpr (traits_type::propagate_on_container_move_assignment::value) {
            // アロケータをムーブする場合
            deallocate_memory();
            m_allocator = std::move(other.m_allocator);
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        } else {
            // アロケータをムーブしない場合
            if (m_allocator == other.m_allocator) {
                // アロケータが同じならリソースをスチール
                deallocate_memory();
                m_data = other.m_data;
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                other.m_data = nullptr;
                other.m_size = 0;
                other.m_capacity = 0;
            } else {
                // アロケータが異なるなら要素をムーブ
                assign(std::move(other));
            }
        }
        return *this;
    }

    /**
     * @brief C文字列代入演算子
     * @param str C文字列
     * @return *this
     */
    string& operator=(const char* str) noexcept {
        assign(str);
        return *this;
    }

    /**
     * @brief string_view代入演算子
     * @param sv string_view
     * @return *this
     */
    string& operator=(std::string_view sv) noexcept {
        assign(sv);
        return *this;
    }

    /**
     * @brief 文字代入演算子
     * @param ch 文字
     * @return *this
     */
    string& operator=(char ch) noexcept {
        clear();
        push_back(ch);
        return *this;
    }

    // --- アクセス ---

    /**
     * @brief 指定インデックスの文字参照
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] reference operator[](size_type pos) noexcept {
        BLUESTL_ASSERT(pos < m_size);
        return m_data[pos];
    }

    /**
     * @brief 指定インデックスの文字参照（const版）
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] const_reference operator[](size_type pos) const noexcept {
        BLUESTL_ASSERT(pos < m_size);
        return m_data[pos];
    }

    /**
     * @brief 範囲チェック付きアクセス
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] reference at(size_type pos) noexcept {
        BLUESTL_ASSERT(pos < m_size);
        return m_data[pos];
    }

    /**
     * @brief 範囲チェック付きアクセス（const版）
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] const_reference at(size_type pos) const noexcept {
        BLUESTL_ASSERT(pos < m_size);
        return m_data[pos];
    }

    /**
     * @brief 先頭文字参照
     * @return 先頭文字参照
     */
    [[nodiscard]] reference front() noexcept {
        BLUESTL_ASSERT(m_size > 0);
        return m_data[0];
    }

    /**
     * @brief 先頭文字参照（const版）
     * @return 先頭文字参照
     */
    [[nodiscard]] const_reference front() const noexcept {
        BLUESTL_ASSERT(m_size > 0);
        return m_data[0];
    }

    /**
     * @brief 末尾文字参照
     * @return 末尾文字参照
     */
    [[nodiscard]] reference back() noexcept {
        BLUESTL_ASSERT(m_size > 0);
        return m_data[m_size - 1];
    }

    /**
     * @brief 末尾文字参照（const版）
     * @return 末尾文字参照
     */
    [[nodiscard]] const_reference back() const noexcept {
        BLUESTL_ASSERT(m_size > 0);
        return m_data[m_size - 1];
    }

    /**
     * @brief 内部データへのポインタ取得
     * @return データポインタ
     */
    [[nodiscard]] char* data() noexcept {
        return m_data;
    }

    /**
     * @brief 内部データへのポインタ取得（const版）
     * @return データポインタ
     */
    [[nodiscard]] const char* data() const noexcept {
        return m_data;
    }

    /**
     * @brief C文字列として取得
     * @return null終端文字列
     */
    [[nodiscard]] const char* c_str() const noexcept {
        return m_data ? m_data : "";
    }

    // --- イテレータ ---

    /**
     * @brief 先頭イテレータ
     * @return 先頭イテレータ
     */
    [[nodiscard]] iterator begin() noexcept {
        return m_data;
    }

    /**
     * @brief 先頭イテレータ（const版）
     * @return 先頭イテレータ
     */
    [[nodiscard]] const_iterator begin() const noexcept {
        return m_data;
    }

    /**
     * @brief 末尾イテレータ
     * @return 末尾イテレータ
     */
    [[nodiscard]] iterator end() noexcept {
        return m_data + m_size;
    }

    /**
     * @brief 末尾イテレータ（const版）
     * @return 末尾イテレータ
     */
    [[nodiscard]] const_iterator end() const noexcept {
        return m_data + m_size;
    }

    /**
     * @brief constイテレータの先頭
     * @return constイテレータの先頭
     */
    [[nodiscard]] const_iterator cbegin() const noexcept {
        return begin();
    }

    /**
     * @brief constイテレータの末尾
     * @return constイテレータの末尾
     */
    [[nodiscard]] const_iterator cend() const noexcept {
        return end();
    }

    /**
     * @brief 逆イテレータの先頭
     * @return 逆イテレータの先頭
     */
    [[nodiscard]] reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    /**
     * @brief 逆イテレータの先頭（const版）
     * @return 逆イテレータの先頭
     */
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 逆イテレータの末尾
     * @return 逆イテレータの末尾
     */
    [[nodiscard]] reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    /**
     * @brief 逆イテレータの末尾（const版）
     * @return 逆イテレータの末尾
     */
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief const逆イテレータの先頭
     * @return const逆イテレータの先頭
     */
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    /**
     * @brief const逆イテレータの末尾
     * @return const逆イテレータの末尾
     */
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return rend();
    }

    // --- 容量 ---

    /**
     * @brief 文字列の長さを取得
     * @return 文字列の長さ
     */
    [[nodiscard]] size_type size() const noexcept {
        return m_size;
    }

    /**
     * @brief 文字列の長さを取得（sizeのエイリアス）
     * @return 文字列の長さ
     */
    [[nodiscard]] size_type length() const noexcept {
        return m_size;
    }

    /**
     * @brief 現在の容量を取得
     * @return 現在の容量
     */
    [[nodiscard]] size_type capacity() const noexcept {
        return m_capacity;
    }

    /**
     * @brief 最大容量を取得
     * @return 最大容量
     */
    [[nodiscard]] size_type max_size() const noexcept {
        return traits_type::max_size(m_allocator);
    }

    /**
     * @brief 空かどうか判定
     * @return 空ならtrue
     */
    [[nodiscard]] bool empty() const noexcept {
        return m_size == 0;
    }

    /**
     * @brief 容量確保
     * @param new_cap 新しい容量
     */
    void reserve(size_type new_cap) {
        if (new_cap <= m_capacity) return;

        size_type actual_cap = calculate_growth(new_cap);
        char* new_data = traits_type::allocate(m_allocator, actual_cap);

        if (m_data) {
            for (size_type i = 0; i <= m_size; ++i) {  // null終端も含めてコピー
                new_data[i] = m_data[i];
            }
            traits_type::deallocate(m_allocator, m_data, m_capacity);
        } else {
            new_data[0] = '\0';
        }

        m_data = new_data;
        m_capacity = actual_cap;
    }

    /**
     * @brief 容量縮小
     */
    void shrink_to_fit() {
        if (m_capacity > m_size + 1) {
            size_type new_cap = m_size + 1;
            char* new_data = traits_type::allocate(m_allocator, new_cap);

            for (size_type i = 0; i <= m_size; ++i) {  // null終端も含めてコピー
                new_data[i] = m_data[i];
            }

            traits_type::deallocate(m_allocator, m_data, m_capacity);
            m_data = new_data;
            m_capacity = new_cap;
        }
    }

    // --- 変更操作 ---

    /**
     * @brief 文字列をクリア
     */
    void clear() noexcept {
        m_size = 0;
        if (m_data) {
            m_data[0] = '\0';
        }
    }

    /**
     * @brief 末尾に文字を追加
     * @param ch 追加する文字
     */
    void push_back(char ch) {
        ensure_capacity(m_size + 2);  // 新しい文字 + null終端
        m_data[m_size] = ch;
        ++m_size;
        m_data[m_size] = '\0';
    }

    /**
     * @brief 末尾の文字を削除
     */
    void pop_back() noexcept {
        if (m_size > 0) {
            --m_size;
            m_data[m_size] = '\0';
        }
    }

    /**
     * @brief 文字列を代入
     * @param str C文字列
     */
    void assign(const char* str) noexcept {
        clear();
        if (str) {
            size_type len = get_cstring_length(str);
            ensure_capacity(len + 1);
            for (size_type i = 0; i < len; ++i) {
                m_data[i] = str[i];
            }
            m_size = len;
            m_data[m_size] = '\0';
        }
    }

    /**
     * @brief 文字列を代入
     * @param sv string_view
     */
    void assign(std::string_view sv) noexcept {
        clear();
        ensure_capacity(sv.size() + 1);
        for (size_type i = 0; i < sv.size(); ++i) {
            m_data[i] = sv[i];
        }
        m_size = sv.size();
        m_data[m_size] = '\0';
    }

    /**
     * @brief 文字列を代入
     * @param other 他のstring
     */
    void assign(const string& other) noexcept {
        clear();
        ensure_capacity(other.m_size + 1);
        for (size_type i = 0; i < other.m_size; ++i) {
            m_data[i] = other.m_data[i];
        }
        m_size = other.m_size;
        m_data[m_size] = '\0';
    }

    /**
     * @brief 文字列を代入（ムーブ）
     * @param other 他のstring
     */
    void assign(string&& other) noexcept {
        if (this != &other) {
            if (m_allocator == other.m_allocator) {
                // アロケータが同じならリソースをスチール
                deallocate_memory();
                m_data = other.m_data;
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                other.m_data = nullptr;
                other.m_size = 0;
                other.m_capacity = 0;
            } else {
                // アロケータが異なるなら要素をムーブ
                assign(std::string_view(other));
            }
        }
    }

    /**
     * @brief 文字列を指定長さで代入
     * @param str C文字列
     * @param count 使用する文字数
     */
    void assign(const char* str, size_type count) noexcept {
        clear();
        if (str) {
            ensure_capacity(count + 1);
            for (size_type i = 0; i < count && str[i]; ++i) {
                m_data[i] = str[i];
                m_size = i + 1;
            }
            m_data[m_size] = '\0';
        }
    }

    /**
     * @brief 文字を指定回数で代入
     * @param count 文字数
     * @param ch 文字
     */
    void assign(size_type count, char ch) noexcept {
        clear();
        ensure_capacity(count + 1);
        for (size_type i = 0; i < count; ++i) {
            m_data[i] = ch;
        }
        m_size = count;
        m_data[m_size] = '\0';
    }

    /**
     * @brief イテレータから代入
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     */
    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last) noexcept {
        clear();
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    /**
     * @brief 文字列を末尾に追加
     * @param str C文字列
     * @return *this
     */
    string& append(const char* str) {
        if (str) {
            size_type str_len = get_cstring_length(str);
            ensure_capacity(m_size + str_len + 1);
            for (size_type i = 0; i < str_len; ++i) {
                m_data[m_size + i] = str[i];
            }
            m_size += str_len;
            m_data[m_size] = '\0';
        }
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加
     * @param sv string_view
     * @return *this
     */
    string& append(std::string_view sv) {
        ensure_capacity(m_size + sv.size() + 1);
        for (size_type i = 0; i < sv.size(); ++i) {
            m_data[m_size + i] = sv[i];
        }
        m_size += sv.size();
        m_data[m_size] = '\0';
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加
     * @param other 他のstring
     * @return *this
     */
    string& append(const string& other) {
        ensure_capacity(m_size + other.m_size + 1);
        for (size_type i = 0; i < other.m_size; ++i) {
            m_data[m_size + i] = other.m_data[i];
        }
        m_size += other.m_size;
        m_data[m_size] = '\0';
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加
     * @param str C文字列
     * @param count 使用する文字数
     * @return *this
     */
    string& append(const char* str, size_type count) {
        if (str) {
            ensure_capacity(m_size + count + 1);
            for (size_type i = 0; i < count && str[i]; ++i) {
                m_data[m_size + i] = str[i];
                ++m_size;
            }
            m_data[m_size] = '\0';
        }
        return *this;
    }

    /**
     * @brief 文字を指定回数末尾に追加
     * @param count 文字数
     * @param ch 文字
     * @return *this
     */
    string& append(size_type count, char ch) {
        ensure_capacity(m_size + count + 1);
        for (size_type i = 0; i < count; ++i) {
            m_data[m_size + i] = ch;
        }
        m_size += count;
        m_data[m_size] = '\0';
        return *this;
    }

    /**
     * @brief イテレータから末尾に追加
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     * @return *this
     */
    template <std::input_iterator InputIt>
    string& append(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param str C文字列
     * @return *this
     */
    string& operator+=(const char* str) {
        return append(str);
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param sv string_view
     * @return *this
     */
    string& operator+=(std::string_view sv) {
        return append(sv);
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param other 他のstring
     * @return *this
     */
    string& operator+=(const string& other) {
        return append(other);
    }

    /**
     * @brief 文字を末尾に追加（+=演算子）
     * @param ch 文字
     * @return *this
     */
    string& operator+=(char ch) {
        push_back(ch);
        return *this;
    }

    /**
     * @brief サイズを変更
     * @param count 新しいサイズ
     * @param ch 埋める文字（デフォルトは'\0'）
     */
    void resize(size_type count, char ch = '\0') {
        if (count < m_size) {
            m_size = count;
            m_data[m_size] = '\0';
        } else if (count > m_size) {
            ensure_capacity(count + 1);
            for (size_type i = m_size; i < count; ++i) {
                m_data[i] = ch;
            }
            m_size = count;
            m_data[m_size] = '\0';
        }
    }

    /**
     * @brief string_viewに変換
     * @return string_view
     */
    [[nodiscard]] operator std::string_view() const noexcept {
        return std::string_view(m_data, m_size);
    }

    // --- 検索操作 ---

    /**
     * @brief 部分文字列を取得
     * @param pos 開始位置
     * @param len 長さ（デフォルトは末尾まで）
     * @return 部分文字列
     */
    [[nodiscard]] string substr(size_type pos = 0, size_type len = npos) const {
        if (pos >= m_size) return string(m_allocator);

        size_type actual_len = (len == npos || pos + len > m_size) ? m_size - pos : len;
        return string(m_data + pos, actual_len, m_allocator);
    }

    /**
     * @brief 文字列検索
     * @param str 検索する文字列
     * @param pos 開始位置
     * @return 見つかった位置、見つからない場合はnpos
     */
    [[nodiscard]] size_type find(const char* str, size_type pos = 0) const noexcept {
        if (!str || pos >= m_size) return npos;

        size_type str_len = get_cstring_length(str);
        if (str_len == 0) return pos;
        if (str_len > m_size - pos) return npos;

        for (size_type i = pos; i <= m_size - str_len; ++i) {
            bool match = true;
            for (size_type j = 0; j < str_len; ++j) {
                if (m_data[i + j] != str[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return npos;
    }

    /**
     * @brief 文字列検索
     * @param sv 検索するstring_view
     * @param pos 開始位置
     * @return 見つかった位置、見つからない場合はnpos
     */
    [[nodiscard]] size_type find(std::string_view sv, size_type pos = 0) const noexcept {
        if (pos >= m_size) return npos;
        if (sv.size() == 0) return pos;
        if (sv.size() > m_size - pos) return npos;

        for (size_type i = pos; i <= m_size - sv.size(); ++i) {
            bool match = true;
            for (size_type j = 0; j < sv.size(); ++j) {
                if (m_data[i + j] != sv[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return npos;
    }

    /**
     * @brief 文字列検索
     * @param other 検索するstring
     * @param pos 開始位置
     * @return 見つかった位置、見つからない場合はnpos
     */
    [[nodiscard]] size_type find(const string& other, size_type pos = 0) const noexcept {
        return find(std::string_view(other), pos);
    }

    /**
     * @brief 文字検索
     * @param ch 検索する文字
     * @param pos 開始位置
     * @return 見つかった位置、見つからない場合はnpos
     */
    [[nodiscard]] size_type find(char ch, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < m_size; ++i) {
            if (m_data[i] == ch) return i;
        }
        return npos;
    }

    /**
     * @brief 指定した文字列で始まるかチェック
     * @param str 検索する文字列
     * @return 始まる場合はtrue
     */
    [[nodiscard]] bool starts_with(const char* str) const noexcept {
        return find(str) == 0;
    }

    /**
     * @brief 指定した文字で始まるかチェック
     * @param ch 検索する文字
     * @return 始まる場合はtrue
     */
    [[nodiscard]] bool starts_with(char ch) const noexcept {
        return !empty() && front() == ch;
    }

    /**
     * @brief 指定したstring_viewで始まるかチェック
     * @param sv 検索するstring_view
     * @return 始まる場合はtrue
     */
    [[nodiscard]] bool starts_with(std::string_view sv) const noexcept {
        return find(sv) == 0;
    }

    /**
     * @brief 指定した文字列で終わるかチェック
     * @param str 検索する文字列
     * @return 終わる場合はtrue
     */
    [[nodiscard]] bool ends_with(const char* str) const noexcept {
        if (!str) return true;
        size_type str_len = get_cstring_length(str);
        if (str_len > m_size) return false;
        if (str_len == 0) return true;

        for (size_type i = 0; i < str_len; ++i) {
            if (m_data[m_size - str_len + i] != str[i]) return false;
        }
        return true;
    }

    /**
     * @brief 指定した文字で終わるかチェック
     * @param ch 検索する文字
     * @return 終わる場合はtrue
     */
    [[nodiscard]] bool ends_with(char ch) const noexcept {
        return !empty() && back() == ch;
    }

    /**
     * @brief 指定したstring_viewで終わるかチェック
     * @param sv 検索するstring_view
     * @return 終わる場合はtrue
     */
    [[nodiscard]] bool ends_with(std::string_view sv) const noexcept {
        if (sv.size() > m_size) return false;
        if (sv.size() == 0) return true;

        for (size_type i = 0; i < sv.size(); ++i) {
            if (m_data[m_size - sv.size() + i] != sv[i]) return false;
        }
        return true;
    }

    /**
     * @brief 指定した文字列が含まれるかチェック
     * @param str 検索する文字列
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] bool contains(const char* str) const noexcept {
        return find(str) != npos;
    }

    /**
     * @brief 指定した文字が含まれるかチェック
     * @param ch 検索する文字
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] bool contains(char ch) const noexcept {
        return find(ch) != npos;
    }

    /**
     * @brief 指定したstring_viewが含まれるかチェック
     * @param sv 検索するstring_view
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] bool contains(std::string_view sv) const noexcept {
        return find(sv) != npos;
    }

    // --- アロケータ ---

    /**
     * @brief アロケータのコピーを取得
     * @return アロケータのコピー
     */
    allocator_type get_allocator() const noexcept {
        return m_allocator;
    }

    // --- 比較演算子（friend関数として定義） ---

    friend bool operator==(const string& lhs, const string& rhs) noexcept {
        if (lhs.m_size != rhs.m_size) return false;
        for (size_type i = 0; i < lhs.m_size; ++i) {
            if (lhs.m_data[i] != rhs.m_data[i]) return false;
        }
        return true;
    }

    friend bool operator!=(const string& lhs, const string& rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator<(const string& lhs, const string& rhs) noexcept {
        size_type min_size = (lhs.m_size < rhs.m_size) ? lhs.m_size : rhs.m_size;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.m_data[i] < rhs.m_data[i]) return true;
            if (lhs.m_data[i] > rhs.m_data[i]) return false;
        }
        return lhs.m_size < rhs.m_size;
    }

    friend bool operator>(const string& lhs, const string& rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(const string& lhs, const string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(const string& lhs, const string& rhs) noexcept {
        return !(lhs < rhs);
    }

    // C文字列との比較
    friend bool operator==(const string& lhs, const char* rhs) noexcept {
        if (!rhs) return lhs.empty();
        size_type rhs_len = get_cstring_length(rhs);
        if (lhs.m_size != rhs_len) return false;
        for (size_type i = 0; i < lhs.m_size; ++i) {
            if (lhs.m_data[i] != rhs[i]) return false;
        }
        return true;
    }

    friend bool operator==(const char* lhs, const string& rhs) noexcept {
        return rhs == lhs;
    }

    friend bool operator!=(const string& lhs, const char* rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator!=(const char* lhs, const string& rhs) noexcept {
        return !(rhs == lhs);
    }

    friend bool operator<(const string& lhs, const char* rhs) noexcept {
        if (!rhs) return false;
        size_type rhs_len = get_cstring_length(rhs);
        size_type min_size = (lhs.m_size < rhs_len) ? lhs.m_size : rhs_len;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.m_data[i] < rhs[i]) return true;
            if (lhs.m_data[i] > rhs[i]) return false;
        }
        return lhs.m_size < rhs_len;
    }

    friend bool operator<(const char* lhs, const string& rhs) noexcept {
        if (!lhs) return !rhs.empty();
        size_type lhs_len = get_cstring_length(lhs);
        size_type min_size = (lhs_len < rhs.m_size) ? lhs_len : rhs.m_size;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs[i] < rhs.m_data[i]) return true;
            if (lhs[i] > rhs.m_data[i]) return false;
        }
        return lhs_len < rhs.m_size;
    }

    friend bool operator>(const string& lhs, const char* rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator>(const char* lhs, const string& rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(const string& lhs, const char* rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator<=(const char* lhs, const string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(const string& lhs, const char* rhs) noexcept {
        return !(lhs < rhs);
    }

    friend bool operator>=(const char* lhs, const string& rhs) noexcept {
        return !(lhs < rhs);
    }

    // string_viewとの比較
    friend bool operator==(const string& lhs, std::string_view rhs) noexcept {
        if (lhs.m_size != rhs.size()) return false;
        for (size_type i = 0; i < lhs.m_size; ++i) {
            if (lhs.m_data[i] != rhs[i]) return false;
        }
        return true;
    }

    friend bool operator==(std::string_view lhs, const string& rhs) noexcept {
        return rhs == lhs;
    }

    friend bool operator!=(const string& lhs, std::string_view rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator!=(std::string_view lhs, const string& rhs) noexcept {
        return !(rhs == lhs);
    }

    friend bool operator<(const string& lhs, std::string_view rhs) noexcept {
        size_type min_size = (lhs.m_size < rhs.size()) ? lhs.m_size : rhs.size();
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.m_data[i] < rhs[i]) return true;
            if (lhs.m_data[i] > rhs[i]) return false;
        }
        return lhs.m_size < rhs.size();
    }

    friend bool operator<(std::string_view lhs, const string& rhs) noexcept {
        size_type min_size = (lhs.size() < rhs.m_size) ? lhs.size() : rhs.m_size;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs[i] < rhs.m_data[i]) return true;
            if (lhs[i] > rhs.m_data[i]) return false;
        }
        return lhs.size() < rhs.m_size;
    }

    friend bool operator>(const string& lhs, std::string_view rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator>(std::string_view lhs, const string& rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(const string& lhs, std::string_view rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator<=(std::string_view lhs, const string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(const string& lhs, std::string_view rhs) noexcept {
        return !(lhs < rhs);
    }

    friend bool operator>=(std::string_view lhs, const string& rhs) noexcept {
        return !(lhs < rhs);
    }

   private:
    Allocator m_allocator;
    char* m_data;
    size_type m_size;
    size_type m_capacity;

    /**
     * @brief C文字列の長さを取得
     * @param str C文字列
     * @return 文字列の長さ
     */
    static size_type get_cstring_length(const char* str) noexcept {
        if (!str) return 0;
        size_type len = 0;
        while (str[len]) ++len;
        return len;
    }

    /**
     * @brief 容量を確保（必要に応じて拡張）
     * @param required_cap 必要な容量
     */
    void ensure_capacity(size_type required_cap) {
        if (required_cap > m_capacity) {
            reserve(required_cap);
        }
    }

    /**
     * @brief 成長サイズを計算
     * @param min_cap 最小必要容量
     * @return 新しい容量
     */
    size_type calculate_growth(size_type min_cap) const noexcept {
        size_type growth = m_capacity + m_capacity / 2;  // 1.5倍成長
        return (growth > min_cap) ? growth : min_cap;
    }

    /**
     * @brief メモリを解放
     */
    void deallocate_memory() noexcept {
        if (m_data) {
            traits_type::deallocate(m_allocator, m_data, m_capacity);
            m_data = nullptr;
            m_capacity = 0;
        }
    }
};

// 型エイリアス（デフォルトアロケータ使用）
using basic_string = string<allocator<char>>;

}  // namespace bluestl
