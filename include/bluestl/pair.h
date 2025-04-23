#pragma once

#include <utility>      // std::move, std::forward, std::swap
#include <type_traits>  // std::decay_t
#include <cstddef>      // std::size_t

#if _HAS_CXX20
#include <compare>
#include <concepts>
#endif // _HAS_CXX20

#if _HAS_CXX23
#include <cstdlib>
#endif // _HAS_CXX23

#include "assert_handler.h"

namespace bluestl
{
    template <class T1, class T2>
    struct pair
    {
        using first_type = T1;
        using second_type = T2;

        first_type first;
        second_type second;

        constexpr pair() noexcept : first(), second() {}
        constexpr pair(const first_type &f, const second_type &s) noexcept : first(f), second(s) {}
        constexpr pair(first_type &&f, second_type &&s) noexcept : first(std::move(f)), second(std::move(s)) {}

        template <class U1, class U2>
        constexpr pair(const pair<U1, U2> &p) noexcept : first(p.first), second(p.second) {}

        template <class U1, class U2>
        constexpr pair(pair<U1, U2> &&p) noexcept : first(std::move(p.first)), second(std::move(p.second)) {}

        template <class U1, class U2>
        constexpr pair &operator=(const pair<U1, U2> &p) noexcept
        {
            first = p.first;
            second = p.second;
            return *this;
        }

        template <class U1, class U2>
        constexpr pair &operator=(pair<U1, U2> &&p) noexcept
        {
            first = std::move(p.first);
            second = std::move(p.second);
            return *this;
        }

        // swapメンバ関数
        constexpr void swap(pair &other) noexcept {
            using std::swap;
            swap(first, other.first);
            swap(second, other.second);
        }

        // 比較演算子
        friend constexpr bool operator==(const pair &lhs, const pair &rhs) noexcept {
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
        friend constexpr bool operator!=(const pair &lhs, const pair &rhs) noexcept {
            return !(lhs == rhs);
        }
        friend constexpr bool operator<(const pair &lhs, const pair &rhs) noexcept {
            return (lhs.first < rhs.first) || (!(rhs.first < lhs.first) && lhs.second < rhs.second);
        }
        friend constexpr bool operator>(const pair &lhs, const pair &rhs) noexcept {
            return rhs < lhs;
        }
        friend constexpr bool operator<=(const pair &lhs, const pair &rhs) noexcept {
            return !(rhs < lhs);
        }
        friend constexpr bool operator>=(const pair &lhs, const pair &rhs) noexcept {
            return !(lhs < rhs);
        }
    };

    // swap非メンバ関数
    template <class T1, class T2>
    constexpr void swap(pair<T1, T2> &a, pair<T1, T2> &b) noexcept {
        a.swap(b);
    }

    // make_pair
    template <class T1, class T2>
    constexpr auto make_pair(T1 &&a, T2 &&b) noexcept {
        return pair<std::decay_t<T1>, std::decay_t<T2>>(std::forward<T1>(a), std::forward<T2>(b));
    }

    // tuple_size, tuple_element, get
    template <std::size_t I, class T1, class T2>
    constexpr auto &get(pair<T1, T2> &p) noexcept {
        static_assert(I < 2, "pairのgetは0または1のみ有効です");
        if constexpr (I == 0) return p.first;
        else return p.second;
    }
    template <std::size_t I, class T1, class T2>
    constexpr const auto &get(const pair<T1, T2> &p) noexcept {
        static_assert(I < 2, "pairのgetは0または1のみ有効です");
        if constexpr (I == 0) return p.first;
        else return p.second;
    }
    template <std::size_t I, class T1, class T2>
    constexpr auto &&get(pair<T1, T2> &&p) noexcept {
        static_assert(I < 2, "pairのgetは0または1のみ有効です");
        if constexpr (I == 0) return std::move(p.first);
        else return std::move(p.second);
    }
    template <std::size_t I, class T1, class T2>
    constexpr const auto &&get(const pair<T1, T2> &&p) noexcept {
        static_assert(I < 2, "pairのgetは0または1のみ有効です");
        if constexpr (I == 0) return std::move(p.first);
        else return std::move(p.second);
    }
    template <class T> struct tuple_size;
    template <class T1, class T2> struct tuple_size<pair<T1, T2>> : std::integral_constant<std::size_t, 2> {};
    template <std::size_t I, class T> struct tuple_element;
    template <class T1, class T2> struct tuple_element<0, pair<T1, T2>> { using type = T1; };
    template <class T1, class T2> struct tuple_element<1, pair<T1, T2>> { using type = T2; };
}; // namespace bluestl