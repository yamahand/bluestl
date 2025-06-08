/**
 * @file hash_map.h
 * @brief bluestl::hash_map コンテナの実装。
 *
 * このファイルは、オープンアドレス法（二次プロービング）を使用したハッシュマップコンテナを提供します。
 * STL の std::unordered_map に似たインターフェースを持ちますが、例外を使用せず、
 * アロケータを外部から指定できる点が異なります。
 * BlueStl プロジェクトの一部として、高速なコンパイルと実行、メモリ効率を重視しています。
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <cassert>
#include <iterator> // For iterator tags and distance
#include <algorithm> // For std::max

#include "log_macros.h"
#include "hash.h"
#include "pair.h"
#include "optional.h"

namespace bluestl {

/**
 * @brief オープンアドレス法（二次プロービング）を使用したハッシュマップ。
 *
 * @tparam Key キーの型。ハッシュ可能で比較可能である必要があります。
 * @tparam T 値の型。
 * @tparam Allocator メモリ割り当てに使用するアロケータの型。bluestl::allocator インターフェースを実装している必要があります。
 *
 * std::unordered_map に似たインターフェースを提供しますが、以下の特徴があります。
 * - 例外をスローしません。エラー発生時はアサーションで停止するか、特定の値を返します。
 * - アロケータを外部から参照で受け取ります。
 * - オープンアドレス法（二次プロービング）を採用しています。
 * - 削除された要素はトゥームストーン（墓石）としてマークされ、再ハッシュ時に物理的に削除されます。
 */
template <typename Key, typename T, typename Allocator>
class hash_map {
private:
    /**
     * @brief バケット構造体。メモリレイアウトを最適化し、状態フラグをビットフィールドとして実装。
     */
    struct bucket {
        pair<Key, T> kv;      ///< キーと値のペア
        uint8_t flags = 0;    ///< 状態フラグ（ビットフィールド）
        static constexpr uint8_t USED_FLAG = 1;     ///< 使用中フラグ
        static constexpr uint8_t DELETED_FLAG = 2;  ///< 削除済みフラグ

        constexpr bucket() noexcept : kv(), flags(0) {}

        constexpr bool is_used() const noexcept { return flags & USED_FLAG; }
        constexpr bool is_deleted() const noexcept { return flags & DELETED_FLAG; }
        constexpr void set_used(bool value) noexcept {
            if (value) flags |= USED_FLAG;
            else flags &= ~USED_FLAG;
        }
        constexpr void set_deleted(bool value) noexcept {
            if (value) flags |= DELETED_FLAG;
            else flags &= ~DELETED_FLAG;
        }
        constexpr void clear_flags() noexcept { flags = 0; }
    };

public:
    // Forward declaration of iterators
    template <bool IsConst>
    class hash_iterator;

public:
    using key_type = Key;                 ///< キーの型
    using mapped_type = T;                ///< マップされる値の型
    using value_type = pair<const Key, T>; ///< キーと値のペアの型 (イテレータが指す型)
    using size_type = std::size_t;        ///< サイズを表す符号なし整数型
    using difference_type = std::ptrdiff_t; ///< イテレータ間の距離を表す符号付き整数型
    using allocator_type = Allocator;     ///< アロケータの型
    using iterator = hash_iterator<false>;       ///< 非 const イテレータ
    using const_iterator = hash_iterator<true>; ///< const イテレータ

    // Constants
    /** @brief 初期容量のデフォルト値 */
    static constexpr size_type initial_capacity = 16;
    /** @brief 無効なインデックスを示す値 */
    static constexpr size_type npos = static_cast<size_type>(-1);
    /** @brief 再ハッシュのトリガーとなる最大負荷率 */
    static constexpr float max_load_factor = 0.75f;

    /**
     * @brief ハッシュマップのイテレータ。
     *
     * @tparam IsConst true の場合は const イテレータ、false の場合は非 const イテレータ。
     *
     * forward_iterator_tag を満たします。
     */
    template <bool IsConst>
    class hash_iterator {
    public:
        using iterator_category = std::forward_iterator_tag; ///< イテレータカテゴリ
        /** @brief イテレータが指す値の型。IsConst に応じて const 修飾されます。 */
        using value_type = std::conditional_t<IsConst, const pair<const Key, T>, pair<const Key, T>>;
        using difference_type = std::ptrdiff_t; ///< イテレータ間の距離を表す型
        using pointer = value_type*;           ///< 値へのポインタ型
        using reference = value_type&;         ///< 値への参照型

        /** @brief デフォルトコンストラクタ。end イテレータ相当の無効なイテレータを構築します。 */
        hash_iterator() noexcept : container_(nullptr), index_(0) {}

