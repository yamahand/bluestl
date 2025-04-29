#pragma once

#include <utility>
#include <type_traits>
#include <cstddef>

#if _HAS_CXX20
#include <compare>
#include <concepts>
#endif  // _HAS_CXX20

#if _HAS_CXX23
#include <cstdlib>
#endif  // _HAS_CXX23

#include "assert_handler.h"

namespace bluestl {
#if defined(_HAS_CXX20) && _HAS_CXX20
template <class T>
concept IsTupleLike = requires(T t) { std::get<0>(t); };
#else
template <class T>
struct IsTupleLikeImpl {
   private:
    template <class U>
    static auto test(int) -> decltype(std::get<0>(std::declval<U>()), std::true_type{});
    template <class>
    static auto test(...) -> std::false_type;

   public:
    static constexpr bool value = decltype(test<T>(0))::value;
};
template <class T>
constexpr bool IsTupleLike = IsTupleLikeImpl<T>::value;
#endif

// piecewise_construct用タグ
struct piecewise_construct_t {
    explicit piecewise_construct_t() = default;
};
inline constexpr piecewise_construct_t piecewise_construct{};

// pair本体
template <class T1, class T2>
struct pair {
    using first_type = T1;
    using second_type = T2;

    first_type first;
    second_type second;

    // デフォルトコンストラクタ
    constexpr pair() noexcept(std::is_nothrow_default_constructible_v<T1> &&
                              std::is_nothrow_default_constructible_v<T2>)
        requires std::is_default_constructible_v<T1> && std::is_default_constructible_v<T2>
        : first(), second() {}

    // コピーコンストラクタ
    constexpr pair(const pair& p) noexcept(std::is_nothrow_copy_constructible_v<T1> &&
                                           std::is_nothrow_copy_constructible_v<T2>)
        requires std::is_copy_constructible_v<T1> && std::is_copy_constructible_v<T2>
    = default;
    // ムーブコンストラクタ
    constexpr pair(pair&& p) noexcept(std::is_nothrow_move_constructible_v<T1> &&
                                      std::is_nothrow_move_constructible_v<T2>)
        requires std::is_move_constructible_v<T1> && std::is_move_constructible_v<T2>
    = default;
    // コピー代入
    constexpr pair& operator=(const pair& p) noexcept(std::is_nothrow_copy_assignable_v<T1> &&
                                                      std::is_nothrow_copy_assignable_v<T2>)
        requires std::is_copy_assignable_v<T1> && std::is_copy_assignable_v<T2>
    = default;
    // ムーブ代入
    constexpr pair& operator=(pair&& p) noexcept(std::is_nothrow_move_assignable_v<T1> &&
                                                 std::is_nothrow_move_assignable_v<T2>)
        requires std::is_move_assignable_v<T1> && std::is_move_assignable_v<T2>
    = default;

    // 通常のコンストラクタ
    constexpr pair(const T1& f, const T2& s) noexcept(std::is_nothrow_copy_constructible_v<T1> &&
                                                      std::is_nothrow_copy_constructible_v<T2>)
        : first(f), second(s) {}
    constexpr pair(T1&& f, T2&& s) noexcept(std::is_nothrow_move_constructible_v<T1> &&
                                            std::is_nothrow_move_constructible_v<T2>)
        : first(std::move(f)), second(std::move(s)) {}

    // 他の型からのコピー/ムーブ
    template <class U1, class U2>
    constexpr pair(const pair<U1, U2>& p) noexcept(std::is_nothrow_constructible_v<T1, const U1&> &&
                                                   std::is_nothrow_constructible_v<T2, const U2&>)
        : first(p.first), second(p.second) {}
    template <class U1, class U2>
    constexpr pair(pair<U1, U2>&& p) noexcept(std::is_nothrow_constructible_v<T1, U1&&> &&
                                              std::is_nothrow_constructible_v<T2, U2&&>)
        : first(std::forward<U1>(p.first)), second(std::forward<U2>(p.second)) {}

    // piecewise_construct対応コンストラクタ
    template <class... Args1, class... Args2>
    constexpr pair(piecewise_construct_t, std::tuple<Args1...> first_args, std::tuple<Args2...> second_args)
        : first(make_from_tuple_helper<T1>(std::move(first_args), std::index_sequence_for<Args1...>{})),
          second(make_from_tuple_helper<T2>(std::move(second_args), std::index_sequence_for<Args2...>{})) {}

    // タプルからの構築
    template <class Tuple1, class Tuple2>
        requires IsTupleLike<Tuple1> && IsTupleLike<Tuple2>
    constexpr pair(Tuple1&& t1, Tuple2&& t2)
        : first(std::get<0>(std::forward<Tuple1>(t1))), second(std::get<0>(std::forward<Tuple2>(t2))) {}

    // swapメンバ関数
    constexpr void swap(pair& other) noexcept {
        using std::swap;
        swap(first, other.first);
        swap(second, other.second);
    }

