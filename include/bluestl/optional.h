#pragma once

#include <utility>
#include <type_traits>

namespace bluestl {

template <typename T>
class optional {
public:
    constexpr optional() noexcept : has_value_(false) {}

    constexpr optional(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(true) {
        new (&storage_) T(value);
    }

    constexpr optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(true) {
        new (&storage_) T(std::move(value));
    }

    constexpr optional(const optional& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(*other);
        }
    }

    constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(std::move(*other));
        }
    }

    constexpr ~optional() { reset(); }

    constexpr optional& operator=(const optional& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                **get() = *other;
            } else if (has_value_) {
                reset();
            } else if (other.has_value_) {
                emplace(*other);
            }
        }
        return *this;
    }

    constexpr optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                **get() = std::move(*other);
            } else if (has_value_) {
                reset();
            } else if (other.has_value_) {
                emplace(std::move(*other));
            }
        }
        return *this;
    }

    template <typename... Args>
    constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        reset();
        new (&storage_) T(std::forward<Args>(args)...);
        has_value_ = true;
        return **this;
    }

    constexpr void reset() noexcept {
        if (has_value_) {
            get()->~T();
            has_value_ = false;
        }
    }

    constexpr T* operator->() noexcept { return get(); }
    constexpr const T* operator->() const noexcept { return get(); }
    constexpr T& operator*() noexcept { return *get(); }
    constexpr const T& operator*() const noexcept { return *get(); }

    constexpr explicit operator bool() const noexcept { return has_value_; }
    constexpr bool has_value() const noexcept { return has_value_; }

    constexpr T& value() {
        BLUESTL_ASSERT(has_value());
        return *get();
    }
    constexpr const T& value() const {
        BLUESTL_ASSERT(has_value());
        return *get();
    }

private:
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    storage_t storage_;
    bool has_value_;

    constexpr T* get() noexcept { return reinterpret_cast<T*>(&storage_); }
    constexpr const T* get() const noexcept { return reinterpret_cast<const T*>(&storage_); }
};

} // namespace bluestl