        /**
         * @brief コンストラクタ。
         * @param container このイテレータが属する hash_map コンテナへのポインタ。
         * @param index バケット配列内のインデックス。
         */
        hash_iterator(const hash_map* container, size_type index) noexcept : container_(container), index_(index) {
            advance_to_valid(); // 有効な要素まで進める
        }

        /** @brief コピーコンストラクタ。 */
        hash_iterator(const hash_iterator& other) noexcept : container_(other.container_), index_(other.index_) {}

        /** @brief コピー代入演算子。 */
        hash_iterator& operator=(const hash_iterator& other) noexcept {
            container_ = other.container_;
            index_ = other.index_;
            return *this;
        }

        /** @brief 等価比較演算子。 */
        bool operator==(const hash_iterator& other) const noexcept {
            return container_ == other.container_ && index_ == other.index_;
        }

        /** @brief 非等価比較演算子。 */
        bool operator!=(const hash_iterator& other) const noexcept {
            return !(*this == other);
        }

        /**
         * @brief 間接参照演算子。
         * @return イテレータが指す要素への参照。
         * @pre イテレータは有効な要素を指している必要があります。
         */
        reference operator*() const noexcept {
            validate();
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].is_used() &&
                           !container_->buckets_[index_].is_deleted());
            return *reinterpret_cast<pointer>(&container_->buckets_[index_].kv);
        }

        /**
         * @brief アロー演算子。
         * @return イテレータが指す要素のメンバへのポインタ。
         * @pre イテレータは有効な要素を指している必要があります。
         */
        pointer operator->() const noexcept {
            validate();
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].is_used() &&
                           !container_->buckets_[index_].is_deleted());
            return reinterpret_cast<pointer>(&container_->buckets_[index_].kv);
        }

        /**
         * @brief 前置インクリメント演算子。次の有効な要素に進みます。
         * @return インクリメント後のイテレータへの参照。
         */
        hash_iterator& operator++() noexcept {
            if (container_ && index_ < container_->capacity_) {
                ++index_;
                advance_to_valid(); // 次の有効なバケットを探す
            }
            return *this;
        }

        /**
         * @brief 後置インクリメント演算子。次の有効な要素に進みます。
         * @return インクリメント前のイテレータのコピー。
         */
        hash_iterator operator++(int) noexcept {
            hash_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        /**
         * @brief 現在のイテレータが指すバケットのインデックスを取得します。
         * @return バケットのインデックス。
         * @note erase 操作などで内部的に使用されます。
         */
        size_type get_index() const noexcept {
            return index_;
        }

    private:
        /** @brief このイテレータが属する hash_map コンテナへのポインタ。const イテレータでもコンテナの状態を変更しないため const。 */
        const hash_map* container_;
        /** @brief 現在指しているバケットのインデックス。 */
        size_type index_;

        /**
         * @brief 現在のインデックスから開始して、次の有効な（使用中で削除されていない）バケットを探し、index_ を更新します。
         *        有効なバケットが見つからない場合は、index_ を capacity_ に設定します (end イテレータの状態)。
         */
        void advance_to_valid() noexcept {
            if (!container_) return;

            while (index_ < container_->capacity_) {
                if (container_->buckets_[index_].is_used() &&
                    !container_->buckets_[index_].is_deleted()) {
                    break;
                }
                ++index_;
            }

            if (index_ >= container_->capacity_) {
                index_ = container_->capacity_;
            }
        }

        /**
         * @brief イテレータの有効性チェック
         */
        void validate() const noexcept {
            BLUESTL_ASSERT(container_ && "Iterator has null container");
            BLUESTL_ASSERT(index_ <= container_->capacity_ && "Iterator index out of bounds");
        }    };

    /**
     * @brief デフォルトコンストラクタ。デフォルトアロケータを使用します。
     */
    hash_map() noexcept
        : size_(0), deleted_count_(0), capacity_(initial_capacity), allocator_(), buckets_(nullptr) {
        allocate_buckets(capacity_);
    }

    /**
     * @brief コンストラクタ。外部アロケータを指定します。
     * @param alloc 使用するアロケータへの参照。hash_map の生存期間中、有効である必要があります。
     */
    hash_map(Allocator& alloc) noexcept
        : size_(0), deleted_count_(0), capacity_(initial_capacity), allocator_(alloc), buckets_(nullptr) {
        allocate_buckets(capacity_);
    }

    /**
     * @brief 範囲コンストラクタ。イテレータ範囲 [first, last) の要素でハッシュマップを初期化します。
     * @tparam InputIt 入力イテレータの型。value_type (pair<const Key, T> または互換型) を指す必要があります。
     * @param first 範囲の開始を示す入力イテレータ。
     * @param last 範囲の終端を示す入力イテレータ。
     * @param alloc 使用するアロケータへの参照。
     */
    template <typename InputIt>
    hash_map(InputIt first, InputIt last, Allocator& alloc) noexcept
        : size_(0), deleted_count_(0), capacity_(initial_capacity), allocator_(alloc), buckets_(nullptr) {
        using category = typename std::iterator_traits<InputIt>::iterator_category;
        size_type count = 0;
        if constexpr (std::is_base_of_v<std::forward_iterator_tag, category>) {
             count = std::distance(first, last);
        } else {
            for(auto it = first; it != last; ++it) {
                ++count;
            }
        }

        size_type required_capacity = (max_load_factor > 0.0f) ? static_cast<size_type>(count / max_load_factor) + 1 : count + 1;
        capacity_ = std::max(initial_capacity, required_capacity);

        allocate_buckets(capacity_);
        insert(first, last);
    }    /**
     * @brief デストラクタ。要素のデストラクタを呼び出し、確保したメモリを解放します。
     */    ~hash_map() noexcept {
        if (buckets_) {
            deallocate_buckets();
        }
    }

    /**
     * @brief コピーコンストラクタ。
     * @param other コピー元の hash_map オブジェクト。
     */
    hash_map(const hash_map& other) noexcept
        : size_(0), deleted_count_(0), capacity_(other.capacity_), allocator_(other.allocator_), buckets_(nullptr) {
        allocate_buckets(capacity_);
        for (size_type i = 0; i < other.capacity_; ++i) {
            if (other.buckets_[i].is_used() && !other.buckets_[i].is_deleted()) {
                insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
            }
        }
    }

    /**
     * @brief コピー代入演算子。
     * @param other コピー元の hash_map オブジェクト。
     * @return *this への参照。
     */        hash_map& operator=(const hash_map& other) noexcept {
            if (this != &other) {
                clear();                if (capacity_ < other.capacity_) {
                    if (buckets_) {
                        deallocate_buckets();
                        buckets_ = nullptr;
                    }
                    capacity_ = other.capacity_;
                    allocate_buckets(capacity_);
                }

            for (size_type i = 0; i < other.capacity_; ++i) {
                if (other.buckets_[i].is_used() && !other.buckets_[i].is_deleted()) {
                    insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
                }
            }
        }
        return *this;
    }

    /**
     * @brief ムーブコンストラクタ。
     * @param other ムーブ元の hash_map オブジェクト。ムーブ後は空の状態になります。
     */
    hash_map(hash_map&& other) noexcept
        : size_(other.size_),
          deleted_count_(other.deleted_count_),
          capacity_(other.capacity_),
          allocator_(other.allocator_),
          buckets_(other.buckets_) {
        other.size_ = 0;
        other.deleted_count_ = 0;
        other.capacity_ = 0;
        other.buckets_ = nullptr;
    }

    /**
     * @brief ムーブ代入演算子。
     * @param other ムーブ元の hash_map オブジェクト。ムーブ後は空の状態になります。
     * @return *this への参照。
     */    hash_map& operator=(hash_map&& other) noexcept {        if (this != &other) {
            if (buckets_) {
                deallocate_buckets();
            }

            size_ = other.size_;
            deleted_count_ = other.deleted_count_;
            capacity_ = other.capacity_;
            buckets_ = other.buckets_;

            other.size_ = 0;
            other.deleted_count_ = 0;
            other.capacity_ = 0;
            other.buckets_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 格納されている要素数を返します。
     * @return 要素数。
     */
    size_type size() const noexcept {
        return size_;
    }
    /**
     * @brief 現在のバケット配列の容量を返します。
     * @return バケット配列の容量。
     */
    size_type capacity() const noexcept {
        return capacity_;
    }
    /**
     * @brief ハッシュマップが空かどうかを返します。
     * @return 空の場合は true、そうでない場合は false。
     */
    bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief ハッシュマップの内容をすべてクリアします。
     *        要素のデストラクタが呼び出されますが、メモリは解放されません。
     */
    void clear() noexcept {
        for (size_type i = 0; i < capacity_; ++i) {
            if (buckets_[i].is_used() && !buckets_[i].is_deleted()) {
                buckets_[i].kv.~pair();
            }
            buckets_[i].clear_flags();
        }
        size_ = 0;
        deleted_count_ = 0;
    }

    /**
     * @brief 最初の要素を指すイテレータを返します。
     * @return 最初の要素を指すイテレータ。マップが空の場合は end() と同じイテレータを返します。
     */
    iterator begin() noexcept {
        return iterator(this, 0);
    }
    /**
     * @brief 最初の要素を指す const イテレータを返します。
     * @return 最初の要素を指す const イテレータ。マップが空の場合は cend() と同じイテレータを返します。
     */
    const_iterator begin() const noexcept {
        return const_iterator(this, 0);
    }
    /**
     * @brief 最初の要素を指す const イテレータを返します。
     * @return 最初の要素を指す const イテレータ。マップが空の場合は cend() と同じイテレータを返します。
     */
    const_iterator cbegin() const noexcept {
        return const_iterator(this, 0);
    }

    /**
     * @brief 末尾の次を指すイテレータ（番兵）を返します。
     * @return 末尾の次を指すイテレータ。
     */
    iterator end() noexcept {
        return iterator(this, capacity_);
    }
    /**
     * @brief 末尾の次を指す const イテレータ（番兵）を返します。
     * @return 末尾の次を指す const イテレータ。
     */
    const_iterator end() const noexcept {
        return const_iterator(this, capacity_);
    }
    /**
     * @brief 末尾の次を指す const イテレータ（番兵）を返します。
     * @return 末尾の次を指す const イテレータ。
     */
    const_iterator cend() const noexcept {
        return const_iterator(this, capacity_);
    }

    /**
     * @brief 指定されたキーに対応する要素への参照を返します。
     *        キーが存在しない場合は、デフォルト構築された値を持つ新しい要素を挿入し、その参照を返します。
     * @param key 検索または挿入するキー。
     * @return キーに対応する値への参照。
     * @note 挿入が発生する可能性があるため、非 const 操作です。
     * @warning キーが存在しない場合にデフォルトコンストラクタが呼ばれるため、mapped_type はデフォルト構築可能である必要があります。
     */
    mapped_type& operator[](const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) return buckets_[idx].kv.second;

        if (should_rehash()) rehash(calculate_new_capacity());

        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = mapped_type();
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return buckets_[idx].kv.second;
        }

        BLUESTL_ASSERT(false && "Hash map insertion failed after rehash check");
        return dummy_value();
    }

    /**
     * @brief 指定されたキーに対応する要素への参照を返します (ムーブ版)。
     *        キーが存在しない場合は、デフォルト構築された値を持つ新しい要素を挿入し、その参照を返します。
     * @param key 検索または挿入するキー (ムーブ)。
     * @return キーに対応する値への参照。
     * @note 挿入が発生する可能性があるため、非 const 操作です。
     * @warning キーが存在しない場合にデフォルトコンストラクタが呼ばれるため、mapped_type はデフォルト構築可能である必要があります。
     */
    mapped_type& operator[](key_type&& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) return buckets_[idx].kv.second;

        if (should_rehash()) rehash(calculate_new_capacity());

        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);
            buckets_[idx].kv.second = mapped_type();
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return buckets_[idx].kv.second;
        }

        BLUESTL_ASSERT(false && "Hash map insertion failed after rehash check");
        return dummy_value();
    }

    /**
     * @brief 指定されたキーに対応する要素への参照を返します。
     *        キーが存在しない場合はアサーションで失敗します（デバッグビルド）。
     * @param key 検索するキー。
     * @return キーに対応する値への参照。
     * @pre キーはハッシュマップ内に存在しなければなりません。
     */
    mapped_type& at(const key_type& key) noexcept {
        size_type idx = find_index(key);
        BLUESTL_ASSERT(idx != npos && "hash_map::at key does not exist");
        if (idx == npos) return dummy_value();
        return buckets_[idx].kv.second;
    }

    /**
     * @brief 指定されたキーに対応する要素への const 参照を返します。
     *        キーが存在しない場合はアサーションで失敗します（デバッグビルド）。
     * @param key 検索するキー。
     * @return キーに対応する値への const 参照。
     * @pre キーはハッシュマップ内に存在しなければなりません。
     */
    const mapped_type& at(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        BLUESTL_ASSERT(idx != npos && "hash_map::at key does not exist");
        if (idx == npos) return const_dummy_value();
        return buckets_[idx].kv.second;
    }

    /**
     * @brief キーに対応する値への参照を optional で返します。
     *        キーが見つからない場合は、値なしの optional を返します。
     * @param key 検索するキー。
     * @return optional<mapped_type&> キーが見つかった場合はその値への参照を含む optional、見つからない場合は値なしの optional。
     */
    optional<mapped_type&> try_get(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return optional<mapped_type&>(buckets_[idx].kv.second);
        }
        return optional<mapped_type&>();
    }

    /**
     * @brief キーに対応する値への const 参照を optional で返します。
     *        キーが見つからない場合は、値なしの optional を返します。
     * @param key 検索するキー。
     * @return optional<const mapped_type&> キーが見つかった場合はその値への const 参照を含む optional、見つからない場合は値なしの optional。
     */
    optional<const mapped_type&> try_get(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return optional<const mapped_type&>(buckets_[idx].kv.second);
        }
        return optional<const mapped_type&>();
    }

    /**
     * @brief キーと値のペアを挿入します。
     *        キーが既に存在する場合は挿入せず、既存の要素を指すイテレータと false を返します。
     * @param key 挿入するキー。
     * @param value 挿入する値。
     * @return pair<iterator, bool> 挿入された要素を指すイテレータと、挿入が成功したか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    pair<iterator, bool> insert(const key_type& key, const mapped_type& value) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());

        size_type idx = find_index(key);
        if (idx != npos) {
            return { iterator(this, idx), false };
        }

        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = value;
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }

        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return { end(), false };
    }

    /**
     * @brief キーと値のペアを挿入します (ムーブ版)。
     *        キーが既に存在する場合は挿入せず、既存の要素を指すイテレータと false を返します。
     * @param key 挿入するキー (ムーブ)。
     * @param value 挿入する値 (ムーブ)。
     * @return pair<iterator, bool> 挿入された要素を指すイテレータと、挿入が成功したか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    pair<iterator, bool> insert(key_type&& key, mapped_type&& value) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());

        size_type idx = find_index(key);
        if (idx != npos) {
            return { iterator(this, idx), false };
        }

        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);
            buckets_[idx].kv.second = std::move(value);
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }

        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return { end(), false };
    }

    /**
     * @brief イテレータ範囲 [first, last) の要素を挿入します。
     * @tparam InputIt 入力イテレータの型。value_type (pair<const Key, T> または互換型) を指す必要があります。
     * @param first 範囲の開始を示す入力イテレータ。
     * @param last 範囲の終端を示す入力イテレータ。
     * @note 各要素について insert(key, value) を呼び出します。キーが既に存在する場合は挿入されません。
     */
    template <typename InputIt>
    void insert(InputIt first, InputIt last) noexcept {
        for (; first != last; ++first) {
            insert(first->first, first->second);
        }
    }

    /**
     * @brief 指定された引数で値を構築し、要素を挿入します。
     *        キーが既に存在する場合、挿入は行われません（try_emplace と同じ動作）。
     * @tparam Args 値 (mapped_type) のコンストラクタに渡される引数の型。
     * @param key 挿入するキー。
     * @param args 値を構築するための引数。完全転送されます。
     * @return pair<iterator, bool> 挿入された要素または既存の要素を指すイテレータと、
     *         挿入が行われたか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    template <typename... Args>
    pair<iterator, bool> emplace(const key_type& key, Args&&... args) noexcept {
        return try_emplace(key, std::forward<Args>(args)...);
    }

    /**
     * @brief 指定された引数で値を構築し、要素を挿入します (キーをムーブする版)。
     *        キーが既に存在する場合、挿入は行われません（try_emplace と同じ動作）。
     * @tparam Args 値 (mapped_type) のコンストラクタに渡される引数の型。
     * @param key 挿入するキー (ムーブ)。
     * @param args 値を構築するための引数。完全転送されます。
     * @return pair<iterator, bool> 挿入された要素または既存の要素を指すイテレータと、
     *         挿入が行われたか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    template <typename... Args>
    pair<iterator, bool> emplace(key_type&& key, Args&&... args) noexcept {
        return try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    /**
     * @brief キーが存在しない場合にのみ、指定された引数で値を構築して要素を挿入します。
     *        キーが既に存在する場合は何もしません。
     * @tparam Args 値 (mapped_type) のコンストラクタに渡される引数の型。
     * @param key 挿入を試みるキー。
     * @param args 値を構築するための引数。完全転送されます。
     * @return pair<iterator, bool> 挿入された要素または既存の要素を指すイテレータと、
     *         挿入が行われたか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    template <typename... Args>
    pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());
        size_type idx = find_index(key);
        if (idx != npos) {
            return { iterator(this, idx), false };
        }
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            new (&buckets_[idx].kv.second) mapped_type(std::forward<Args>(args)...);
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }
        BLUESTL_ASSERT(false && "Hash map try_emplace failed");
        return { end(), false };
    }

    /**
     * @brief キーが存在しない場合にのみ、指定された引数で値を構築して要素を挿入します (キーをムーブする版)。
     *        キーが既に存在する場合は何もしません。
     * @tparam Args 値 (mapped_type) のコンストラクタに渡される引数の型。
     * @param key 挿入を試みるキー (ムーブ)。
     * @param args 値を構築するための引数。完全転送されます。
     * @return pair<iterator, bool> 挿入された要素または既存の要素を指すイテレータと、
     *         挿入が行われたか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    template <typename... Args>
    pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());
        size_type idx = find_index(key);
        if (idx != npos) {
            return { iterator(this, idx), false };
        }
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);
            new (&buckets_[idx].kv.second) mapped_type(std::forward<Args>(args)...);
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }
        BLUESTL_ASSERT(false && "Hash map try_emplace failed");
        return { end(), false };
    }    /**
     * @brief 指定されたキーに対応する要素を削除します。
     * @param key 削除する要素のキー。
     * @return 要素が削除された場合は true、キーが見つからなかった場合は false。
     */
    bool erase(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return false;        // 要素を破棄してトゥームストーンとしてマーク
        buckets_[idx].kv.~pair<Key, T>();
        buckets_[idx].set_deleted(true);
        // Note: is_used() はトゥームストーンでは true のままにする
        --size_;
        ++deleted_count_;

        // 削除された要素が多い場合、またはロードファクターが低い場合にリハッシュを実行
        constexpr double max_deleted_ratio = 0.25; // 削除された要素が25%を超えた場合
        if (capacity_ > 0 &&
            (deleted_count_ > capacity_ * max_deleted_ratio ||
             deleted_count_ > size_)) {
            rehash(calculate_optimal_capacity());
        }

        return true;
    }    /**
     * @brief 指定されたイテレータが指す要素を削除します。
     * @param pos 削除する要素を指すイテレータ。end() であってはなりません。
     * @return 削除された要素の次の要素を指すイテレータ。
     * @pre pos は有効な要素を指しており、end() ではありません。
     */
    iterator erase(iterator pos) noexcept {
        BLUESTL_ASSERT(pos != end() && "Cannot erase end iterator");
        if (pos == end()) return end();

        size_type idx = pos.get_index();

        // 次のイテレータを削除前に計算
        iterator next_it(this, idx);
        ++next_it;        if (idx < capacity_ && buckets_[idx].is_used() && !buckets_[idx].is_deleted()) {
            // 要素を破棄してトゥームストーンとしてマーク
            buckets_[idx].kv.~pair<Key, T>();
            buckets_[idx].set_deleted(true);
            // Note: is_used() はトゥームストーンでは true のままにする
            --size_;
            ++deleted_count_;

            // 削除された要素が多い場合、またはロードファクターが低い場合にリハッシュを実行
            constexpr double max_deleted_ratio = 0.25; // 削除された要素が25%を超えた場合
            if (capacity_ > 0 &&
                (deleted_count_ > capacity_ * max_deleted_ratio ||
                 deleted_count_ > size_)) {
                // リハッシュ後にイテレータを再構築
                size_type old_next_idx = next_it.get_index();
                rehash(calculate_optimal_capacity());

                // リハッシュ後の適切な次のイテレータを見つける
                if (old_next_idx >= capacity_) {
                    return end();
                } else {
                    next_it = iterator(this, old_next_idx);
                    if (!buckets_[old_next_idx].is_used() || buckets_[old_next_idx].is_deleted()) {
                        ++next_it; // 次の有効な要素に進む
                    }
                }
            }
        }

        return next_it;
    }

    /**
     * @brief 指定された const イテレータが指す要素を削除します。
     * @param pos 削除する要素を指す const イテレータ。cend() であってはなりません。
     * @return 削除された要素の次の要素を指すイテレータ。
     * @pre pos は有効な要素を指しており、cend() ではありません。
     */
    iterator erase(const_iterator pos) noexcept {
        BLUESTL_ASSERT(pos != cend() && "Cannot erase end iterator");
        if (pos == cend()) return end();

        size_type idx = pos.get_index();
        iterator next_it(this, idx);
        ++next_it;

        if (idx < capacity_ && buckets_[idx].is_used() && !buckets_[idx].is_deleted()) {
            buckets_[idx].set_deleted(true);
            --size_;
            ++deleted_count_;
            if (deleted_count_ > size_) rehash(capacity_);
        }

        return next_it;
    }

    /**
     * @brief イテレータ範囲 [first, last) の要素を削除します。
     * @param first 削除する範囲の開始を示すイテレータ。
     * @param last 削除する範囲の終端を示すイテレータ。
     * @return 削除された最後の要素の次の要素を指すイテレータ (last と同じになるはず)。
     */
    iterator erase(iterator first, iterator last) noexcept {
        if (first == end() || first == last) {
            return last;
        }

        iterator current = first;
        while (current != last) {
            current = erase(current);
        }
        return current;
    }

    /**
     * @brief 指定されたキーに対応する要素を検索します。
     * @param key 検索するキー。
     * @return キーが見つかった場合はその要素を指すイテレータ、見つからなかった場合は end()。
     */
    iterator find(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end();
        return iterator(this, idx);
    }

    /**
     * @brief 指定されたキーを持つ要素を検索します (const 版)。
     * @param key 検索するキー。
     * @return キーが見つかった場合はその要素を指す const イテレータ、見つからなかった場合は cend()。
     */
    const_iterator find(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return cend();
        return const_iterator(this, idx);
    }

    /**
     * @brief 指定されたキーを持つ要素が存在するかどうかを確認します。
     * @param key 確認するキー。
     * @return キーが存在する場合は true、存在しない場合は false。
     */
    bool contains(const key_type& key) const noexcept {
        return find_index(key) != npos;
    }

    /**
     * @brief キーが存在しない場合は要素を挿入し、存在する場合は値を代入します。
     * @param key 挿入または代入するキー。
     * @param value 挿入または代入する値。
     * @return pair<iterator, bool> 挿入または更新された要素を指すイテレータと、
     *         新規に挿入されたか (true) / 値が代入されたか (false) を示す bool 値のペア。
     */
    pair<iterator, bool> insert_or_assign(const key_type& key, const mapped_type& value) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());
        size_type idx = find_index(key);
        if (idx != npos) {
            buckets_[idx].kv.second = value;
            return { iterator(this, idx), false };
        }
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = value;
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }
        BLUESTL_ASSERT(false && "Hash map insert_or_assign failed");
        return { end(), false };
    }

    /**
     * @brief キーが存在しない場合は要素を挿入し、存在する場合は値を代入します (ムーブ版)。
     * @param key 挿入または代入するキー (ムーブ)。
     * @param value 挿入または代入する値 (ムーブ)。
     * @return pair<iterator, bool> 挿入または更新された要素を指すイテレータと、
     *         新規に挿入されたか (true) / 値が代入されたか (false) を示す bool 値のペア。
     */
    pair<iterator, bool> insert_or_assign(key_type&& key, mapped_type&& value) noexcept {
        if (should_rehash()) rehash(calculate_new_capacity());
        size_type idx = find_index(key);
        if (idx != npos) {
            buckets_[idx].kv.second = std::move(value);
            return { iterator(this, idx), false };
        }
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);
            buckets_[idx].kv.second = std::move(value);
            buckets_[idx].set_used(true);
            buckets_[idx].set_deleted(false);
            ++size_;
            return { iterator(this, idx), true };
        }
        BLUESTL_ASSERT(false && "Hash map insert_or_assign failed");
        return { end(), false };
    }

    /**
     * @brief Range-based for対応
     */
    struct range_type {
        hash_map& map;
        auto begin() noexcept { return map.begin(); }
        auto end() noexcept { return map.end(); }
    };

    range_type entries() noexcept { return {*this}; }

