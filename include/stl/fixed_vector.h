#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>
#include <concepts>
#include <iterator>
#include "stl/assert_handler.h"

namespace bluestl {

template <typename T, std::size_t Capacity>
class fixed_vector {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr fixed_vector() noexcept : size_(0) {}

    // デストラクタ
    ~fixed_vector() noexcept {
        clear();
    }

    constexpr bool push_back(const T& value) noexcept {
        if (size_ >= Capacity) return false;
        new (data() + size_) T(value);
        ++size_;
        return true;
    }

    constexpr void pop_back() noexcept {
        if (size_ > 0) {
            --size_;
            data()[size_].~T();
        }
    }

    constexpr void clear() noexcept {
        for (size_type i = 0; i < size_; ++i) {
            data()[i].~T();
        }
        size_ = 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }

    [[nodiscard]] constexpr T* data() noexcept { return reinterpret_cast<T*>(storage_); }
    [[nodiscard]] constexpr const T* data() const noexcept { return reinterpret_cast<const T*>(storage_); }

    [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }
    [[nodiscard]] constexpr iterator end() noexcept { return data() + size_; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + size_; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    [[nodiscard]] constexpr reference at(size_type pos) noexcept {
        BLUESTL_ASSERT(pos < size_);
        return data()[pos];
    }
    [[nodiscard]] constexpr const_reference at(size_type pos) const noexcept {
        BLUESTL_ASSERT(pos < size_);
        return data()[pos];
    }
    // operator[]
    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept {
        return data()[pos];
    }
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept {
        return data()[pos];
    }
    // front/back/empty
    [[nodiscard]] constexpr reference front() noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return data()[0];
    }
    [[nodiscard]] constexpr const_reference front() const noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return data()[0];
    }
    [[nodiscard]] constexpr reference back() noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return data()[size_ - 1];
    }
    [[nodiscard]] constexpr const_reference back() const noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return data()[size_ - 1];
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    // ムーブ対応 push_back
    constexpr bool push_back(T&& value) noexcept {
        if (size_ >= Capacity) return false;
        new (data() + size_) T(std::move(value));
        ++size_;
        return true;
    }

    // emplace_back
    template <class... Args>
    constexpr bool emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
        if (size_ >= Capacity) return false;
        new (data() + size_) T(std::forward<Args>(args)...);
        ++size_;
        return true;
    }

    // assign
    constexpr void assign(size_type count, const T& value) noexcept {
        clear();
        for (size_type i = 0; i < count && i < Capacity; ++i) {
            push_back(value);
        }
    }

    // insert（単一要素）
    constexpr iterator insert(iterator pos, const T& value) noexcept {
        size_type idx = pos - data();
        if (size_ >= Capacity || idx > size_) return end();
        for (size_type i = size_; i > idx; --i) {
            new (data() + i) T(std::move(data()[i - 1]));
            data()[i - 1].~T();
        }
        new (data() + idx) T(value);
        ++size_;
        return data() + idx;
    }

    // erase（単一要素）
    constexpr iterator erase(iterator pos) noexcept {
        size_type idx = pos - data();
        if (idx >= size_) return end();
        data()[idx].~T();
        for (size_type i = idx; i < size_ - 1; ++i) {
            new (data() + i) T(std::move(data()[i + 1]));
            data()[i + 1].~T();
        }
        --size_;
        return data() + idx;
    }

    // swap
    constexpr void swap(fixed_vector& other) noexcept {
        for (size_type i = 0; i < Capacity && i < other.size_; ++i) {
            std::swap(data()[i], other.data()[i]);
        }
        std::swap(size_, other.size_);
    }

    // 比較演算子
    friend constexpr bool operator==(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        if (lhs.size_ != rhs.size_) return false;
        for (size_type i = 0; i < lhs.size_; ++i) {
            if (!(lhs.data()[i] == rhs.data()[i])) return false;
        }
        return true;
    }
    friend constexpr bool operator!=(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend constexpr bool operator<(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        size_type n = lhs.size_ < rhs.size_ ? lhs.size_ : rhs.size_;
        for (size_type i = 0; i < n; ++i) {
            if (lhs.data()[i] < rhs.data()[i]) return true;
            if (rhs.data()[i] < lhs.data()[i]) return false;
        }
        return lhs.size_ < rhs.size_;
    }
    friend constexpr bool operator>(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        return rhs < lhs;
    }
    friend constexpr bool operator<=(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        return !(rhs < lhs);
    }
    friend constexpr bool operator>=(const fixed_vector& lhs, const fixed_vector& rhs) noexcept {
        return !(lhs < rhs);
    }

    // コピー/ムーブコンストラクタ・代入演算子
    constexpr fixed_vector(const fixed_vector& other) noexcept : size_(0) {
        assign(other.size_, T{});
        for (size_type i = 0; i < other.size_; ++i) {
            new (data() + i) T(other.data()[i]);
        }
        size_ = other.size_;
    }
    constexpr fixed_vector& operator=(const fixed_vector& other) noexcept {
        if (this != &other) {
            clear();
            for (size_type i = 0; i < other.size_; ++i) {
                push_back(other.data()[i]);
            }
        }
        return *this;
    }
    constexpr fixed_vector(fixed_vector&& other) noexcept : size_(0) {
        assign(other.size_, T{});
        for (size_type i = 0; i < other.size_; ++i) {
            new (data() + i) T(std::move(other.data()[i]));
        }
        size_ = other.size_;
        other.clear();
    }
    constexpr fixed_vector& operator=(fixed_vector&& other) noexcept {
        if (this != &other) {
            clear();
            for (size_type i = 0; i < other.size_; ++i) {
                push_back(std::move(other.data()[i]));
            }
            other.clear();
        }
        return *this;
    }

    // イニシャライザリストコンストラクタ
    constexpr fixed_vector(std::initializer_list<T> ilist) noexcept : size_(0) {
        for (const auto& v : ilist) {
            push_back(v);
        }
    }

private:
    alignas(T) unsigned char storage_[sizeof(T) * Capacity];
    size_type size_;
};

} // namespace bluestl