    // 比較演算子
    friend constexpr bool operator==(const pair& lhs, const pair& rhs) noexcept {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
    friend constexpr bool operator!=(const pair& lhs, const pair& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend constexpr bool operator<(const pair& lhs, const pair& rhs) noexcept {
        return (lhs.first < rhs.first) || (!(rhs.first < lhs.first) && lhs.second < rhs.second);
    }
    friend constexpr bool operator>(const pair& lhs, const pair& rhs) noexcept {
        return rhs < lhs;
    }
    friend constexpr bool operator<=(const pair& lhs, const pair& rhs) noexcept {
        return !(rhs < lhs);
    }
    friend constexpr bool operator>=(const pair& lhs, const pair& rhs) noexcept {
        return !(lhs < rhs);
    }

#if _HAS_CXX20
    // <=>（宇宙船演算子）による比較
    friend constexpr auto operator<=>(const pair& lhs, const pair& rhs) noexcept(noexcept(lhs.first <=> rhs.first) &&
                                                                                 noexcept(lhs.second <=> rhs.second))
        requires requires {
            lhs.first <=> rhs.first;
            lhs.second <=> rhs.second;
        }
    {
        if (auto cmp = lhs.first <=> rhs.first; cmp != 0) return cmp;
        return lhs.second <=> rhs.second;
    }
#endif

   private:
    template <class T, class Tuple, size_t... Indices>
    static constexpr T construct_from_tuple(Tuple&& t, std::index_sequence<Indices...>) {
        return T(std::get<Indices>(std::forward<Tuple>(t))...);
    }

    // make_from_tupleのヘルパー関数
    template <class T, class Tuple, std::size_t... Is>
    static constexpr T make_from_tuple_helper(Tuple&& t, std::index_sequence<Is...>) {
        return T(std::get<Is>(std::forward<Tuple>(t))...);
    }
};

// CTAD (Class Template Argument Deduction) ガイド
template <class T1, class T2>
pair(T1, T2) -> pair<T1, T2>;
template <class U1, class U2>
pair(const pair<U1, U2>&) -> pair<U1, U2>;
template <class U1, class U2>
pair(pair<U1, U2>&&) -> pair<U1, U2>;

// swap非メンバ関数
template <class T1, class T2>
constexpr void swap(pair<T1, T2>& a, pair<T1, T2>& b) noexcept {
    a.swap(b);
}

// make_pair
template <class T1, class T2>
constexpr auto make_pair(T1&& a, T2&& b) noexcept {
    return pair<std::decay_t<T1>, std::decay_t<T2>>(std::forward<T1>(a), std::forward<T2>(b));
}

// tuple_size, tuple_element, get
template <std::size_t I, class T1, class T2>
constexpr auto& get(pair<T1, T2>& p) noexcept {
    static_assert(I < 2, "pairのgetは0または1のみ有効です");
    if constexpr (I == 0)
        return p.first;
    else
        return p.second;
}
template <std::size_t I, class T1, class T2>
constexpr const auto& get(const pair<T1, T2>& p) noexcept {
    static_assert(I < 2, "pairのgetは0または1のみ有効です");
    if constexpr (I == 0)
        return p.first;
    else
        return p.second;
}
template <std::size_t I, class T1, class T2>
constexpr auto&& get(pair<T1, T2>&& p) noexcept {
    static_assert(I < 2, "pairのgetは0または1のみ有効です");
    if constexpr (I == 0)
        return std::move(p.first);
    else
        return std::move(p.second);
}
template <std::size_t I, class T1, class T2>
constexpr const auto&& get(const pair<T1, T2>&& p) noexcept {
    static_assert(I < 2, "pairのgetは0または1のみ有効です");
    if constexpr (I == 0)
        return std::move(p.first);
    else
        return std::move(p.second);
}
template <class T>
struct tuple_size;
template <class T1, class T2>
struct tuple_size<pair<T1, T2>> : std::integral_constant<std::size_t, 2> {};
template <std::size_t I, class T>
struct tuple_element;
template <class T1, class T2>
struct tuple_element<0, pair<T1, T2>> {
    using type = T1;
};
template <class T1, class T2>
struct tuple_element<1, pair<T1, T2>> {
    using type = T2;
};

// tupleからpairを生成するユーティリティ関数
#if defined(_HAS_CXX20) && _HAS_CXX20
template <class Tuple1, class Tuple2>
constexpr auto pair_from_tuples(Tuple1&& t1, Tuple2&& t2)
    requires(IsTupleLike<Tuple1> && IsTupleLike<Tuple2>)
{
    using First = std::remove_cvref_t<decltype(std::get<0>(t1))>;
    using Second = std::remove_cvref_t<decltype(std::get<0>(t2))>;
    return pair<First, Second>(std::get<0>(std::forward<Tuple1>(t1)), std::get<0>(std::forward<Tuple2>(t2)));
}
#else
template <class Tuple1, class Tuple2, std::enable_if_t<IsTupleLike<Tuple1> && IsTupleLike<Tuple2>, int> = 0>
constexpr auto pair_from_tuples(Tuple1&& t1, Tuple2&& t2) {
    using First = typename std::remove_cv<typename std::remove_reference<decltype(std::get<0>(t1))>::type>::type;
    using Second = typename std::remove_cv<typename std::remove_reference<decltype(std::get<0>(t2))>::type>::type;
    return pair<First, Second>(std::get<0>(std::forward<Tuple1>(t1)), std::get<0>(std::forward<Tuple2>(t2)));
}
#endif
#if 0
	// bluestl::tuple用のstd::getオーバーロード
	// これによりstd::get<0>(bluestl::tuple<...>)が使える
	// tuple.hの定義に合わせて提供
	template <std::size_t I, typename... Types>
	constexpr auto& get(bluestl::tuple<Types...>& t) noexcept {
		return bluestl::get<I>(t);
	}
	template <std::size_t I, typename... Types>
	constexpr const auto& get(const bluestl::tuple<Types...>& t) noexcept {
		return bluestl::get<I>(t);
	}
	template <std::size_t I, typename... Types>
	constexpr auto&& get(bluestl::tuple<Types...>&& t) noexcept {
		return bluestl::get<I>(std::move(t));
	}
	template <std::size_t I, typename... Types>
	constexpr const auto&& get(const bluestl::tuple<Types...>&& t) noexcept {
		return bluestl::get<I>(std::move(t));
	}
#endif
};  // namespace bluestl