private:
    /** @brief バケット配列へのポインタ。 */
    bucket* buckets_;
    /** @brief 現在格納されている要素数（削除済みを除く）。 */
    size_type size_;
    /** @brief 削除済みとしてマークされた要素数（トゥームストーンの数）。 */
    size_type deleted_count_;
    /** @brief バケット配列の現在の容量。 */
    size_type capacity_;
    /** @brief メモリ割り当てに使用するアロケータへの参照。 */
    Allocator& allocator_;

    // Make hash_iterator a friend to access private members
    template <bool IsConst>
    friend class hash_iterator;    /**
     * @brief 指定された容量 n でバケット配列を割り当て、各バケットをデフォルト構築します。
     * @param n 割り当てるバケットの数。
     */
    void allocate_buckets(size_type n) noexcept {
        using bucket_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<bucket>;
        bucket_allocator bucket_alloc(allocator_);
        buckets_ = bucket_alloc.allocate(n);
        for (size_type i = 0; i < n; ++i) {
            new (&buckets_[i]) bucket();
        }
    }

    /**
     * @brief バケット配列の割り当てとデストラクタ呼び出しを行います。
     */
    void deallocate_buckets() noexcept {
        if (buckets_) {
            destroy_buckets();
            using bucket_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<bucket>;
            bucket_allocator bucket_alloc(allocator_);
            bucket_alloc.deallocate(buckets_, capacity_);
            buckets_ = nullptr;
        }
    }

    /**
     * @brief バケット配列内の有効な要素（使用中で削除されていない）のデストラクタを呼び出します。
     * @note メモリの解放は行いません。
     */
    void destroy_buckets() noexcept {
        for (size_type i = 0; i < capacity_; ++i) {
            if (buckets_[i].is_used() && !buckets_[i].is_deleted()) {
                buckets_[i].kv.~pair();
            }
        }
    }

    /**
     * @brief 再ハッシュが必要かどうかを判断します。
     * @return 再ハッシュが必要な場合は true、そうでない場合は false。
     */
    bool should_rehash() const noexcept {
        return (size_ + 1 + deleted_count_) > static_cast<size_type>(capacity_ * max_load_factor);
    }    /**
     * @brief 再ハッシュ時の新しい容量を計算します。
     * @return 新しい容量。
     */
    size_type calculate_new_capacity() const noexcept {
        return capacity_ > 0 ? capacity_ * 2 : initial_capacity;
    }

    /**
     * @brief 現在の要素数に基づいて最適な容量を計算します。
     * トゥームストーンを考慮せず、実際の要素数のみを基準にします。
     * @return 最適な容量。
     */
    size_type calculate_optimal_capacity() const noexcept {
        // 目標ロードファクターを0.75として、必要な最小容量を計算
        constexpr double target_load_factor = 0.75;
        size_type min_capacity = static_cast<size_type>(size_ / target_load_factor) + 1;

        // 2の冪に最も近い値を選択（効率的なハッシュのため）
        size_type optimal_capacity = initial_capacity;
        while (optimal_capacity < min_capacity) {
            optimal_capacity *= 2;
        }

        // 現在の容量よりも小さくならないようにする（縮小を避ける）
        return optimal_capacity > capacity_ ? optimal_capacity : capacity_;
    }

    /**
     * @brief 型特性に基づいて要素を移動またはコピー
     */
    template<typename K, typename V>
    void transfer_element(K&& key, V&& value, size_type idx) noexcept {
        if constexpr (std::is_nothrow_move_constructible_v<key_type> &&
                     std::is_nothrow_move_constructible_v<mapped_type>) {
            buckets_[idx].kv.first = std::move(key);
            buckets_[idx].kv.second = std::move(value);
        } else {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = value;
        }
        buckets_[idx].set_used(true);
        buckets_[idx].set_deleted(false);
    }

    /**
     * @brief ハッシュマップを指定された新しい容量に再ハッシュします。
     */
    void rehash(size_type new_capacity) noexcept {
        bucket* old_buckets = buckets_;
        size_type old_capacity = capacity_;
        size_type old_size = size_;

        allocate_buckets(new_capacity);
        size_ = 0;
        deleted_count_ = 0;
        capacity_ = new_capacity;

        for (size_type i = 0; i < old_capacity; ++i) {
            if (old_buckets[i].is_used() && !old_buckets[i].is_deleted()) {
                size_type idx = insert_index(old_buckets[i].kv.first);
                if (idx != npos) {
                    transfer_element(
                        std::move(old_buckets[i].kv.first),
                        std::move(old_buckets[i].kv.second),
                        idx
                    );
                    ++size_;
                }
            }
        }

        BLUESTL_ASSERT(size_ == old_size && "Element count mismatch after rehash");        for (size_type i = 0; i < old_capacity; ++i) {
            if (old_buckets[i].is_used() && !old_buckets[i].is_deleted()) {
                old_buckets[i].kv.~pair();
            }
        }

        using bucket_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<bucket>;
        bucket_allocator bucket_alloc(allocator_);
        bucket_alloc.deallocate(old_buckets, old_capacity);
    }

    /**
     * @brief 二次プロービング用の補助関数。
     */
    constexpr size_type probe_next(size_type base_idx, size_type probe_count) const noexcept {
        size_type step = (probe_count * probe_count + probe_count) >> 1;
        return (base_idx + step) % capacity_;
    }

    /**
     * @brief キー検索用の最適化されたプロービング処理
     */
    size_type find_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos;

        const size_type hash_val = bluestl::hash(key) % capacity_;
        size_type probe_count = 0;

        while (probe_count < capacity_) {
            size_type idx = probe_next(hash_val, probe_count);

            if (!buckets_[idx].is_used()) {
                return npos;
            }

            if (!buckets_[idx].is_deleted() && buckets_[idx].kv.first == key) {
                return idx;
            }

            ++probe_count;
        }
        return npos;
    }

    /**
     * @brief 新しいキーを挿入するための適切なバケットインデックスを検索します。
     */
    size_type insert_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos;

        size_type hash_val = bluestl::hash(key) % capacity_;
        size_type first_deleted = npos;

        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = probe_next(hash_val, i);

            if (!buckets_[idx].is_used()) {
                return (first_deleted != npos) ? first_deleted : idx;
            }

            if (buckets_[idx].is_deleted()) {
                if (first_deleted == npos) {
                    first_deleted = idx;
                }
            } else if (buckets_[idx].kv.first == key) {
                return idx;
            }
        }

        return first_deleted;
    }

    /**
     * @brief at() などでキーが見つからなかった場合に返すダミーの値への参照。
     */
    static mapped_type& dummy_value() noexcept {
        static mapped_type dummy{};
        return dummy;
    }

    /**
     * @brief const版のダミー値参照を返す関数
     */
    static const mapped_type& const_dummy_value() noexcept {
        static const mapped_type dummy{};
        return dummy;
    }
};

} // namespace bluestl
