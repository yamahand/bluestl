// -----------------------------------------------------------------------------
// Bluestl fixed_hash_map.h
// C++20準拠・STL風インターフェースの固定サイズハッシュマップ
// -----------------------------------------------------------------------------
/**
 * @file fixed_hash_map.h
 * @brief Bluestlプロジェクトのfixed_hash_mapクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * fixed_hash_mapは、固定サイズのハッシュマップを提供します。
 * - RTTIなし、例外なし、ヒープ割り当てなし、header-only設計
 * - STL std::unordered_mapに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
 *   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
 *
 * 主な特徴:
 *   - C++20準拠、STL std::unordered_map風のAPI
 *   - RTTI/例外/ヒープ割り当てなし
 *   - header-only、#pragma onceによるインクルードガード
 *   - 固定サイズで動的メモリ割り当てなし
 *   - find/insert/erase/at/operator[] などの操作
 *   - STL std::unordered_mapとの違い: 固定サイズ、例外非対応、RTTI非使用、最小限の実装
 */
#pragma once

#include "assert_handler.h"
#include "hash.h"
#include "pair.h"
#include "optional.h"

namespace bluestl {
/**
 * @class fixed_hash_map
 * @brief 固定サイズのハッシュマップ。STL std::unordered_map風インターフェース。
 * @tparam Key キー型
 * @tparam T 値型
 * @tparam Capacity バケット数（固定）
 *
 * - 固定サイズのハッシュマップを提供
 * - RTTI/例外/ヒープ割り当てなし
 * - STL std::unordered_map風インターフェース
 */
template <typename Key, typename T, std::size_t Capacity = 128>
class fixed_hash_map {
   public:
    /** @brief キー型 */
    using key_type = Key;
    /** @brief 値型 */
    using mapped_type = T;
    /** @brief キーと値のペア型 */
    using value_type = pair<const Key, T>;
    /** @brief サイズ型 */
    using size_type = std::size_t;
    /** @brief 参照型 */
    using reference = value_type&;
    /** @brief const参照型 */
    using const_reference = const value_type&;

    // Constants
    static constexpr size_type npos = static_cast<size_type>(-1);

    /**
     * @struct bucket
     * @brief 内部バケット構造体
     */
    struct bucket {
        pair<Key, T> kv;  ///< キーと値のペア
        bool used;        ///< 使用フラグ
        bool deleted;     ///< 削除フラグ
        /**
         * @brief デフォルトコンストラクタ
         */
        constexpr bucket() noexcept : kv(), used(false), deleted(false) {}
    };

    class iterator;
    class const_iterator;

    /**
     * @brief デフォルトコンストラクタ
     * @details 空のハッシュマップを生成します。全バケットは未使用状態となります。
     *         例外は発生しません。
     */
    constexpr fixed_hash_map() noexcept : size_(0) {}

    /**
     * @brief 要素数を取得します。
     * @return 現在格納されている要素数を返します。
     * @details 計算量はO(1)です。
     */
    constexpr size_type size() const noexcept {
        return size_;
    }
    /**
     * @brief 容量（バケット数）を取得します。
     * @return 固定されたバケット数（Capacity）を返します。
     * @details 計算量はO(1)です。
     */
    constexpr size_type capacity() const noexcept {
        return Capacity;
    }
    /**
     * @brief コンテナが空かどうかを判定します。
     * @retval true 要素が1つも格納されていない場合
     * @retval false 1つ以上の要素が格納されている場合
     * @details 計算量はO(1)です。
     */
    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief すべての要素を削除し、空にします。
     * @details 全バケットを未使用状態に戻し、要素数を0にリセットします。
     *         例外は発生しません。
     */
    constexpr void clear() noexcept {
        for (size_type i = 0; i < Capacity; ++i) {
            buckets_[i].used = false;
            buckets_[i].deleted = false;
        }
        size_ = 0;
    }

    /**
     * @brief キーに対応する値への参照を返します。
     * @param key 検索するキー
     * @return 対応する値への参照。存在しない場合はデフォルト値を挿入し、その参照を返します。
     * @details 例外は発生しません。要素が存在しない場合は新たに挿入されます。
     */
    constexpr T& operator[](const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) return buckets_[idx].kv.second;
        return insert(key, T{}).first->second;
    }

