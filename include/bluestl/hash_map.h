\
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
#include <iterator>

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
    /**
     * @brief バケット構造体。キーと値のペア、使用状態、削除状態を保持します。
     */
    struct bucket {
        pair<Key, T> kv;      ///< キーと値のペア
        bool used = false;    ///< このバケットが使用中かどうか
        bool deleted = false; ///< このバケットが削除済み（トゥームストーン）かどうか
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
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].used &&
                           !container_->buckets_[index_].deleted);
            // const Key を持つ pair への参照を返すため reinterpret_cast を使用
            return *reinterpret_cast<pointer>(&container_->buckets_[index_].kv);
        }

        /**
         * @brief アロー演算子。
         * @return イテレータが指す要素のメンバへのポインタ。
         * @pre イテレータは有効な要素を指している必要があります。
         */
        pointer operator->() const noexcept {
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].used &&
                           !container_->buckets_[index_].deleted);
            // const Key を持つ pair へのポインタを返すため reinterpret_cast を使用
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

            while (index_ < container_->capacity_ &&
                   (!container_->buckets_[index_].used || container_->buckets_[index_].deleted)) {
                ++index_;
            }

            // 有効な要素が見つからず、末尾に達した場合
            if (index_ >= container_->capacity_) {
                index_ = container_->capacity_; // end() と同じ状態にする
            }
        }
    };

    /**
     * @brief コンストラクタ。外部アロケータを指定します。
     * @param alloc 使用するアロケータへの参照。hash_map の生存期間中、有効である必要があります。
     */
    hash_map(Allocator& alloc) noexcept
        : size_(0), deleted_count_(0), capacity_(initial_capacity), allocator_(alloc), buckets_(nullptr) {
        allocate_buckets(capacity_);
    }

    /**
     * @brief デストラクタ。要素のデストラクタを呼び出し、確保したメモリを解放します。
     */
    ~hash_map() noexcept {
        if (buckets_) {
            destroy_buckets(); // 要素のデストラクタ呼び出し
            allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
        }
    }

    /**
     * @brief コピーコンストラクタ。
     * @param other コピー元の hash_map オブジェクト。
     */
    hash_map(const hash_map& other) noexcept
        : size_(0), deleted_count_(0), capacity_(other.capacity_), allocator_(other.allocator_), buckets_(nullptr) {
        allocate_buckets(capacity_);
        // 有効な要素のみをコピー
        for (size_type i = 0; i < other.capacity_; ++i) {
            if (other.buckets_[i].used && !other.buckets_[i].deleted) {
                // insert を使うことで、新しいバケット配列での正しい位置に配置される
                insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
            }
        }
    }

    /**
     * @brief コピー代入演算子。
     * @param other コピー元の hash_map オブジェクト。
     * @return *this への参照。
     */
    hash_map& operator=(const hash_map& other) noexcept {
        if (this != &other) {
            clear(); // 現在の内容をクリア
            // 必要であれば容量を拡張
            if (capacity_ < other.capacity_) {
                if (buckets_) {
                    // 要素のデストラクタは clear() で呼ばれているはずだが念のため
                    // destroy_buckets(); // 不要？ clear() で used=false になっている
                    allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
                    buckets_ = nullptr;
                }
                capacity_ = other.capacity_;
                allocate_buckets(capacity_);
            }

            // 有効な要素のみをコピー
            for (size_type i = 0; i < other.capacity_; ++i) {
                if (other.buckets_[i].used && !other.buckets_[i].deleted) {
                    insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
                }
            }
            // アロケータはコピーしない（参照なので）
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
          allocator_(other.allocator_), // アロケータ参照はコピー
          buckets_(other.buckets_) {
        // ムーブ元のリソースを解放
        other.size_ = 0;
        other.deleted_count_ = 0;
        other.capacity_ = 0;
        other.buckets_ = nullptr;
    }

    /**
     * @brief ムーブ代入演算子。
     * @param other ムーブ元の hash_map オブジェクト。ムーブ後は空の状態になります。
     * @return *this への参照。
     */
    hash_map& operator=(hash_map&& other) noexcept {
        if (this != &other) {
            // 既存のリソースを解放
            if (buckets_) {
                destroy_buckets();
                allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
            }

            // リソースをムーブ
            size_ = other.size_;
            deleted_count_ = other.deleted_count_;
            capacity_ = other.capacity_;
            buckets_ = other.buckets_;
            // アロケータ参照はコピー (ムーブ元のアロケータを引き続き使う)
            // allocator_ = other.allocator_; // 参照なので代入不要

            // ムーブ元のリソースを解放
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
            if (buckets_[i].used && !buckets_[i].deleted) {
                // 要素のデストラクタを呼び出す
                buckets_[i].kv.~pair(); // Key と T のデストラクタが呼ばれる
            }
            buckets_[i].used = false;
            buckets_[i].deleted = false;
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
        // capacity_ をインデックスとして渡すことで、advance_to_valid が end 状態にする
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
        if (idx != npos) return buckets_[idx].kv.second; // 既存の要素を返す

        // 新しい要素を挿入する必要がある
        if (should_rehash()) rehash(calculate_new_capacity());

        idx = insert_index(key); // 挿入位置を探す（トゥームストーンも考慮）
        if (idx != npos) {
            // 新しい要素を構築 (placement new は不要、代入でOK)
            buckets_[idx].kv.first = key; // キーをコピー
            buckets_[idx].kv.second = mapped_type();
            buckets_[idx].used = true;
            buckets_[idx].deleted = false; // 挿入なので削除済みではない
            ++size_;
            return buckets_[idx].kv.second;
        }

        // ここに到達するのは、rehash が適切に行われていればありえないはず
        BLUESTL_ASSERT(false && "Hash map insertion failed after rehash check");
        return dummy_value(); // アサーション失敗時のフォールバック
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
        size_type idx = find_index(key); // find_index は const& を取るのでムーブされない
        if (idx != npos) return buckets_[idx].kv.second; // 既存の要素を返す

        // 新しい要素を挿入する必要がある
        if (should_rehash()) rehash(calculate_new_capacity());

        // insert_index も const& を取るのでムーブされない
        idx = insert_index(key);
        if (idx != npos) {
            // 新しい要素を構築
            buckets_[idx].kv.first = std::move(key); // キーをムーブ
            // 値をデフォルト構築
            buckets_[idx].kv.second = mapped_type();
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
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
        if (idx == npos) return dummy_value(); // リリースビルドでのフォールバック
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
        if (idx == npos) return dummy_value(); // リリースビルドでのフォールバック
        // const 版なので dummy_value() は const& を返すべきだが、現状は非 const
        // TODO: const 版の dummy_value を用意するか、設計を見直す
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
        return optional<mapped_type&>(); // 値なし optional
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
        return optional<const mapped_type&>(); // 値なし optional
    }

    /**
     * @brief キーと値のペアを挿入します。
     *        キーが既に存在する場合は挿入せず、既存の要素を指すイテレータと false を返します。
     * @param key 挿入するキー。
     * @param value 挿入する値。
     * @return pair<iterator, bool> 挿入された要素を指すイテレータと、挿入が成功したか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     */
    pair<iterator, bool> insert(const key_type& key, const mapped_type& value) noexcept {
        // Check if we need to rehash
        if (should_rehash()) rehash(calculate_new_capacity());

        // Check if key already exists
        size_type idx = find_index(key);
        if (idx != npos) {
            return { iterator(this, idx), false };  // Key already exists
        }

        // Find a spot to insert
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = value;
            buckets_[idx].used = true;
            buckets_[idx].deleted = false; // 新規挿入なので削除済みではない
            ++size_;
            return { iterator(this, idx), true }; // 挿入成功
        }

        // Should never happen if rehashing works correctly
        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return { end(), false }; // 失敗を示す
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

        size_type idx = find_index(key); // find_index は const& を取る
        if (idx != npos) {
            return { iterator(this, idx), false };
        }

        idx = insert_index(key); // insert_index は const& を取る
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);   // キーをムーブ
            buckets_[idx].kv.second = std::move(value); // 値をムーブ
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
            ++size_;
            return { iterator(this, idx), true };
        }

        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return { end(), false };
    }

    /**
     * @brief 指定された引数で値を構築し、要素を挿入します。
     *        キーが既に存在する場合、挿入は行われません（try_emplace と同じ動作）。
     * @tparam Args 値 (mapped_type) のコンストラクタに渡される引数の型。
     * @param key 挿入するキー。
     * @param args 値を構築するための引数。完全転送されます。
     * @return pair<iterator, bool> 挿入された要素または既存の要素を指すイテレータと、
     *         挿入が行われたか (true) / キーが既に存在したか (false) を示す bool 値のペア。
     * @note std::unordered_map::emplace とは異なり、キーが既存の場合に引数から値を構築しません。
     *       この動作は try_emplace と同じです。
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
     * @note std::unordered_map::emplace とは異なり、キーが既存の場合に引数から値を構築しません。
     *       この動作は try_emplace と同じです。
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
            // キーが既に存在する場合、何もしない
            return { iterator(this, idx), false };
        }
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            // placement new を使用して、指定された引数で値を直接構築
            new (&buckets_[idx].kv.second) mapped_type(std::forward<Args>(args)...);
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
            ++size_;
            return { iterator(this, idx), true }; // 挿入成功
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
        size_type idx = find_index(key); // find_index は const& を取る
        if (idx != npos) {
            // キーが既に存在する場合、何もしない
            return { iterator(this, idx), false };
        }
        idx = insert_index(key); // insert_index は const& を取る
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key); // キーをムーブ
            // placement new を使用して、指定された引数で値を直接構築
            new (&buckets_[idx].kv.second) mapped_type(std::forward<Args>(args)...);
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
            ++size_;
            return { iterator(this, idx), true }; // 挿入成功
        }
        BLUESTL_ASSERT(false && "Hash map try_emplace failed");
        return { end(), false };
    }

    /**
     * @brief 指定されたキーに対応する要素を削除します。
     * @param key 削除する要素のキー。
     * @return 要素が削除された場合は true、キーが見つからなかった場合は false。
     */
    bool erase(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return false; // キーが見つからない

        // 要素を削除済みとしてマーク（トゥームストーン）
        buckets_[idx].deleted = true;
        // 要素のデストラクタはここでは呼ばない（再ハッシュ時または clear/デストラクタで処理）
        --size_;
        ++deleted_count_;

        // トゥームストーンが多すぎる場合は、再ハッシュを検討
        // （単純化のため、削除要素数が有効要素数を超えたらリハッシュ）
        if (deleted_count_ > size_) rehash(capacity_); // 現在の容量でリハッシュ（トゥームストーン除去）

        return true;
    }

    /**
     * @brief 指定されたイテレータが指す要素を削除します。
     * @param pos 削除する要素を指すイテレータ。end() であってはなりません。
     * @return 削除された要素の次の要素を指すイテレータ。
     * @pre pos は有効な要素を指しており、end() ではありません。
     */
    iterator erase(iterator pos) noexcept {
        BLUESTL_ASSERT(pos != end() && "Cannot erase end iterator");
        if (pos == end()) return end(); // アサーション無効時のガード

        size_type idx = pos.get_index(); // イテレータからインデックスを取得
        iterator next_it(this, idx); // 次のイテレータを準備
        ++next_it; // 先にインクリメントしておく

        if (idx < capacity_ && buckets_[idx].used && !buckets_[idx].deleted) {
            buckets_[idx].deleted = true; // トゥームストーンを設定
            // デストラクタは呼ばない
            --size_;
            ++deleted_count_;

            // トゥームストーンが多すぎる場合はリハッシュ
            if (deleted_count_ > size_) rehash(capacity_);
            // リハッシュが発生した場合、next_it は無効になる可能性があるが、
            // C++標準の erase と同様に、削除後の次の要素を指すイテレータを返す仕様とする。
            // リハッシュ後の正しい次の要素を見つけるのは複雑なため、
            // ここではリハッシュ前の次の要素（の可能性がある位置）を指すイテレータを返す。
            // より厳密にするには、リハッシュ後にキーで再検索する必要がある。
        }

        return next_it; // 削除前の次の要素を指すイテレータを返す
    }

    /**
     * @brief 指定された const イテレータが指す要素を削除します。
     * @param pos 削除する要素を指す const イテレータ。cend() であってはなりません。
     * @return 削除された要素の次の要素を指すイテレータ。
     * @pre pos は有効な要素を指しており、cend() ではありません。
     */
    iterator erase(const_iterator pos) noexcept {
        BLUESTL_ASSERT(pos != cend() && "Cannot erase end iterator");
        if (pos == cend()) return end(); // アサーション無効時のガード

        size_type idx = pos.get_index();
        iterator next_it(this, idx); // 非 const の next イテレータ
        ++next_it;

        if (idx < capacity_ && buckets_[idx].used && !buckets_[idx].deleted) {
            buckets_[idx].deleted = true;
            --size_;
            ++deleted_count_;
            if (deleted_count_ > size_) rehash(capacity_);
        }

        return next_it;
    }

    /**
     * @brief 指定されたキーを持つ要素を検索します。
     * @param key 検索するキー。
     * @return キーが見つかった場合はその要素を指すイテレータ、見つからなかった場合は end()。
     */
    iterator find(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end(); // 見つからない場合は end()
        return iterator(this, idx); // 見つかった要素を指すイテレータ
    }

    /**
     * @brief 指定されたキーを持つ要素を検索します (const 版)。
     * @param key 検索するキー。
     * @return キーが見つかった場合はその要素を指す const イテレータ、見つからなかった場合は cend()。
     */
    const_iterator find(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return cend(); // 見つからない場合は cend()
        return const_iterator(this, idx); // 見つかった要素を指す const イテレータ
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
            // キーが存在する場合、値を代入
            buckets_[idx].kv.second = value;
            return { iterator(this, idx), false }; // 代入を示す false
        }
        // キーが存在しない場合、挿入
        idx = insert_index(key);
        if (idx != npos) {
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = value;
            buckets_[idx].used = true;
            buckets_[idx].deleted = false; // 新規挿入なので削除済みではない
            ++size_;
            return { iterator(this, idx), true }; // 新規挿入を示す true
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
        size_type idx = find_index(key); // find_index は const& を取る
        if (idx != npos) {
            // キーが存在する場合、値をムーブ代入
            buckets_[idx].kv.second = std::move(value);
            return { iterator(this, idx), false }; // 代入を示す false
        }
        // キーが存在しない場合、挿入
        idx = insert_index(key); // insert_index は const& を取る
        if (idx != npos) {
            buckets_[idx].kv.first = std::move(key);   // キーをムーブ
            buckets_[idx].kv.second = std::move(value); // 値をムーブ
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
            ++size_;
            return { iterator(this, idx), true }; // 新規挿入を示す true
        }
        BLUESTL_ASSERT(false && "Hash map insert_or_assign failed");
        return { end(), false };
    }

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
    friend class hash_iterator;

    /**
     * @brief 指定された容量 n でバケット配列を割り当て、各バケットをデフォルト構築します。
     * @param n 割り当てるバケットの数。
     */
    void allocate_buckets(size_type n) noexcept {
        // アロケータを使用してメモリを確保
        buckets_ = static_cast<bucket*>(allocator_.allocate(n * sizeof(bucket)));
        // placement new を使用して、確保したメモリ上に bucket オブジェクトを構築
        for (size_type i = 0; i < n; ++i) {
            new (&buckets_[i]) bucket(); // デフォルトコンストラクタを呼び出す
        }
    }

    /**
     * @brief バケット配列内の有効な要素（使用中で削除されていない）のデストラクタを呼び出します。
     * @note メモリの解放は行いません。
     */
    void destroy_buckets() noexcept {
        for (size_type i = 0; i < capacity_; ++i) {
            if (buckets_[i].used && !buckets_[i].deleted) {
                // pair<Key, T> のデストラクタを明示的に呼び出す
                buckets_[i].kv.~pair();
            }
            // used や deleted フラグは変更しない
        }
    }

    /**
     * @brief 再ハッシュが必要かどうかを判断します。
     *        負荷率（(要素数 + 削除済み要素数) / 容量）が最大負荷率を超える場合に true を返します。
     *        挿入前にチェックするため、要素数を +1 して評価します。
     * @return 再ハッシュが必要な場合は true、そうでない場合は false。
     */
    bool should_rehash() const noexcept {
        // 負荷率 = (有効要素数 + トゥームストーン数) / 容量
        // 次の挿入で負荷率が max_load_factor を超えるかチェック
        return (size_ + 1 + deleted_count_) > static_cast<size_type>(capacity_ * max_load_factor);
    }

    /**
     * @brief 再ハッシュ時の新しい容量を計算します。現在の容量の2倍を返します。
     * @return 新しい容量。
     */
    size_type calculate_new_capacity() const noexcept {
        // 単純に2倍する。0 の場合は initial_capacity になるように考慮が必要だが、
        // 最初の allocate_buckets で initial_capacity が設定されるため、capacity_ が 0 になることは通常ない。
        return capacity_ > 0 ? capacity_ * 2 : initial_capacity;
    }

    /**
     * @brief ハッシュマップを指定された新しい容量に再ハッシュします。
     *        新しいバケット配列を割り当て、既存の有効な要素を新しい配列に移動（再挿入）し、
     *        古いバケット配列を解放します。トゥームストーンはこの過程で除去されます。
     * @param new_capacity 新しいバケット配列の容量。
     */
    void rehash(size_type new_capacity) noexcept {
        bucket* old_buckets = buckets_;
        size_type old_capacity = capacity_;
        size_type old_size = size_; // 再ハッシュ前の有効要素数を保存

        // 新しい容量でバケットを確保
        allocate_buckets(new_capacity);
        // メンバ変数を更新
        size_ = 0; // 新しいバケットに挿入し直すのでリセット
        deleted_count_ = 0; // トゥームストーンはコピーしないのでリセット
        capacity_ = new_capacity;

        // 古いバケットから新しいバケットへ要素を移動（再挿入）
        for (size_type i = 0; i < old_capacity; ++i) {
            if (old_buckets[i].used && !old_buckets[i].deleted) {
                // insert を使用して新しいバケット配列に要素を挿入
                // これにより、新しいハッシュ値に基づいて正しい位置に配置される
                // ムーブではなくコピーで挿入する (Key, T がムーブ可能とは限らないため)
                // TODO: Key, T がムーブ可能ならムーブを検討
                insert(old_buckets[i].kv.first, old_buckets[i].kv.second);
            }
        }

        // 再ハッシュ後に要素数が一致するか検証
        BLUESTL_ASSERT(size_ == old_size && "Some elements were lost during rehash");

        // 古いバケットの要素のデストラクタを呼び出す (allocate_buckets で構築されたもの)
        // 注意: old_buckets の各要素のデストラクタを呼ぶ必要がある
        for (size_type i = 0; i < old_capacity; ++i) {
             if (old_buckets[i].used && !old_buckets[i].deleted) {
                 // insert でコピーされたので、元の要素のデストラクタを呼ぶ
                 old_buckets[i].kv.~pair();
             }
             // bucket 自体のデストラクタは trivial なので不要
        }


        // 古いバケット配列のメモリを解放
        allocator_.deallocate(old_buckets, old_capacity * sizeof(bucket));
    }

    /**
     * @brief 指定されたキーに対応するバケットのインデックスを検索します。
     *        二次プロービングを使用して衝突を解決します。
     * @param key 検索するキー。
     * @return キーが見つかった場合はそのバケットのインデックス、見つからなかった場合は npos。
     */
    size_type find_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos; // 容量が 0 の場合は検索不可

        size_type hash_val = bluestl::hash(key) % capacity_; // 初期ハッシュ位置
        size_type tombstone_idx = npos; // 最初に見つかったトゥームストーンの位置

        // 二次プロービング: idx = (hash + i*i) % capacity
        for (size_type i = 0; i < capacity_; ++i) { // 最大 capacity 回試行
            size_type idx = (hash_val + i * i) % capacity_;

            if (!buckets_[idx].used) {
                // 空のスロットが見つかった -> キーは存在しない
                // もし途中でトゥームストーンが見つかっていたとしても、その先にキーはない
                return npos;
            }

            if (buckets_[idx].deleted) {
                // トゥームストーンが見つかった
                // 検索は続行するが、挿入のために位置を覚えておく必要はない（find なので）
                continue; // 次のプローブ位置へ
            }

            // 使用中のバケットが見つかった
            if (buckets_[idx].kv.first == key) {
                // キーが一致した -> 見つかった
                return idx;
            }
        }

        // テーブルを一周しても見つからなかった（またはすべてトゥームストーンだった）
        return npos;
    }

    /**
     * @brief 新しいキーを挿入するための適切なバケットインデックスを検索します。
     *        空のスロット、または最初に見つかったトゥームストーンのスロットを返します。
     *        キーが既に存在する場合は、そのインデックスを返します（挿入は行われない）。
     *        二次プロービングを使用します。
     * @param key 挿入するキー。
     * @return 挿入に適したインデックス、またはキーが既に存在する場合はそのインデックス。
     *         テーブルが満杯（空きもトゥームストーンもない）の場合は npos を返す可能性があるが、
     *         通常は should_rehash() で事前にリハッシュされるため発生しないはず。
     */
    size_type insert_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos; // 容量 0 は異常系

        size_type hash_val = bluestl::hash(key) % capacity_;
        size_type first_deleted = npos; // 最初に見つかったトゥームストーンのインデックス

        // 二次プロービング
        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = (hash_val + i * i) % capacity_;

            if (!buckets_[idx].used) {
                // 空のスロットが見つかった
                // もしトゥームストーンが先に見つかっていればそちらを優先して返す
                // そうでなければ、この空きスロットを返す
                return (first_deleted != npos) ? first_deleted : idx;
            }

            if (buckets_[idx].deleted) {
                // トゥームストーンが見つかった
                // 最初に見つかったトゥームストーンの位置を記録しておく
                if (first_deleted == npos) {
                    first_deleted = idx;
                }
                // 挿入位置を探しているので、検索は続行
            } else if (buckets_[idx].kv.first == key) {
                // キーが既に存在する -> そのインデックスを返す (挿入は行われない)
                return idx;
            }
        }

        // テーブルを一周しても空きスロットが見つからなかった場合
        // （理論上、負荷率管理とリハッシュが正しければここには到達しないはず）
        // もしトゥームストーンが見つかっていれば、その位置を返す
        // （テーブルがトゥームストーンで埋まっている場合など）
        return first_deleted; // first_deleted が npos の場合、挿入不可を示す
    }

    /**
     * @brief at() などでキーが見つからなかった場合に返すダミーの値への参照。
     * @return mapped_type の静的なデフォルト構築オブジェクトへの参照。
     * @warning この関数が返す参照への書き込みは未定義動作を引き起こす可能性があります。
     *          主にアサーション無効時のフォールバックとして使用されます。
     *          const 版の at() のために const バージョンも必要になる可能性があります。
     */
    static mapped_type& dummy_value() noexcept {
        static mapped_type dummy{}; // デフォルト構築された静的オブジェクト
        return dummy;
    }
};

} // namespace bluestl
