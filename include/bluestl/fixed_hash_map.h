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

namespace bluestl
{
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
class fixed_hash_map
{
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
    using reference = value_type &;
    /** @brief const参照型 */
    using const_reference = const value_type &;

    /**
     * @struct bucket
     * @brief 内部バケット構造体
     */
    struct bucket
    {
        pair<Key, T> kv; ///< キーと値のペア
        bool used;       ///< 使用中フラグ
        /**
         * @brief デフォルトコンストラクタ
         */
        constexpr bucket() noexcept : kv(), used(false) {}
    };

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
    constexpr size_type size() const noexcept { return size_; }
    /**
     * @brief 容量（バケット数）を取得します。
     * @return 固定されたバケット数（Capacity）を返します。
     * @details 計算量はO(1)です。
     */
    constexpr size_type capacity() const noexcept { return Capacity; }
    /**
     * @brief コンテナが空かどうかを判定します。
     * @retval true 要素が1つも格納されていない場合
     * @retval false 1つ以上の要素が格納されている場合
     * @details 計算量はO(1)です。
     */
    constexpr bool empty() const noexcept { return size_ == 0; }

    /**
     * @brief すべての要素を削除し、空にします。
     * @details 全バケットを未使用状態に戻し、要素数を0にリセットします。
     *         例外は発生しません。
     */
    constexpr void clear() noexcept
    {
        for (size_type i = 0; i < Capacity; ++i)
            buckets_[i].used = false;
        size_ = 0;
    }

    /**
     * @brief キーに対応する値への参照を返します。
     * @param key 検索するキー
     * @return 対応する値への参照。存在しない場合はデフォルト値を挿入し、その参照を返します。
     * @details 例外は発生しません。要素が存在しない場合は新たに挿入されます。
     */
    constexpr T &operator[](const Key &key) noexcept
    {
        size_type idx = find_index(key);
        if (idx != npos)
            return buckets_[idx].kv.second;
        return insert(key, T{}).first->second;
    }

    /**
     * @brief キーに対応する値への参照を返します。
     * @param key 検索するキー
     * @return 対応する値への参照。存在しない場合はダミー値への参照を返します。
     * @details 例外は発生しません。要素が存在しない場合はダミー値（staticなT）を返します。
     */
    constexpr T &at(const Key &key) noexcept
    {
        size_type idx = find_index(key);
        if (idx == npos)
            return dummy();
        return buckets_[idx].kv.second;
    }

    /**
     * @brief キーと値のペアを挿入します。
     * @param key 挿入するキー
     * @param value 挿入する値
     * @return 挿入位置へのポインタと、挿入が行われたかどうかのペア
     * @retval {value_type*, true} 新規挿入された場合
     * @retval {value_type*, false} 既に同じキーが存在した場合
     * @retval {nullptr, false} 容量超過で挿入できなかった場合
     * @details 例外は発生しません。
     */
    constexpr pair<value_type *, bool> insert(const Key &key, const T &value) noexcept
    {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i)
        {
            size_type idx = (hash + i) % Capacity;
            if (!buckets_[idx].used)
            {
                buckets_[idx].kv.first = key;	 // = value_type(key, value);
                buckets_[idx].kv.second = value; // = value_type(key, value);
                buckets_[idx].used = true;
                ++size_;
                return {to_value_type(&buckets_[idx]), true};
            }
            if (buckets_[idx].used && buckets_[idx].kv.first == key)
            {
                return {to_value_type(&buckets_[idx]), false};
            }
        }
        return {nullptr, false};
    }

    /**
     * @brief 指定したキーの要素を削除します。
     * @param key 削除するキー
     * @retval true 削除に成功した場合
     * @retval false 指定キーが存在しなかった場合
     * @details 例外は発生しません。
     */
    constexpr bool erase(const Key &key) noexcept
    {
        size_type idx = find_index(key);
        if (idx == npos)
            return false;
        buckets_[idx].used = false;
        --size_;
        return true;
    }

    /**
     * @brief 指定したキーの要素を検索します。
     * @param key 検索するキー
     * @return 見つかった場合はvalue_typeへのポインタ、見つからなければnullptr
     * @details 例外は発生しません。
     */
    constexpr value_type *find(const Key &key) noexcept
    {
        size_type idx = find_index(key);
        if (idx == npos)
            return nullptr;
        return to_value_type(&buckets_[idx]);
    }

    /**
     * @brief 指定したキーの要素を検索します（const版）。
     * @param key 検索するキー
     * @return 見つかった場合はconst value_typeへのポインタ、見つからなければnullptr
     * @details 例外は発生しません。
     */
    constexpr const value_type *find(const Key &key) const noexcept
    {
        size_type idx = find_index(key);
        if (idx == npos)
            return nullptr;
        return to_value_type(&buckets_[idx]);
    }

    static constexpr size_type npos = static_cast<size_type>(-1);

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
    constexpr size_type find_index(const Key &key) const noexcept
    {
        size_type hash = bluestl::hash(key) % Capacity;
        for (size_type i = 0; i < Capacity; ++i)
        {
            size_type idx = (hash + i) % Capacity;
            if (!buckets_[idx].used)
                return npos;
            if (buckets_[idx].kv.first == key)
                return idx;
        }
        return npos;
    }
    /**
     * @brief バケット構造体からvalue_type型へのポインタに変換します。
     * @param b バケットへのポインタ
     * @return value_type型へのポインタ
     * @details reinterpret_castで変換します。例外は発生しません。
     */
    constexpr value_type *to_value_type(bucket *b) const noexcept
    {
        return reinterpret_cast<value_type *>(&b->kv);
    }
    /**
     * @brief バケット構造体からconst value_type型へのポインタに変換します。
     * @param b バケットへのconstポインタ
     * @return const value_type型へのポインタ
     * @details reinterpret_castで変換します。例外は発生しません。
     */
    constexpr const value_type *to_value_type(const bucket *b) const noexcept
    {
        return reinterpret_cast<const value_type *>(&b->kv);
    }
    /**
     * @brief 存在しないキーアクセス時に返すダミー値
     * @return 静的なT型ダミー値への参照
     * @details 例外は発生しません。
     */
    static T &dummy()
    {
        static T d{};
        return d;
    }
};

} // namespace bluestl
