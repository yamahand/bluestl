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

template <typename Key, typename T, typename Allocator>
class hash_map {
    // Bucket structure to store key-value pairs
    struct bucket {
        pair<Key, T> kv;
        bool used = false;
        bool deleted = false;  // For tombstone marking
    };

   public:
    // Forward declaration of iterators
    template <bool IsConst>
    class hash_iterator;

   public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = pair<const Key, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = Allocator;
    using iterator = hash_iterator<false>;
    using const_iterator = hash_iterator<true>;

    // Constants
    static constexpr size_type initial_capacity = 16;
    static constexpr size_type npos = static_cast<size_type>(-1);
    static constexpr float max_load_factor = 0.75f;

    // Iterator implementation
    template <bool IsConst>
    class hash_iterator {
       public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::conditional_t<IsConst, const pair<const Key, T>, pair<const Key, T>>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        // Default constructor for end iterator
        hash_iterator() noexcept : container_(nullptr), index_(0) {}

        // Constructor
        hash_iterator(const hash_map* container, size_type index) noexcept : container_(container), index_(index) {
            advance_to_valid();
        }

        // Copy constructor
        hash_iterator(const hash_iterator& other) noexcept : container_(other.container_), index_(other.index_) {}

        // Assignment operator
        hash_iterator& operator=(const hash_iterator& other) noexcept {
            container_ = other.container_;
            index_ = other.index_;
            return *this;
        }

        // Equality operators
        bool operator==(const hash_iterator& other) const noexcept {
            return container_ == other.container_ && index_ == other.index_;
        }

        bool operator!=(const hash_iterator& other) const noexcept {
            return !(*this == other);
        }