    /**
     * @brief キーに対応する値への参照を返します。
     * @param key 検索するキー
     * @return 対応する値への参照。存在しない場合はダミー値への参照を返します。
     * @details 例外は発生しません。要素が存在しない場合はダミー値（staticなT）を返します。
     */
    constexpr T& at(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return dummy();
        return buckets_[idx].kv.second;
    }
    /**
     * @brief キーに対応する値への参照をoptionalで返します。
     * @param key 検索するキー
     * @return optional<mapped_type&> 値が存在すれば参照、なければ値なし
     * @details 例外は発生しません。
     */
    constexpr bluestl::optional<T&> try_get(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return bluestl::optional<T&>(buckets_[idx].kv.second);
        }
        return bluestl::optional<T&>();
    }
    /**
     * @brief キーに対応する値へのconst参照をoptionalで返します。
     * @param key 検索するキー
     * @return optional<const mapped_type&> 値が存在すればconst参照、なければ値なし
     * @details 例外は発生しません。
     */
    constexpr bluestl::optional<const T&> try_get(const Key& key) const noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return bluestl::optional<const T&>(buckets_[idx].kv.second);
        }
        return bluestl::optional<const T&>();
    }
    /**
     * @brief キーが存在するか判定します。
     * @param key 検索するキー
     * @retval true 存在する
     * @retval false 存在しない
     * @details 例外は発生しません。
     */
    constexpr bool contains(const Key& key) const noexcept {
        return find_index(key) != npos;
    }
    /**
     * @brief キーに対応する値へのconst参照を返します。
     * @param key 検索するキー
     * @return 対応する値へのconst参照。存在しない場合はダミー値へのconst参照を返します。
     * @details 例外は発生しません。
     */
    constexpr const T& at(const Key& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return dummy();
        return buckets_[idx].kv.second;
    }

    /**
     * @brief 指定したキーの要素を検索します。
     * @param key 検索するキー
     * @return 見つかった場合は該当要素を指すイテレータ、見つからなければend()を返します。
     * @details 例外は発生しません。
     */
    constexpr iterator find(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end();
        return iterator(this, idx);
    }
    /**
     * @brief 指定したキーの要素を検索します（const版）。
     * @param key 検索するキー
     * @return 見つかった場合は該当要素を指すconstイテレータ、見つからなければend()を返します。
     * @details 例外は発生しません。
     */
    constexpr const_iterator find(const Key& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end();
        return const_iterator(this, idx);
    }

    ///
    template <class... Args>
    pair<iterator, bool> emplace(Args&&... args) {
        // value_type (pair<const Key, T>) を直接構築
        value_type new_value(std::forward<Args>(args)...);

        // キーのハッシュ値を計算
        size_type hash = bluestl::hash(new_value.first) % Capacity;

        // バケットを線形探索
        for (size_type i = 0; i < Capacity; ++i) {
            size_type idx = (hash + i) % Capacity;

            // 未使用or削除済みバケットが見つかった場合
            if (!buckets_[idx].used || buckets_[idx].deleted) {
                // 新しい要素を挿入
                buckets_[idx].kv.first = new_value.first;
                buckets_[idx].kv.second = std::move(new_value.second);
                buckets_[idx].used = true;
                buckets_[idx].deleted = false;
                ++size_;
                return { iterator(this, idx), true };
            }

            // 同じキーが既に存在する場合
            if (buckets_[idx].used && buckets_[idx].kv.first == new_value.first) {
                return { iterator(this, idx), false };
            }
        }

        // 容量が不足している場合
        return { end(), false };
    }

    /**
     * @brief キーと値を直接構築して挿入します（既存なら何もしない）。
     * @tparam Args 値のコンストラクタ引数型
     * @param key 挿入するキー
     * @param args 値のコンストラクタ引数
     * @return 挿入位置のイテレータと、挿入が行われたかどうかのペア
     * @details 既に同じキーが存在する場合は挿入せず、その位置のイテレータとfalseを返します。
     */
    template <typename... Args>
    constexpr pair<iterator, bool> try_emplace(const Key& key, Args&&... args) noexcept {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i) {
            size_type idx = (hash + i) % Capacity;
            if (!buckets_[idx].used || buckets_[idx].deleted) {
                buckets_[idx].kv.first = key;
                buckets_[idx].kv.second = T(std::forward<Args>(args)...);
                buckets_[idx].used = true;
                buckets_[idx].deleted = false;
                ++size_;
                return { iterator(this, idx), true };
            }
            if (buckets_[idx].used && buckets_[idx].kv.first == key) {
                return { iterator(this, idx), false };
            }
        }
        return { end(), false };
    }

    /**
     * @brief キーが存在すれば値を上書き、なければ挿入します。
     * @param key 挿入または上書きするキー
     * @param value 挿入または上書きする値
     * @return 挿入位置のイテレータと、挿入（新規true/上書きfalse）
     */
    constexpr pair<iterator, bool> insert_or_assign(const Key& key, const T& value) noexcept {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i) {
            size_type idx = (hash + i) % Capacity;
            if (!buckets_[idx].used || buckets_[idx].deleted) {
                buckets_[idx].kv.first = key;
                buckets_[idx].kv.second = value;
                buckets_[idx].used = true;
                buckets_[idx].deleted = false;
                ++size_;
                return { iterator(this, idx), true };
            }
            if (buckets_[idx].used && buckets_[idx].kv.first == key) {
                buckets_[idx].kv.second = value;
                return { iterator(this, idx), false };
            }
        }
        return { end(), false };
    }

    /**
     * @brief 指定したキーの要素数を返します（0または1）。
     * @param key 検索するキー
     * @return キーが存在すれば1、なければ0
     */
    constexpr size_type count(const Key& key) const noexcept {
        return contains(key) ? 1 : 0;
    }

    // --- iterator, const_iterator の定義ここから ---
    class iterator {
       public:
        using value_type = typename fixed_hash_map::value_type;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        /**
         * @brief デフォルトコンストラクタ
         */
        constexpr iterator() noexcept : map_(nullptr), idx_(0) {}
        /**
         * @brief 内部用コンストラクタ
         */
        constexpr iterator(fixed_hash_map* map, size_type idx) noexcept : map_(map), idx_(idx) {
            skip_to_valid();
        }
        /**
         * @brief デリファレンス
         */
        constexpr reference operator*() const noexcept {
            return *reinterpret_cast<value_type*>(&map_->buckets_[idx_].kv);
        }
        /**
         * @brief ポインタアクセス
         */
        constexpr pointer operator->() const noexcept {
            return reinterpret_cast<value_type*>(&map_->buckets_[idx_].kv);
        }
        /**
         * @brief 前置インクリメント
         */
        constexpr iterator& operator++() noexcept {
            ++idx_;
            skip_to_valid();
            return *this;
        }
        /**
         * @brief 後置インクリメント
         */
        constexpr iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        /**
         * @brief 等価比較
         */
        constexpr bool operator==(const iterator& rhs) const noexcept {
            return map_ == rhs.map_ && idx_ == rhs.idx_;
        }
        /**
         * @brief 非等価比較
         */
        constexpr bool operator!=(const iterator& rhs) const noexcept {
            return !(*this == rhs);
        }
        // --- 内部用 ---
        fixed_hash_map* map_;
        size_type idx_;

       private:
        constexpr void skip_to_valid() noexcept {
            while (map_ && idx_ < Capacity && (!map_->buckets_[idx_].used || map_->buckets_[idx_].deleted)) ++idx_;
        }
    };

    /**
     * @brief constイテレータ型（前方イテレータ）
     * @details 有効な要素のみを巡回します。
     */
    class const_iterator {
       public:
        using value_type = typename fixed_hash_map::value_type;
        using reference = const value_type&;
        using pointer = const value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        /**
         * @brief デフォルトコンストラクタ
         */
        constexpr const_iterator() noexcept : map_(nullptr), idx_(0) {}
        /**
         * @brief 内部用コンストラクタ
         */
        constexpr const_iterator(const fixed_hash_map* map, size_type idx) noexcept : map_(map), idx_(idx) {
            skip_to_valid();
        }
        /**
         * @brief デリファレンス
         */
        constexpr reference operator*() const noexcept {
            return *reinterpret_cast<const value_type*>(&map_->buckets_[idx_].kv);
        }
        /**
         * @brief ポインタアクセス
         */
        constexpr pointer operator->() const noexcept {
            return reinterpret_cast<const value_type*>(&map_->buckets_[idx_].kv);
        }
        /**
         * @brief 前置インクリメント
         */
        constexpr const_iterator& operator++() noexcept {
            ++idx_;
            skip_to_valid();
            return *this;
        }
        /**
         * @brief 後置インクリメント
         */
        constexpr const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        /**
         * @brief 等価比較
         */
        constexpr bool operator==(const const_iterator& rhs) const noexcept {
            return map_ == rhs.map_ && idx_ == rhs.idx_;
        }
        /**
         * @brief 非等価比較
         */
        constexpr bool operator!=(const const_iterator& rhs) const noexcept {
            return !(*this == rhs);
        }
        // --- 内部用 ---
        const fixed_hash_map* map_;
        size_type idx_;

       private:
        constexpr void skip_to_valid() noexcept {
            while (map_ && idx_ < Capacity && (!map_->buckets_[idx_].used || map_->buckets_[idx_].deleted)) ++idx_;
        }
    };
    // --- iterator, const_iterator の定義ここまで ---

    /**
     * @brief キーと値のペアを挿入します。
     * @param key 挿入するキー
     * @param value 挿入する値
     * @return pair<iterator, bool> 挿入位置のイテレータと、挿入が行われたかどうか
     * @details 既に同じキーが存在する場合は挿入せず、その位置のイテレータとfalseを返します。
     *          容量超過時はend()とfalseを返します。
     */
    constexpr pair<iterator, bool> insert(const Key& key, const T& value) noexcept {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i) {
            size_type idx = (hash + i) % Capacity;
            if (!buckets_[idx].used) {
                buckets_[idx].kv.first = key;
                buckets_[idx].kv.second = value;
                buckets_[idx].used = true;
                ++size_;
                return { iterator(this, idx), true };
            }
            if (buckets_[idx].used && buckets_[idx].kv.first == key) {
                return { iterator(this, idx), false };
            }
        }
        return { end(), false };
    }    /**
     * @brief 指定したイテレータ位置の要素を削除します。
     * @param pos 削除する要素を指すイテレータ
     * @return 削除した次の要素を指すイテレータ
     * @details posがend()の場合はend()を返します。
     */
    constexpr iterator erase(iterator pos) noexcept {
        if (pos == end()) return end();
        size_type idx = pos.idx_;
        buckets_[idx].deleted = true;
        --size_;

        // 削除後、次の有効な要素を探す
        iterator next_it(this, idx + 1);
        return next_it;
    }

    /**
     * @brief 他の fixed_hash_map と内容を入れ替えます。
     * @param other 入れ替え対象の fixed_hash_map
     * @details 例外は発生しません。
     */
    void swap(fixed_hash_map& other) noexcept {
        if (this == &other) {
            return;  // 自分自身との swap は何もしない
        }

        // バケット配列を入れ替え
        for (size_type i = 0; i < Capacity; ++i) {
            std::swap(buckets_[i], other.buckets_[i]);
        }

        // サイズを入れ替え
        std::swap(size_, other.size_);
    }

    /**
     * @brief 先頭イテレータを取得します。
     * @return 先頭要素を指すイテレータ
     */
    constexpr iterator begin() noexcept {
        return iterator(this, 0);
    }
    /**
     * @brief 末尾イテレータを取得します。
     * @return 末尾（終端）を指すイテレータ
     */
    constexpr iterator end() noexcept {
        return iterator(this, Capacity);
    }
    /**
     * @brief 先頭constイテレータを取得します。
     * @return 先頭要素を指すconstイテレータ
     */
    constexpr const_iterator begin() const noexcept {
        return const_iterator(this, 0);
    }
    /**
     * @brief 末尾constイテレータを取得します。
     * @return 末尾（終端）を指すconstイテレータ
     */
    constexpr const_iterator end() const noexcept {
        return const_iterator(this, Capacity);
    }
    /**
     * @brief 先頭constイテレータを取得します。
     * @return 先頭要素を指すconstイテレータ
     */
    constexpr const_iterator cbegin() const noexcept {
        return const_iterator(this, 0);
    }
    /**
     * @brief 末尾constイテレータを取得します。
     * @return 末尾（終端）を指すconstイテレータ
     */
    constexpr const_iterator cend() const noexcept {
        return const_iterator(this, Capacity);
    }

   private:
    /**
     * @brief 内部バケット配列
     * @details 固定長のバケット配列で、各バケットはキーと値のペアおよび使用中フラグを持ちます。
     */
    bucket buckets_[Capacity];
    /**
     * @brief 現在格納されている要素数
     * @details 挿入・削除時に更新されます。
     */
    size_type size_;

    /**
     * @brief 指定したキーのインデックスを検索します。
     * @param key 検索するキー
     * @return 見つかった場合はインデックス、見つからなければnpos
     * @details 線形探索でバケットを走査します。例外は発生しません。
     */
    constexpr size_type find_index(const Key& key) const noexcept {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i) {
            size_type idx = (hash + i) % Capacity;
            // 未使用バケットに到達したら探索終了
            if (!buckets_[idx].used) return npos;
            // !deleted && key一致 判定
            if (!buckets_[idx].deleted && buckets_[idx].kv.first == key) return idx;
        }
        return npos;
    }
    /**
     * @brief バケット構造体からvalue_type型へのポインタに変換します。
     * @param b バケットへのポインタ
     * @return value_type型へのポインタ
     * @details reinterpret_castで変換します。例外は発生しません。
     */
    constexpr value_type* to_value_type(bucket* b) const noexcept {
        return reinterpret_cast<value_type*>(&b->kv);
    }
    /**
     * @brief バケット構造体からconst value_type型へのポインタに変換します。
     * @param b バケットへのconstポインタ
     * @return const value_type型へのポインタ
     * @details reinterpret_castで変換します。例外は発生しません。
     */
    constexpr const value_type* to_value_type(const bucket* b) const noexcept {
        return reinterpret_cast<const value_type*>(&b->kv);
    }
    /**
     * @brief 存在しないキーアクセス時に返すダミー値
     * @return 静的なT型ダミー値への参照
     * @details 例外は発生しません。
     */
    static T& dummy() {
        static T d{};
        return d;
    }
};

