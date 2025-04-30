// ... (ファイル冒頭のコメントは変更なし) ...

#pragma once

#include "allocator.h"
#include "allocator_traits.h" // allocator_traits をインクルード
#include "log_interface.h"
#include "log_macros.h"
#include <concepts>      // std::input_iterator のために追加
#include <iterator>      // std::reverse_iterator, std::iterator_traits, std::distance, std::make_move_iterator
#include <memory>        // std::addressof
#include <type_traits> // std::is_nothrow_destructible, std::is_nothrow_move_constructible, etc.
#include <utility>       // std::move, std::swap, std::forward
#include <initializer_list> // std::initializer_list
#include <algorithm>     // std::min, std::max (calculate_new_capacityで使用)
#include <compare>       // std::compare_three_way_result_t (operator<=>で使用)


namespace bluestl {
/**
 * @class vector
 * @brief 動的配列コンテナ。STL std::vector風インターフェース。
 * @tparam T 要素型
 * @tparam Allocator アロケータ型 (デフォルトは bluestl::allocator<T>)
 *
 * - T型の動的配列を提供
 * - RTTI/例外なし
 * - STL std::vector風インターフェース
 * - allocatorは値で保持し、allocator_traits経由で操作
 */
template <typename T, typename Allocator = allocator<T>> // デフォルトアロケータを型付きに修正
class vector {
   public:
    // --- 型定義 ---
    using value_type = T;
    using allocator_type = Allocator;
    using traits_type = allocator_traits<Allocator>; // allocator_traits のエイリアス
    using size_type = typename traits_type::size_type;
    using difference_type = typename traits_type::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename traits_type::pointer;
    using const_pointer = typename traits_type::const_pointer;
    using iterator = pointer;                                              // シンプルなポインタイテレータ
    using const_iterator = const_pointer;                                  // シンプルなポインタイテレータ
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // --- コンストラクタ ---

    /**
     * @brief デフォルトコンストラクタ（アロケータがデフォルト構築可能なら）
     * @details アロケータをデフォルト構築します。
     */
    vector() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
        requires std::is_default_constructible_v<Allocator>
        : m_allocator(), m_data(nullptr), m_size(0), m_capacity(0) {}

    /**
     * @brief アロケータのみを受け取るコンストラクタ
     * @param alloc 使用するアロケータ (コピーまたはムーブされる)
     */
    explicit vector(const Allocator& alloc) noexcept
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {}

    explicit vector(Allocator&& alloc) noexcept
        : m_allocator(std::move(alloc)), m_data(nullptr), m_size(0), m_capacity(0) {}


    /**
     * @brief 指定された数の要素を持つコンストラクタ (デフォルト値で初期化)
     * @param count 要素数
     * @param alloc アロケータ
     */
    explicit vector(size_type count, const Allocator& alloc = Allocator())
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (count > 0) {
            allocate_and_construct(count, T());
        }
    }