        // Dereference operator
        reference operator*() const noexcept {
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].used &&
                           !container_->buckets_[index_].deleted);
            return *reinterpret_cast<pointer>(&container_->buckets_[index_].kv);
        }

        // Arrow operator
        pointer operator->() const noexcept {
            BLUESTL_ASSERT(container_ && index_ < container_->capacity_ && container_->buckets_[index_].used &&
                           !container_->buckets_[index_].deleted);
            return reinterpret_cast<pointer>(&container_->buckets_[index_].kv);
        }

        // Pre-increment
        hash_iterator& operator++() noexcept {
            if (container_ && index_ < container_->capacity_) {
                ++index_;
                advance_to_valid();
            }
            return *this;
        }

        // Post-increment
        hash_iterator operator++(int) noexcept {
            hash_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // インデックスへのアクセスを提供するメソッドを追加
        size_type get_index() const noexcept {
            return index_;
        }

       private:
        const hash_map* container_;
        size_type index_;

        // Find next valid bucket
        void advance_to_valid() noexcept {
            if (!container_) return;

            while (index_ < container_->capacity_ &&
                   (!container_->buckets_[index_].used || container_->buckets_[index_].deleted)) {
                ++index_;
            }

            if (index_ >= container_->capacity_) {
                // Reached the end
                index_ = container_->capacity_;
            }
        }
    };

    // Constructor with external allocator
    hash_map(Allocator& alloc) noexcept
        : size_(0), deleted_count_(0), capacity_(initial_capacity), allocator_(alloc), buckets_(nullptr) {
        allocate_buckets(capacity_);
    }

    // Destructor
    ~hash_map() noexcept {
        if (buckets_) {
            destroy_buckets();
            allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
        }
    }

    // Copy constructor
    hash_map(const hash_map& other) noexcept
        : size_(0), deleted_count_(0), capacity_(other.capacity_), allocator_(other.allocator_), buckets_(nullptr) {
        allocate_buckets(capacity_);
        for (size_type i = 0; i < other.capacity_; ++i) {
            if (other.buckets_[i].used && !other.buckets_[i].deleted) {
                insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
            }
        }
    }

    // Copy assignment operator
    hash_map& operator=(const hash_map& other) noexcept {
        if (this != &other) {
            clear();
            if (capacity_ < other.capacity_) {
                if (buckets_) {
                    allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
                    buckets_ = nullptr;
                }
                capacity_ = other.capacity_;
                allocate_buckets(capacity_);
            }

            for (size_type i = 0; i < other.capacity_; ++i) {
                if (other.buckets_[i].used && !other.buckets_[i].deleted) {
                    insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
                }
            }
        }
        return *this;
    }

    // Move constructor
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

    // Move assignment operator
    hash_map& operator=(hash_map&& other) noexcept {
        if (this != &other) {
            if (buckets_) {
                destroy_buckets();
                allocator_.deallocate(buckets_, capacity_ * sizeof(bucket));
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

    // Capacity related methods
    size_type size() const noexcept {
        return size_;
    }
    size_type capacity() const noexcept {
        return capacity_;
    }
    bool empty() const noexcept {
        return size_ == 0;
    }

    // Clear the hash map
    void clear() noexcept {
        for (size_type i = 0; i < capacity_; ++i) {
            buckets_[i].used = false;
            buckets_[i].deleted = false;
        }
        size_ = 0;
        deleted_count_ = 0;
    }

    // Iterator methods
    iterator begin() noexcept {
        return iterator(this, 0);
    }
    const_iterator begin() const noexcept {
        return const_iterator(this, 0);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(this, 0);
    }

    iterator end() noexcept {
        return iterator(this, capacity_);
    }
    const_iterator end() const noexcept {
        return const_iterator(this, capacity_);
    }
    const_iterator cend() const noexcept {
        return const_iterator(this, capacity_);
    }

    // Access operators
    mapped_type& operator[](const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) return buckets_[idx].kv.second;

        // Need to insert new element
        if (should_rehash()) rehash(calculate_new_capacity());

        idx = insert_index(key);
        if (idx != npos) {
            // Initialize with default constructor
            buckets_[idx].kv.first = key;
            buckets_[idx].kv.second = mapped_type();
            buckets_[idx].used = true;
            buckets_[idx].deleted = false;
            ++size_;
            return buckets_[idx].kv.second;
        }

        // This should never happen if the hash map is properly rehashed
        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return dummy_value();
    }

    // Access with bounds checking (returns dummy for not found)
    mapped_type& at(const key_type& key) noexcept {
        size_type idx = find_index(key);
        BLUESTL_ASSERT(idx != npos && "hash_map::at key does not exist");
        if (idx == npos) return dummy_value();
        return buckets_[idx].kv.second;
    }

    const mapped_type& at(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        BLUESTL_ASSERT(idx != npos && "hash_map::at key does not exist");
        if (idx == npos) return dummy_value();
        return buckets_[idx].kv.second;
    }

    /**
     * @brief キーに対応する値への参照をoptionalで返す。見つからなければ値なし。
     * @param key 検索するキー
     * @return optional<mapped_type&> 値が存在すれば参照、なければ値なし
     */
    optional<mapped_type&> try_get(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return optional<mapped_type&>(buckets_[idx].kv.second);
        }
        return optional<mapped_type&>();
    }

    /**
     * @brief キーに対応する値へのconst参照をoptionalで返す。見つからなければ値なし。
     * @param key 検索するキー
     * @return optional<const mapped_type&> 値が存在すればconst参照、なければ値なし
     */
    optional<const mapped_type&> try_get(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        if (idx != npos) {
            return optional<const mapped_type&>(buckets_[idx].kv.second);
        }
        return optional<const mapped_type&>();
    }

    // Insert key-value pair
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
            buckets_[idx].deleted = false;
            ++size_;
            return { iterator(this, idx), true };
        }

        // Should never happen if rehashing works correctly
        BLUESTL_ASSERT(false && "Hash map insertion failed");
        return { end(), false };
    }

    // Erase element by key
    bool erase(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return false;

        // Mark as deleted (tombstone)
        buckets_[idx].deleted = true;
        --size_;
        ++deleted_count_;

        // Consider rehashing if too many tombstones
        if (deleted_count_ > size_) rehash(capacity_);

        return true;
    }

    // Erase element by iterator
    iterator erase(iterator pos) noexcept {
        BLUESTL_ASSERT(pos != end() && "Cannot erase end iterator");

        size_type idx = pos.get_index();
        iterator next(this, idx);
        ++next;

        if (idx < capacity_ && buckets_[idx].used && !buckets_[idx].deleted) {
            buckets_[idx].deleted = true;
            --size_;
            ++deleted_count_;

            // Consider rehashing if too many tombstones
            if (deleted_count_ > size_) rehash(capacity_);
        }

        return next;
    }

    // Erase element by iterator
    iterator erase(const_iterator pos) noexcept {
        BLUESTL_ASSERT(pos != end() && "Cannot erase end iterator");

        size_type idx = pos.get_index();
        iterator next(this, idx);
        ++next;

        if (idx < capacity_ && buckets_[idx].used && !buckets_[idx].deleted) {
            buckets_[idx].deleted = true;
            --size_;
            ++deleted_count_;

            // Consider rehashing if too many tombstones
            if (deleted_count_ > size_) rehash(capacity_);
        }

        return next;
    }

    // Find element - returns iterator
    iterator find(const key_type& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end();
        return iterator(this, idx);
    }

    const_iterator find(const key_type& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return end();
        return const_iterator(this, idx);
    }

    // Check if key exists
    bool contains(const key_type& key) const noexcept {
        return find_index(key) != npos;
    }

   private:
    bucket* buckets_;
    size_type size_;
    size_type deleted_count_;
    size_type capacity_;
    Allocator& allocator_;

    // Make hash_iterator a friend to access private members
    template <bool IsConst>
    friend class hash_iterator;

    // Allocate and initialize buckets
    void allocate_buckets(size_type n) noexcept {
        buckets_ = static_cast<bucket*>(allocator_.allocate(n * sizeof(bucket)));
        for (size_type i = 0; i < n; ++i) {
            // placement new で bucket オブジェクトを構築
            new (&buckets_[i]) bucket();
        }
    }

    // Destroy all buckets
    void destroy_buckets() noexcept {
        for (size_type i = 0; i < capacity_; ++i) {
            if (buckets_[i].used && !buckets_[i].deleted) {
                // Explicitly call destructors if needed
                buckets_[i].kv.~pair();
            }
        }
    }

    // Determine if rehashing is needed
    bool should_rehash() const noexcept {
        return (size_ + 1 + deleted_count_) > static_cast<size_type>(capacity_ * max_load_factor);
    }

    // Calculate new capacity for rehashing
    size_type calculate_new_capacity() const noexcept {
        return capacity_ * 2;
    }

    // Rehash the map to a new capacity
    void rehash(size_type new_capacity) noexcept {
        bucket* old_buckets = buckets_;
        size_type old_capacity = capacity_;

        // Allocate new buckets
        allocate_buckets(new_capacity);
        size_type old_size = size_;
        size_ = 0;
        deleted_count_ = 0;
        capacity_ = new_capacity;

        // Move elements to new buckets
        for (size_type i = 0; i < old_capacity; ++i) {
            if (old_buckets[i].used && !old_buckets[i].deleted) {
                insert(old_buckets[i].kv.first, old_buckets[i].kv.second);
            }
        }

        // Ensure all elements were transferred
        BLUESTL_ASSERT(size_ == old_size && "Some elements were lost during rehash");

        // Deallocate old buckets
        allocator_.deallocate(old_buckets, old_capacity * sizeof(bucket));
    }

    // Find bucket index for a key
    size_type find_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos;

        size_type hash_val = bluestl::hash(key) % capacity_;
        size_type first_deleted = npos;

        // Quadratic probing: idx = (hash + i*i) % capacity
        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = (hash_val + i * i) % capacity_;

            if (!buckets_[idx].used) {
                // Found an empty slot, key doesn't exist
                return npos;
            }

            if (buckets_[idx].deleted) {
                // Keep track of first deleted bucket for possible insertion
                if (first_deleted == npos) {
                    first_deleted = idx;
                }
                continue;
            }

            if (buckets_[idx].kv.first == key) {
                // Found the key
                return idx;
            }
        }

        // If we've checked all slots and found none, the key doesn't exist
        return npos;
    }

    // Find an index to insert a new key
    size_type insert_index(const key_type& key) const noexcept {
        if (capacity_ == 0) return npos;

        size_type hash_val = bluestl::hash(key) % capacity_;
        size_type first_deleted = npos;

        // Quadratic probing: idx = (hash + i*i) % capacity
        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = (hash_val + i * i) % capacity_;

            if (!buckets_[idx].used) {
                // Empty slot - use this
                return (first_deleted != npos) ? first_deleted : idx;
            }

            if (buckets_[idx].deleted) {
                // Keep track of first deleted bucket
                if (first_deleted == npos) {
                    first_deleted = idx;
                }
            } else if (buckets_[idx].kv.first == key) {
                // Key already exists - return its index
                return idx;
            }
        }

        // Return first deleted slot if available
        return first_deleted;
    }

    // Static dummy value for safety
    static mapped_type& dummy_value() noexcept {
        static mapped_type dummy{};
        return dummy;
    }
};

}  // namespace bluestl