/**
 * @brief 2つのfixed_hash_mapが等しいか比較します。
 * @return 全ての要素が等しければtrue
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator==(const fixed_hash_map<Key, T, Capacity>& lhs,
                          const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    if (lhs.size() != rhs.size()) return false;
    for (auto it = lhs.begin(); it != lhs.end(); ++it) {
        auto found = rhs.find(it->first);
        if (found == rhs.end() || !(it->second == found->second)) return false;
    }
    return true;
}

/**
 * @brief 2つのfixed_hash_mapが等しくないか比較します。
 * @return 等しくなければtrue
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator!=(const fixed_hash_map<Key, T, Capacity>& lhs,
                          const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    return !(lhs == rhs);
}

/**
 * @brief 2つのfixed_hash_mapの辞書順比較（小なり）
 * @return lhsがrhsより辞書順で小さい場合true
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator<(const fixed_hash_map<Key, T, Capacity>& lhs,
                         const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();
    for (; it1 != lhs.end() && it2 != rhs.end(); ++it1, ++it2) {
        if (it1->first < it2->first) return true;
        if (it2->first < it1->first) return false;
        if (it1->second < it2->second) return true;
        if (it2->second < it1->second) return false;
    }
    return lhs.size() < rhs.size();
}

/**
 * @brief 2つのfixed_hash_mapの辞書順比較（小なりイコール）
 * @return lhsがrhsより辞書順で小さいか等しい場合true
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator<=(const fixed_hash_map<Key, T, Capacity>& lhs,
                          const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    return !(rhs < lhs);
}

/**
 * @brief 2つのfixed_hash_mapの辞書順比較（大なり）
 * @return lhsがrhsより辞書順で大きい場合true
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator>(const fixed_hash_map<Key, T, Capacity>& lhs,
                         const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    return rhs < lhs;
}

/**
 * @brief 2つのfixed_hash_mapの辞書順比較（大なりイコール）
 * @return lhsがrhsより辞書順で大きいか等しい場合true
 */
template <typename Key, typename T, std::size_t Capacity>
constexpr bool operator>=(const fixed_hash_map<Key, T, Capacity>& lhs,
                          const fixed_hash_map<Key, T, Capacity>& rhs) noexcept {
    return !(lhs < rhs);
}

}  // namespace bluestl

// --- 構造化束縛対応に関する注意 ---
/**
 * @brief 構造化束縛について
 * @details MSVC等一部コンパイラではiterator型へのtuple_size/tuple_element特殊化はサポートされません。
 *          そのため、構造化束縛（auto [k, v] = *it;）を利用したい場合は、
 *          fixed_hash_mapのイテレータが返すvalue_type（pair<const Key, T>）を利用してください。
 *          例: for (auto it = map.begin(); it != map.end(); ++it) { auto [k, v] = *it; ... }
 *          もしくは範囲for: for (const auto& [k, v] : map) { ... }
 */
