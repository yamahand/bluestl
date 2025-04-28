#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace bluestl {

// tuple_impl: 再帰的に値を格納
struct tuple_impl_base {};

template <typename... Types>
struct tuple_impl;

template <typename Head, typename... Tail>
struct tuple_impl<Head, Tail...> {
    Head value;
    tuple_impl<Tail...> tail;
    constexpr tuple_impl() = default;
    constexpr tuple_impl(const Head& h, const Tail&... t) : value(h), tail(t...) {}

    //template <typename OtherTuple>
    //requires std::is_same_v<std::remove_cvref_t<OtherTuple>, tuple<Types...>>
    //constexpr tuple_impl(OtherTuple &&other) : value(std::forward<OtherTuple>(other).value) { }

    template <typename UHead, typename... UTail>
    constexpr tuple_impl(UHead&& h, UTail&&... t)
    requires (std::is_constructible_v<Head, UHead&&>) : value(std::forward<UHead>(h)), tail(std::forward<UTail>(t)...) {}
};

template <>
struct tuple_impl<> : tuple_impl_base {
    constexpr tuple_impl() = default;
};

// tuple本体
template <typename... Types>
class tuple : private tuple_impl<Types...> {
    using impl_type = tuple_impl<Types...>;
public:
    constexpr tuple() = default;
    template <typename = void>
    constexpr tuple(const Types&... args) requires (sizeof...(Types) > 0) : impl_type(args...) {}
    template <typename... UTypes>
    constexpr tuple(UTypes&&... args) : impl_type(std::forward<UTypes>(args)...) {}


    // get<I>
    template <std::size_t I>
    constexpr auto& get() & { return get_impl<I>(*this); }
    template <std::size_t I>
    constexpr const auto& get() const & { return get_impl<I>(*this); }
    template <std::size_t I>
    constexpr auto&& get() && { return std::move(get_impl<I>(*this)); }
    template <std::size_t I>
    constexpr const auto&& get() const && { return std::move(get_impl<I>(*this)); }

    // 比較演算子
    friend constexpr bool operator==(const tuple& lhs, const tuple& rhs) noexcept {
        return tuple_equal(lhs, rhs, std::make_index_sequence<sizeof...(Types)>{});
    }
    friend constexpr bool operator!=(const tuple& lhs, const tuple& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend constexpr bool operator<(const tuple& lhs, const tuple& rhs) noexcept {
        return tuple_less(lhs, rhs, std::make_index_sequence<sizeof...(Types)>{});
    }
    friend constexpr bool operator>(const tuple& lhs, const tuple& rhs) noexcept {
        return rhs < lhs;
    }
    friend constexpr bool operator<=(const tuple& lhs, const tuple& rhs) noexcept {
        return !(rhs < lhs);
    }
    friend constexpr bool operator>=(const tuple& lhs, const tuple& rhs) noexcept {
        return !(lhs < rhs);
    }
private:
    // get_impl: I番目の値を取得
    template <std::size_t I, typename TupleT>
    static constexpr auto& get_impl(TupleT& t) {
        if constexpr (I == 0) {
            return t.value;
        } else {
            return get_impl<I - 1>(t.tail);
        }
    }
    // 比較ヘルパー
    template <typename TupleT, std::size_t... Is>
    static constexpr bool tuple_equal(const TupleT& lhs, const TupleT& rhs, std::index_sequence<Is...>) {
        return ((lhs.template get<Is>() == rhs.template get<Is>()) && ...);
    }
    template <typename TupleT, std::size_t... Is>
    static constexpr bool tuple_less(const TupleT& lhs, const TupleT& rhs, std::index_sequence<Is...>) {
        return tuple_less_impl(lhs, rhs, std::index_sequence<Is...>{});
    }
    template <typename TupleT, std::size_t I0, std::size_t... Is>
    static constexpr bool tuple_less_impl(const TupleT& lhs, const TupleT& rhs, std::index_sequence<I0, Is...>) {
        if (lhs.template get<I0>() < rhs.template get<I0>()) return true;
        if (rhs.template get<I0>() < lhs.template get<I0>()) return false;
        if constexpr (sizeof...(Is) == 0) return false;
        else return tuple_less_impl(lhs, rhs, std::index_sequence<Is...>{});
    }
    template <typename TupleT>
    static constexpr bool tuple_less_impl(const TupleT&, const TupleT&, std::index_sequence<>) {
        return false;
    }
};

// tuple_size
template <typename T>
struct tuple_size;

template <typename... Types>
struct tuple_size<tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

// tuple_element
template <std::size_t I, typename Tuple>
struct tuple_element;

template <std::size_t I, typename Head, typename... Tail>
struct tuple_element<I, tuple<Head, Tail...>> : tuple_element<I - 1, tuple<Tail...>> {};

template <typename Head, typename... Tail>
struct tuple_element<0, tuple<Head, Tail...>> {
    using type = Head;
};

template <std::size_t I>
struct tuple_element<I, tuple<>> { static_assert(I < 0, "tuple_element index out of range"); };

// get関数
template <std::size_t I, typename... Types>
constexpr auto& get(tuple<Types...>& t) noexcept {
    return t.template get<I>();
}
template <std::size_t I, typename... Types>
constexpr const auto& get(const tuple<Types...>& t) noexcept {
    return t.template get<I>();
}
template <std::size_t I, typename... Types>
constexpr auto&& get(tuple<Types...>&& t) noexcept {
    return std::move(t).template get<I>();
}
template <std::size_t I, typename... Types>
constexpr const auto&& get(const tuple<Types...>&& t) noexcept {
    return std::move(t).template get<I>();
}

// tuple_cat用ヘルパー
namespace detail {
    template <typename Tuple1, typename Tuple2, std::size_t... I1, std::size_t... I2>
    constexpr auto tuple_cat_two_impl(Tuple1&& t1, Tuple2&& t2,
        std::index_sequence<I1...>, std::index_sequence<I2...>) {
        return bluestl::tuple<
            std::remove_reference_t<decltype(bluestl::get<I1>(std::forward<Tuple1>(t1)))>...,
            std::remove_reference_t<decltype(bluestl::get<I2>(std::forward<Tuple2>(t2)))>...
        >(
            bluestl::get<I1>(std::forward<Tuple1>(t1))...,
            bluestl::get<I2>(std::forward<Tuple2>(t2))...
        );
    }
    template <typename Tuple1, typename Tuple2, typename... Tuples>
    constexpr auto tuple_cat_variadic(Tuple1&& t1, Tuple2&& t2, Tuples&&... ts) {
        auto merged = tuple_cat_two_impl(
            std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
            std::make_index_sequence<bluestl::tuple_size<std::remove_reference_t<Tuple1>>::value>{},
            std::make_index_sequence<bluestl::tuple_size<std::remove_reference_t<Tuple2>>::value>{}
        );
        if constexpr (sizeof...(ts) == 0) {
            return merged;
        } else {
            return tuple_cat_variadic(std::move(merged), std::forward<Tuples>(ts)...);
        }
    }
    template <typename Tuple1>
    constexpr auto tuple_cat_variadic(Tuple1&& t1) {
        return std::forward<Tuple1>(t1);
    }
}

template <typename... Tuples>
constexpr auto tuple_cat(Tuples&&... tuples) {
    if constexpr (sizeof...(tuples) == 0) {
        return bluestl::tuple<>{};
    } else {
        return detail::tuple_cat_variadic(std::forward<Tuples>(tuples)...);
    }
}

} // namespace bluestl