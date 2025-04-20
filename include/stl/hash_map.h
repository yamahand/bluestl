#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include "hash.h"
#include "allocator.h"

namespace bluestl {

template <typename Key, typename T, typename Allocator = allocator>
class hash_map {
    struct bucket {
        std::pair<Key, T> kv;
        bool used = false;
    };
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using allocator_type = Allocator;
    static constexpr size_type initial_capacity = 16; 

    hash_map(Allocator& alloc) noexcept
        : size_(0), capacity_(initial_capacity), allocator_(alloc), buckets_(nullptr) {
        allocate_buckets(capacity_);
    }

    ~hash_map() noexcept {
        clear();
        if (buckets_) allocator_.deallocate(buckets_, capacity_);
    }

    hash_map(const hash_map& other) noexcept : size_(0), capacity_(other.capacity_), allocator_(other.allocator_), buckets_(nullptr) {
        allocate_buckets(capacity_);
        for (size_type i = 0; i < other.capacity_; ++i) {
            if (other.buckets_[i].used) insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
        }
    }
    hash_map& operator=(const hash_map& other) noexcept {
        if (this != &other) {
            clear();
            if (capacity_ < other.capacity_) {
                if (buckets_) allocator_.deallocate(buckets_, capacity_);
                capacity_ = other.capacity_;
                allocate_buckets(capacity_);
            }
            for (size_type i = 0; i < other.capacity_; ++i) {
                if (other.buckets_[i].used) insert(other.buckets_[i].kv.first, other.buckets_[i].kv.second);
            }
        }
        return *this;
    }

    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type capacity() const noexcept { return capacity_; }
    constexpr bool empty() const noexcept { return size_ == 0; }

    void clear() noexcept {
        for (size_type i = 0; i < capacity_; ++i) buckets_[i].used = false;
        size_ = 0;
    }

    T& operator[](const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx != npos) return buckets_[idx].kv.second;
        return insert(key, T{}).first->second;
    }

    T& at(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return dummy();
        return buckets_[idx].kv.second;
    }

    std::pair<value_type*, bool> insert(const Key& key, const T& value) noexcept {
        if ((size_ + 1) * 2 > capacity_) rehash(capacity_ * 2);
        size_type hash = bluestl::hash(key) % capacity_;
        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = (hash + i) % capacity_;
            if (!buckets_[idx].used) {
                buckets_[idx].kv = std::pair<Key, T>(key, value);
                buckets_[idx].used = true;
                ++size_;
                return { to_value_type(&buckets_[idx]), true };
            }
            if (buckets_[idx].used && buckets_[idx].kv.first == key) {
                return { to_value_type(&buckets_[idx]), false };
            }
        }
        return { nullptr, false };
    }

    bool erase(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return false;
        buckets_[idx].used = false;
        --size_;
        return true;
    }

    // findやAPIで返すときはconst Keyに変換
    value_type* to_value_type(bucket* b) const noexcept {
        return reinterpret_cast<value_type*>(&b->kv);
    }

    // findの返り値などでto_value_typeを使う
    value_type* find(const Key& key) noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return nullptr;
        return to_value_type(&buckets_[idx]);
    }
    const value_type* find(const Key& key) const noexcept {
        size_type idx = find_index(key);
        if (idx == npos) return nullptr;
        return to_value_type(&buckets_[idx]);
    }

    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    bucket* buckets_;
    size_type size_;
    size_type capacity_;
    Allocator& allocator_;

    void allocate_buckets(size_type n) noexcept {
        buckets_ = static_cast<bucket*>(allocator_.allocate(n));
        for (size_type i = 0; i < n; ++i) buckets_[i].used = false;
    }

    void rehash(size_type new_capacity) noexcept {
        bucket* old_buckets = buckets_;
        size_type old_capacity = capacity_;
        allocate_buckets(new_capacity);
        size_ = 0;
        capacity_ = new_capacity;
        for (size_type i = 0; i < old_capacity; ++i) {
            if (old_buckets[i].used) insert(old_buckets[i].kv.first, old_buckets[i].kv.second);
        }
        allocator_.deallocate(old_buckets, old_capacity);
    }

    size_type find_index(const Key& key) const noexcept {
        size_type hash = bluestl::hash(key) % capacity_;
        for (size_type i = 0; i < capacity_; ++i) {
            size_type idx = (hash + i) % capacity_;
            if (!buckets_[idx].used) return npos;
            if (buckets_[idx].kv.first == key) return idx;
        }
        return npos;
    }
    static T& dummy() {
        static T d{};
        return d;
    }
};

} // namespace bluestl
