#include "../include/bluestl/tuple.h"
#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("tupleの基本動作", "[tuple]") {
    bluestl::tuple<int, double, char> t1(1, 2.5, 'a');
	auto t11 = bluestl::get<0>(t1);
	auto t12 = bluestl::get<1>(t1);
	auto t13 = bluestl::get<2>(t1);
    REQUIRE(bluestl::get<0>(t1) == 1);
    REQUIRE(bluestl::get<1>(t1) == 2.5);
    REQUIRE(bluestl::get<2>(t1) == 'a');

    bluestl::tuple<> t_empty;
    SUCCEED();
}

TEST_CASE("tupleの比較演算子", "[tuple][compare]") {
    bluestl::tuple<int, double> t1(1, 2.0);
    bluestl::tuple<int, double> t2(1, 2.0);
    bluestl::tuple<int, double> t3(2, 1.0);
    REQUIRE(t1 == t2);
    REQUIRE(t1 != t3);
    REQUIRE(t1 < t3);
    REQUIRE(t3 > t1);
    REQUIRE(t1 <= t2);
    REQUIRE(t3 >= t2);
}

TEST_CASE("tuple_catの動作", "[tuple][tuple_cat]") {
    bluestl::tuple<int, double> t1(1, 2.5);
    bluestl::tuple<char> t2('x');
    auto t3 = bluestl::tuple_cat(t1, t2);
    REQUIRE(bluestl::get<0>(t3) == 1);
    REQUIRE(bluestl::get<1>(t3) == 2.5);
    REQUIRE(bluestl::get<2>(t3) == 'x');

    auto t4 = bluestl::tuple_cat();
    SUCCEED();
}

TEST_CASE("tuple_size, tuple_elementの動作", "[tuple][traits]") {
    using T = bluestl::tuple<int, double, char>;
    static_assert(bluestl::tuple_size<T>::value == 3);
    static_assert(std::is_same_v<bluestl::tuple_element<0, T>::type, int>);
    static_assert(std::is_same_v<bluestl::tuple_element<1, T>::type, double>);
    static_assert(std::is_same_v<bluestl::tuple_element<2, T>::type, char>);
}