    /**
     * @brief 指定された数の要素を持つコンストラクタ (指定値で初期化)
     * @param count 要素数
     * @param value 初期値
     * @param alloc アロケータ
     */
    vector(size_type count, const T& value, const Allocator& alloc = Allocator())
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
         if (count > 0) {
            allocate_and_construct(count, value);
        }
    }

    /**
     * @brief イテレータ範囲から構築するコンストラクタ
     * @tparam InputIt イテレータ型 (入力イテレータ要件を満たす)
     * @param first 範囲の開始
     * @param last 範囲の終端
     * @param alloc アロケータ
     */
    template <std::input_iterator InputIt>
    vector(InputIt first, InputIt last, const Allocator& alloc = Allocator())
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        // TODO: イテレータのカテゴリに応じて効率的な実装を選択 (std::distanceが使えるか等)
        for (; first != last; ++first) {
            push_back(*first); // 最も単純な実装
        }
    }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元vector
     * @details アロケータは select_on_container_copy_construction で選択されます。
     */
    vector(const vector& other)
        : m_allocator(traits_type::select_on_container_copy_construction(other.m_allocator)),
          m_data(nullptr), m_size(0), m_capacity(0) {
        if (other.m_size > 0) {
            reserve(other.m_size); // selectされたアロケータで確保
            // 例外安全性を考慮しない (Bluestlの方針)
            for (size_type i = 0; i < other.m_size; ++i) {
                traits_type::construct(m_allocator, m_data + i, other.m_data[i]);
            }
            m_size = other.m_size;
        }
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元vector
     * @details アロケータはムーブされます。リソースの所有権が移動します。
     */
    vector(vector&& other) noexcept
        : m_allocator(std::move(other.m_allocator)), // アロケータをムーブ
          m_data(other.m_data),
          m_size(other.m_size),
          m_capacity(other.m_capacity) {
        // ムーブ元のリソースを無効化
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    /**
     * @brief コピーコンストラクタ（アロケータ指定）
     * @param other コピー元vector
     * @param alloc 使用するアロケータ (コピーまたはムーブされる)
     */
    vector(const vector& other, const Allocator& alloc)
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
         if (other.m_size > 0) {
            reserve(other.m_size);
            for (size_type i = 0; i < other.m_size; ++i) {
                traits_type::construct(m_allocator, m_data + i, other.m_data[i]);
            }
            m_size = other.m_size;
        }
    }
     vector(const vector& other, Allocator&& alloc)
        : m_allocator(std::move(alloc)), m_data(nullptr), m_size(0), m_capacity(0) {
         if (other.m_size > 0) {
            reserve(other.m_size);
            for (size_type i = 0; i < other.m_size; ++i) {
                traits_type::construct(m_allocator, m_data + i, other.m_data[i]);
            }
            m_size = other.m_size;
        }
    }


    /**
     * @brief ムーブコンストラクタ（アロケータ指定）
     * @param other ムーブ元vector
     * @param alloc 使用するアロケータ (コピーまたはムーブされる)
     * @details アロケータが異なる場合、要素ごとのムーブが発生します。
     */
    vector(vector&& other, const Allocator& alloc)
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        if (m_allocator == other.m_allocator) {
            // アロケータが同じならリソースをムーブ (効率的)
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        } else {
            // アロケータが異なるなら要素をムーブ
            if (other.m_size > 0) {
                reserve(other.m_size);
                for (size_type i = 0; i < other.m_size; ++i) {
                    // std::move_if_noexcept を使うのがより安全だが、Bluestlでは例外を考慮しない
                    traits_type::construct(m_allocator, m_data + i, std::move(other.m_data[i]));
                }
                m_size = other.m_size;
                // ムーブ元の要素は破棄されるべきだが、clearはアロケータが異なる場合問題を起こす可能性
                // other.~vector(); // ムーブ元は破棄される運命
            }
        }
    }
     vector(vector&& other, Allocator&& alloc) noexcept(std::is_nothrow_move_constructible_v<Allocator>)
        : m_allocator(std::move(alloc)), m_data(nullptr), m_size(0), m_capacity(0) {
         // アロケータをムーブしたので、otherのアロケータとは比較できない
         // 要素ごとのムーブを行うのが安全
         if (other.m_size > 0) {
            reserve(other.m_size);
            for (size_type i = 0; i < other.m_size; ++i) {
                traits_type::construct(m_allocator, m_data + i, std::move(other.m_data[i]));
            }
            m_size = other.m_size;
         }
         // other.clear(); // アロケータがムーブされているので呼べない
         other.m_data = nullptr; // 手動でリセット
         other.m_size = 0;
         other.m_capacity = 0;
    }


    /**
     * @brief イニシャライザリストコンストラクタ
     * @param ilist 初期化リスト
     * @param alloc アロケータ
     */
    vector(std::initializer_list<T> ilist, const Allocator& alloc = Allocator())
        : m_allocator(alloc), m_data(nullptr), m_size(0), m_capacity(0) {
        reserve(ilist.size());
        pointer current = m_data;
        for (const auto& v : ilist) {
            traits_type::construct(m_allocator, current++, v);
        }
        m_size = ilist.size();
    }

    /**
     * @brief デストラクタ
     */
    ~vector() {
        destroy_elements();
        deallocate_memory();
    }

    // --- 代入演算子 ---

    /**
     * @brief コピー代入演算子
     * @param other コピー元vector
     * @return *this
     */
    vector& operator=(const vector& other) {
        if (this == &other) {
            return *this;
        }

        if constexpr (traits_type::propagate_on_container_copy_assignment::value) {
            if (m_allocator != other.m_allocator) {
                // アロケータが異なり、伝播が必要な場合、既存のメモリを解放
                clear(); // 古いアロケータで要素破棄
                deallocate_memory(); // 古いアロケータでメモリ解放
                m_capacity = 0; // 容量リセット
                // m_data は nullptr になっているはず
            }
            m_allocator = other.m_allocator; // アロケータをコピー
        }

        // assign を使うと効率的
        assign(other.begin(), other.end());

        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元vector
     * @return *this
     */
    vector& operator=(vector&& other) noexcept(
        std::is_nothrow_move_assignable_v<Allocator> ||
        !traits_type::propagate_on_container_move_assignment::value
    ) {
        if (this == &other) {
            return *this;
        }

        if constexpr (traits_type::propagate_on_container_move_assignment::value) {
            // アロケータをムーブする場合
            destroy_elements();    // 既存の要素を破棄
            deallocate_memory(); // 既存のメモリを解放
            m_allocator = std::move(other.m_allocator); // アロケータをムーブ
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            // ムーブ元をリセット
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        } else {
            // アロケータをムーブしない場合
            if (m_allocator == other.m_allocator) {
                // アロケータが同じならリソースをスチール
                destroy_elements();
                deallocate_memory();
                m_data = other.m_data;
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                other.m_data = nullptr;
                other.m_size = 0;
                other.m_capacity = 0;
            } else {
                // アロケータが異なるなら要素ごとにムーブ
                // assign を使うと効率的
                assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
                // ムーブ元のクリアは assign 内で行われるべきだが、念のため
                other.clear();
            }
        }
        return *this;
    }

    /**
     * @brief イニシャライザリスト代入演算子
     * @param ilist 初期化リスト
     * @return *this
     */
    vector& operator=(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    // --- 要素アクセス ---

    /**
     * @brief 指定インデックスの要素参照 (境界チェックなし)
     * @param i インデックス
     * @return 要素参照
     */
    reference operator[](size_type i) noexcept {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }
    const_reference operator[](size_type i) const noexcept {
        BLUESTL_ASSERT(i < m_size);
        return m_data[i];
    }

    /**
     * @brief 指定インデックスの要素参照 (境界チェックあり)
     * @param i インデックス
     * @return 要素参照
     */
    reference at(size_type i) {
        BLUESTL_ASSERT(i < m_size); // Bluestlではアサートで境界チェック
        return m_data[i];
    }
    const_reference at(size_type i) const {
        BLUESTL_ASSERT(i < m_size); // Bluestlではアサートで境界チェック
        return m_data[i];
    }

    /**
     * @brief 先頭要素への参照
     * @return 先頭要素への参照
     */
    reference front() noexcept {
        BLUESTL_ASSERT(!empty());
        return m_data[0];
    }
    const_reference front() const noexcept {
        BLUESTL_ASSERT(!empty());
        return m_data[0];
    }

    /**
     * @brief 末尾要素への参照
     * @return 末尾要素への参照
     */
    reference back() noexcept {
        BLUESTL_ASSERT(!empty());
        return m_data[m_size - 1];
    }
    const_reference back() const noexcept {
        BLUESTL_ASSERT(!empty());
        return m_data[m_size - 1];
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

    // --- イテレータ ---

    /** @brief 先頭イテレータ取得 */
    iterator begin() noexcept { return m_data; }
    const_iterator begin() const noexcept { return m_data; }
    const_iterator cbegin() const noexcept { return begin(); }

    /** @brief 末尾イテレータ取得 */
    iterator end() noexcept { return m_data + m_size; }
    const_iterator end() const noexcept { return m_data + m_size; }
    const_iterator cend() const noexcept { return end(); }

    /** @brief 逆イテレータ取得 */
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    /** @brief 逆末尾イテレータ取得 */
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    // --- 容量 ---

    /**
     * @brief 空かどうか判定
     * @return 空ならtrue
     */
    [[nodiscard]] bool empty() const noexcept {
        return m_size == 0;
    }

    /**
     * @brief 現在の要素数を取得
     * @return 要素数
     */
    size_type size() const noexcept {
        return m_size;
    }

    /**
     * @brief アロケータが確保できる最大の要素数を返す
     * @return 最大要素数
     */
    size_type max_size() const noexcept {
        return traits_type::max_size(m_allocator);
    }

    /**
     * @brief 容量を予約する
     * @param new_capacity 予約する最小容量
     * @details 現在の容量が new_capacity より小さい場合、メモリの再確保が発生する可能性があります。
     */
    void reserve(size_type new_capacity) {
        if (new_capacity <= m_capacity) {
            return;
        }
        // max_size チェック (Bluestlではアサート)
        BLUESTL_ASSERT(new_capacity <= max_size());

        pointer new_data = traits_type::allocate(m_allocator, new_capacity);
        pointer old_data = m_data;
        size_type old_size = m_size;
        size_type old_capacity = m_capacity;

        // 要素をムーブ (例外安全性を考慮しない)
        if (old_data) {
            for (size_type i = 0; i < old_size; ++i) {
                traits_type::construct(m_allocator, new_data + i, std::move_if_noexcept(old_data[i]));
                traits_type::destroy(m_allocator, old_data + i);
            }
            traits_type::deallocate(m_allocator, old_data, old_capacity);
        }

        m_data = new_data;
        m_capacity = new_capacity;
        // m_size は変更しない
    }

    /**
     * @brief 現在の容量を取得
     * @return 容量
     */
    size_type capacity() const noexcept {
        return m_capacity;
    }

    /**
     * @brief 容量を要素数に合わせて縮小
     */
    void shrink_to_fit() {
        if (m_capacity <= m_size) {
            return; // 縮小不要、または既にぴったり
        }

        if (m_size == 0) {
            // サイズが0ならメモリを解放
            deallocate_memory();
            m_data = nullptr;
            m_capacity = 0;
            return;
        }

        // 新しい領域を確保して要素をムーブ
        pointer new_data = traits_type::allocate(m_allocator, m_size);
        pointer old_data = m_data;
        size_type old_capacity = m_capacity;

        // 要素をムーブ (例外安全性を考慮しない)
        for (size_type i = 0; i < m_size; ++i) {
            traits_type::construct(m_allocator, new_data + i, std::move_if_noexcept(old_data[i]));
            traits_type::destroy(m_allocator, old_data + i);
        }
        traits_type::deallocate(m_allocator, old_data, old_capacity);

        m_data = new_data;
        m_capacity = m_size;
    }

    // --- 変更操作 ---

    /**
     * @brief 全要素を削除
     * @details 要素のデストラクタが呼び出されますが、メモリは解放されません。
     */
    void clear() noexcept {
        destroy_elements();
        m_size = 0;
    }

    /**
     * @brief 指定位置に要素を挿入 (コピー)
     * @param pos 挿入位置を示すイテレータ
     * @param value 挿入する値
     * @return 挿入された要素を指すイテレータ
     */
    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value); // emplaceに委譲
    }

    /**
     * @brief 指定位置に要素を挿入 (ムーブ)
     * @param pos 挿入位置を示すイテレータ
     * @param value 挿入する値 (ムーブされる)
     * @return 挿入された要素を指すイテレータ
     */
    iterator insert(const_iterator pos, T&& value) {
         return emplace(pos, std::move(value)); // emplaceに委譲
    }

    /**
     * @brief 指定位置に要素を直接構築 (emplace)
     * @tparam Args 構築に使用する引数の型
     * @param pos 挿入位置を示すイテレータ
     * @param args 構築に使用する引数
     * @return 挿入された要素を指すイテレータ
     */
    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        const size_type index = pos - cbegin();
        BLUESTL_ASSERT(index <= m_size);

        if (m_size == m_capacity) {
            // 容量が足りない場合、再確保
            size_type new_cap = calculate_new_capacity(m_size + 1);
            reserve(new_cap);
            // reserveによりイテレータが無効になるため、ポインタを再計算
            pointer insert_ptr = m_data + index;
            // 後ろの要素をムーブ
            if (index < m_size) {
                pointer old_end = m_data + m_size;
                pointer new_end = old_end + 1;
                // 末尾要素を新しい位置にムーブ構築
                traits_type::construct(m_allocator, new_end -1 , std::move_if_noexcept(*(old_end - 1)));
                // 残りを後ろにムーブ代入
                for (pointer p = old_end - 1; p > insert_ptr; --p) {
                    *p = std::move_if_noexcept(*(p - 1));
                }
                // 挿入位置の古い要素を破棄 (ムーブ代入されたので不要)
                // traits_type::destroy(m_allocator, insert_ptr); // ムーブ代入なら不要
            }
             // 新しい要素を構築
            traits_type::construct(m_allocator, insert_ptr, std::forward<Args>(args)...);

        } else {
            // 容量が足りる場合
            pointer insert_ptr = m_data + index;
            if (index == m_size) {
                // 末尾への挿入
                traits_type::construct(m_allocator, insert_ptr, std::forward<Args>(args)...);
            } else {
                // 途中への挿入
                // 末尾要素を新しい位置にムーブ構築
                traits_type::construct(m_allocator, m_data + m_size, std::move_if_noexcept(m_data[m_size - 1]));
                // 挿入位置以降の要素を後ろにムーブ
                for (pointer p = m_data + m_size - 1; p > insert_ptr; --p) {
                    *p = std::move_if_noexcept(*(p - 1));
                }
                // 挿入位置に新しい要素をムーブ代入または構築
                // *insert_ptr = T(std::forward<Args>(args)...); // 代入だと既存要素の破棄が必要
                traits_type::destroy(m_allocator, insert_ptr); // 古い要素を破棄
                traits_type::construct(m_allocator, insert_ptr, std::forward<Args>(args)...); // 新しく構築
            }
        }

        ++m_size;
        return m_data + index;
    }


    /**
     * @brief 指定位置に指定個数の要素を挿入
     * @param pos 挿入位置を示すイテレータ
     * @param count 挿入する要素数
     * @param value 挿入する値
     * @return 最初に挿入された要素を指すイテレータ
     */
    iterator insert(const_iterator pos, size_type count, const T& value) {
        const size_type index = pos - cbegin();
        BLUESTL_ASSERT(index <= m_size);
        if (count == 0) return m_data + index;

        if (m_size + count > m_capacity) {
            size_type new_cap = calculate_new_capacity(m_size + count);
            reserve(new_cap);
            // reserveによりイテレータが無効になるため、ポインタを再計算
            pointer insert_ptr = m_data + index;
            pointer old_end = m_data + m_size; // reserve前のサイズ
            pointer new_end = m_data + m_size + count; // reserve後のサイズ

            // 後ろの要素をムーブ
            if (index < m_size) {
                 pointer move_src_start = m_data + index;
                 pointer move_dst_start = m_data + index + count;
                 // 後ろからムーブ構築
                 for(size_type i = 0; i < (m_size - index); ++i) {
                     traits_type::construct(m_allocator, new_end - 1 - i, std::move_if_noexcept(*(old_end - 1 - i)));
                 }
                 // ムーブ元の要素を破棄
                 for(size_type i = 0; i < (m_size - index); ++i) {
                     traits_type::destroy(m_allocator, move_src_start + i);
                 }
            }
            // 新しい要素を構築
            for (size_type i = 0; i < count; ++i) {
                traits_type::construct(m_allocator, insert_ptr + i, value);
            }

        } else {
             // 容量が足りる場合
            pointer insert_ptr = m_data + index;
            const size_type elems_after = m_size - index;

            if (elems_after > 0) {
                // 後ろに移動する要素がある
                pointer old_end = m_data + m_size;
                if (elems_after > count) {
                    // 移動元と移動先が重ならない部分をムーブ構築
                    for(size_type i = 0; i < count; ++i) {
                        traits_type::construct(m_allocator, old_end + i, std::move_if_noexcept(*(old_end - count + i)));
                    }
                    // 重なる部分をムーブ代入 (後ろから)
                    for(size_type i = 0; i < elems_after - count; ++i) {
                         *(old_end - 1 - i) = std::move_if_noexcept(*(old_end - 1 - i - count));
                    }
                } else {
                    // 全てムーブ構築 (後ろから)
                     for(size_type i = 0; i < elems_after; ++i) {
                        traits_type::construct(m_allocator, old_end + count - 1 - i, std::move_if_noexcept(*(old_end - 1 - i)));
                    }
                }
                 // 挿入位置に値を代入または構築
                 for(size_type i = 0; i < count; ++i) {
                     if (i < elems_after) {
                         *(insert_ptr + i) = value; // 代入
                     } else {
                         traits_type::construct(m_allocator, insert_ptr + i, value); // 構築
                     }
                 }

            } else {
                // 末尾への挿入
                for (size_type i = 0; i < count; ++i) {
                    traits_type::construct(m_allocator, insert_ptr + i, value);
                }
            }
        }
        m_size += count;
        return m_data + index;
    }

    /**
     * @brief 指定位置にイテレータ範囲の要素を挿入
     * @tparam InputIt イテレータ型 (入力イテレータ要件を満たす)
     * @param pos 挿入位置を示すイテレータ
     * @param first 範囲の開始
     * @param last 範囲の終端
     * @return 最初に挿入された要素を指すイテレータ
     */
    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        // TODO: イテレータカテゴリに応じて最適化 (std::distanceが使えるか等)
        size_type index = pos - cbegin();
        if (first == last) return m_data + index;

        // 一旦vectorにコピーしてからinsert(pos, count, value)のような実装も可能だが、
        // ここでは単純に一つずつ挿入する (効率は悪い可能性がある)
        vector temp(first, last, m_allocator); // 一時的にコピー
        return insert(pos, std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end())); // ムーブで挿入
    }

     /**
     * @brief 指定位置にイテレータ範囲の要素を挿入 (ムーブ版ヘルパー)
     */
    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, std::move_iterator<InputIt> first, std::move_iterator<InputIt> last) {
        const size_type index = pos - cbegin();
        const size_type count = std::distance(first, last); // ForwardIterator以上が必要
        BLUESTL_ASSERT(index <= m_size);
        if (count == 0) return m_data + index;

         if (m_size + count > m_capacity) {
            size_type new_cap = calculate_new_capacity(m_size + count);
            reserve(new_cap);
            // reserveによりイテレータが無効になるため、ポインタを再計算
            pointer insert_ptr = m_data + index;
            pointer old_end = m_data + m_size; // reserve前のサイズ
            pointer new_end = m_data + m_size + count; // reserve後のサイズ

            // 後ろの要素をムーブ
            if (index < m_size) {
                 pointer move_src_start = m_data + index;
                 pointer move_dst_start = m_data + index + count;
                 // 後ろからムーブ構築
                 for(size_type i = 0; i < (m_size - index); ++i) {
                     traits_type::construct(m_allocator, new_end - 1 - i, std::move_if_noexcept(*(old_end - 1 - i)));
                 }
                 // ムーブ元の要素を破棄
                 for(size_type i = 0; i < (m_size - index); ++i) {
                     traits_type::destroy(m_allocator, move_src_start + i);
                 }
            }
            // 新しい要素をムーブ構築
            pointer current = insert_ptr;
            for (; first != last; ++first, ++current) {
                traits_type::construct(m_allocator, current, *first); // *first は右辺値参照のはず
            }

        } else {
             // 容量が足りる場合
            pointer insert_ptr = m_data + index;
            const size_type elems_after = m_size - index;
            pointer old_end = m_data + m_size;

            if (elems_after > 0) {
                // 後ろに移動する要素がある
                 if (elems_after > count) {
                    // 移動元と移動先が重ならない部分をムーブ構築
                    for(size_type i = 0; i < count; ++i) {
                        traits_type::construct(m_allocator, old_end + i, std::move_if_noexcept(*(old_end - count + i)));
                    }
                    // 重なる部分をムーブ代入 (後ろから)
                    for(size_type i = 0; i < elems_after - count; ++i) {
                         *(old_end - 1 - i) = std::move_if_noexcept(*(old_end - 1 - i - count));
                    }
                } else {
                    // 全てムーブ構築 (後ろから)
                     for(size_type i = 0; i < elems_after; ++i) {
                        traits_type::construct(m_allocator, old_end + count - 1 - i, std::move_if_noexcept(*(old_end - 1 - i)));
                    }
                }
                 // 挿入位置に値をムーブ代入またはムーブ構築
                 pointer current = insert_ptr;
                 size_type i = 0;
                 for (; first != last; ++first, ++current, ++i) {
                     if (i < elems_after) {
                         *current = *first; // ムーブ代入
                     } else {
                         traits_type::construct(m_allocator, current, *first); // ムーブ構築
                     }
                 }

            } else {
                // 末尾への挿入
                pointer current = insert_ptr;
                for (; first != last; ++first, ++current) {
                    traits_type::construct(m_allocator, current, *first);
                }
            }
        }
        m_size += count;
        return m_data + index;
    }


    /**
     * @brief 指定位置の要素を削除
     * @param pos 削除する要素を指すイテレータ
     * @return 削除された要素の次の要素を指すイテレータ
     */
    iterator erase(const_iterator pos) {
        const size_type index = pos - cbegin();
        BLUESTL_ASSERT(index < m_size);
        pointer p = m_data + index;

        // 要素を破棄
        traits_type::destroy(m_allocator, p);

        // 後続の要素を前にムーブ
        if (index < m_size - 1) {
            pointer next_p = p + 1;
            pointer end_p = m_data + m_size;
            for (; next_p != end_p; ++p, ++next_p) {
                *p = std::move_if_noexcept(*next_p); // ムーブ代入
            }
             // ムーブ元の最後の要素を破棄 (ムーブ代入されたので不要)
             // traits_type::destroy(m_allocator, p); // 不要
        }

        --m_size;
        return m_data + index;
    }

    /**
     * @brief 指定範囲の要素を削除
     * @param first 削除範囲の開始イテレータ
     * @param last 削除範囲の終端イテレータ
     * @return 削除された範囲の次の要素を指すイテレータ
     */
    iterator erase(const_iterator first, const_iterator last) {
        const size_type first_index = first - cbegin();
        const size_type last_index = last - cbegin();
        BLUESTL_ASSERT(first_index <= last_index && last_index <= m_size);

        if (first_index == last_index) {
            return m_data + first_index;
        }

        pointer p_first = m_data + first_index;
        pointer p_last = m_data + last_index;
        const size_type count = last_index - first_index;

        // 範囲内の要素を破棄
        for (pointer p = p_first; p != p_last; ++p) {
            traits_type::destroy(m_allocator, p);
        }

        // 後続の要素を前にムーブ
        if (last_index < m_size) {
            pointer move_dest = p_first;
            pointer move_src = p_last;
            pointer end_p = m_data + m_size;
            for (; move_src != end_p; ++move_dest, ++move_src) {
                *move_dest = std::move_if_noexcept(*move_src); // ムーブ代入
            }
            // ムーブ元の要素を破棄 (ムーブ代入されたので不要)
            // for (pointer p = move_dest; p != end_p; ++p) {
            //     traits_type::destroy(m_allocator, p);
            // }
        }

        m_size -= count;
        return m_data + first_index;
    }

    /**
     * @brief 要素を末尾に追加 (コピー)
     * @param value 追加する値
     */
    void push_back(const T& value) {
        emplace_back(value); // emplace_backに委譲
    }

    /**
     * @brief 要素を末尾に追加 (ムーブ)
     * @param value 追加する値 (ムーブされる)
     */
    void push_back(T&& value) {
        emplace_back(std::move(value)); // emplace_backに委譲
    }

    /**
     * @brief 要素を末尾に直接構築 (emplace_back)
     * @tparam Args 構築に使用する引数の型
     * @param args 構築に使用する引数
     * @return 構築された要素への参照
     */
    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (m_size == m_capacity) {
            reserve(calculate_new_capacity(m_size + 1));
        }
        pointer p = m_data + m_size;
        traits_type::construct(m_allocator, p, std::forward<Args>(args)...);
        ++m_size;
        return *p;
    }

    /**
     * @brief 末尾の要素を削除
     */
    void pop_back() {
        BLUESTL_ASSERT(!empty());
        --m_size;
        traits_type::destroy(m_allocator, m_data + m_size);
    }

    /**
     * @brief 要素数を変更
     * @param new_size 新しい要素数
     * @details サイズが小さくなる場合、超過分の要素は破棄されます。
     *          サイズが大きくなる場合、新しい要素はデフォルト構築されます。
     */
    void resize(size_type new_size) {
         resize(new_size, T()); // デフォルト値でリサイズ
    }

    /**
     * @brief 要素数を変更し、追加要素を指定値で初期化
     * @param new_size 新しい要素数
     * @param value 追加時の初期値
     */
    void resize(size_type new_size, const T& value) {
        if (new_size < m_size) {
            // サイズを縮小
            pointer erase_start = m_data + new_size;
            pointer erase_end = m_data + m_size;
            for (pointer p = erase_start; p != erase_end; ++p) {
                traits_type::destroy(m_allocator, p);
            }
            m_size = new_size;
        } else if (new_size > m_size) {
            // サイズを拡大
            reserve(new_size); // 必要なら再確保
            pointer construct_start = m_data + m_size;
            pointer construct_end = m_data + new_size;
            for (pointer p = construct_start; p != construct_end; ++p) {
                traits_type::construct(m_allocator, p, value); // 指定値で構築
            }
            m_size = new_size;
        }
        // new_size == m_size の場合は何もしない
    }

    /**
     * @brief vector同士の内容を入れ替え
     * @param other 入れ替え先vector
     */
    void swap(vector& other) noexcept(
        std::is_nothrow_swappable_v<Allocator> ||
        !traits_type::propagate_on_container_swap::value
    ) {
        if (this == &other) return;

        if constexpr (traits_type::propagate_on_container_swap::value) {
            using std::swap;
            swap(m_allocator, other.m_allocator);
        } else {
            // アロケータを交換しない場合、アロケータが等しい必要がある
            // is_always_equal が true ならチェック不要
            if constexpr (!traits_type::is_always_equal::value) {
                 BLUESTL_ASSERT(m_allocator == other.m_allocator);
            }
        }
        // データメンバを交換
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
        swap(m_capacity, other.m_capacity);
    }

    /**
     * @brief 指定された数の要素で内容を置き換える
     * @param count 新しい要素数
     * @param value 新しい要素の値
     */
    void assign(size_type count, const T& value) {
        clear(); // 既存の要素を破棄
        reserve(count); // 十分な容量を確保
        for (size_type i = 0; i < count; ++i) {
            traits_type::construct(m_allocator, m_data + i, value);
        }
        m_size = count;
    }

    /**
     * @brief イテレータ範囲の要素で内容を置き換える
     * @tparam InputIt イテレータ型 (入力イテレータ要件を満たす)
     * @param first 範囲の開始
     * @param last 範囲の終端
     */
    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        // TODO: イテレータカテゴリに応じて効率化 (std::distanceが使えるか等)
        for (; first != last; ++first) {
            emplace_back(*first); // emplace_back を使う
        }
    }

     /**
     * @brief イニシャライザリストで内容を置き換える
     * @param ilist 初期化リスト
     */
    void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }


    // --- アロケータ ---

    /**
     * @brief アロケータのコピーを取得
     * @return アロケータのコピー
     */
    allocator_type get_allocator() const noexcept {
        return m_allocator;
    }

    /**
     * @brief アロケータの参照を取得
     * @return アロケータの参照
     */
    const allocator_type& get_allocator_ref() const noexcept {
        return m_allocator;
    }

    // --- 比較演算子 ---
    friend bool operator==(const vector& lhs, const vector& rhs) {
        if (lhs.m_size != rhs.m_size) return false;
        // std::equal を使うのが簡潔だが、依存を減らすためループで比較
        for (size_type i = 0; i < lhs.m_size; ++i) {
            if (!(lhs.m_data[i] == rhs.m_data[i])) return false; // operator== が定義されている前提
        }
        return true;
    }

    // operator!= は operator== から自動導出 (C++20)
    // friend bool operator!=(const vector& lhs, const vector& rhs) { return !(lhs == rhs); }

    // 三方比較演算子 (C++20)
    friend std::compare_three_way_result_t<T> operator<=>(const vector& lhs, const vector& rhs) {
         // std::lexicographical_compare_three_way を使うのが簡潔
         const_pointer l = lhs.m_data;
         const_pointer r = rhs.m_data;
         const size_type ln = lhs.m_size;
         const size_type rn = rhs.m_size;
         const size_type n = std::min(ln, rn);
         for(size_type i = 0; i < n; ++i) {
             if (auto cmp = l[i] <=> r[i]; cmp != 0) { // T に operator<=> が定義されている前提
                 return cmp;
             }
         }
         return ln <=> rn; // サイズで比較
    }


   private:
    Allocator m_allocator;  // アロケータ (値で保持)
    pointer m_data;         // データ領域へのポインタ
    size_type m_size;       // 現在の要素数
    size_type m_capacity;   // 現在の容量 (要素数)

    // --- プライベートヘルパー関数 ---

    /**
     * @brief 新しい容量を計算する (現在の容量に基づく)
     * @param required_capacity 少なくとも必要な容量
     * @return 計算された新しい容量
     * @details 一般的な 1.5倍 or 2倍 成長戦略。オーバーフローも考慮。
     */
    size_type calculate_new_capacity(size_type required_capacity) const {
        const size_type current_cap = m_capacity;
        const size_type max = max_size();
        if (current_cap >= max / 2) { // オーバーフローチェック
             BLUESTL_ASSERT(required_capacity <= max); // 要求が最大を超えていたらアサート
             return max;
        }
        // 2倍または要求された容量の大きい方を選ぶ (ただしmaxを超えない)
        return std::min(max, std::max(current_cap * 2, required_capacity));
    }

    /**
     * @brief メモリを解放する
     */
    void deallocate_memory() noexcept {
         if (m_data) {
            traits_type::deallocate(m_allocator, m_data, m_capacity);
            m_data = nullptr; // 解放後はnullptrにしておく
            m_capacity = 0;   // 容量もリセット
        }
    }

    /**
     * @brief 格納されている全要素のデストラクタを呼び出す
     */
    void destroy_elements() noexcept {
        if (m_data) {
            for (size_type i = 0; i < m_size; ++i) {
                traits_type::destroy(m_allocator, m_data + i);
            }
        }
    }

    /**
     * @brief 指定された数の要素を指定値で確保・構築する (コンストラクタ用)
     */
    void allocate_and_construct(size_type count, const T& value) {
        BLUESTL_ASSERT(count > 0);
        reserve(count); // メモリ確保
        // 例外安全性を考慮しない (Bluestlの方針)
        for (size_type i = 0; i < count; ++i) {
            traits_type::construct(m_allocator, m_data + i, value);
        }
        m_size = count;
    }

};

// --- 非メンバ関数 swap ---
template <typename T, typename Allocator>
void swap(vector<T, Allocator>& lhs, vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

// --- Deduction guide (C++17以降) ---
// イテレータ範囲からの推論
template <std::input_iterator InputIt, // std::input_iterator を使用
          typename Allocator = allocator<typename std::iterator_traits<InputIt>::value_type>>
vector(InputIt, InputIt, Allocator = Allocator())
    -> vector<typename std::iterator_traits<InputIt>::value_type, Allocator>;

// イニシャライザリストからの推論
template <typename T, typename Allocator = allocator<T>>
vector(std::initializer_list<T>, Allocator = Allocator())
    -> vector<T, Allocator>;


} // namespace bluestl
