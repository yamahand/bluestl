#pragma once
#include "bluestl/pair.h"
#include <catch2/catch_test_macros.hpp>
#include <string>

using bluestl::pair;
using bluestl::make_pair;
using bluestl::swap;

TEST_CASE("pairの基本動作", "[pair]") {
    pair<int, double> p1;
    REQUIRE(p1.first == 0);
    REQUIRE(p1.second == 0.0);

    pair<int, double> p2(42, 3.14);
    REQUIRE(p2.first == 42);
    REQUIRE(p2.second == 3.14);

    pair<int, double> p3 = p2;
    REQUIRE(p3 == p2);
    REQUIRE_FALSE(p3 != p2);

    pair<int, double> p4(1, 2.0);
    swap(p2, p4);
    REQUIRE(p2.first == 1);
    REQUIRE(p2.second == 2.0);
    REQUIRE(p4.first == 42);
    REQUIRE(p4.second == 3.14);
}

TEST_CASE("make_pairの動作", "[pair]") {
    auto p = bluestl::make_pair(1, std::string("abc"));
    REQUIRE(p.first == 1);
    REQUIRE(p.second == "abc");
}

TEST_CASE("pairの比較演算子", "[pair]") {
    pair<int, int> a(1, 2), b(1, 3), c(2, 1);
    REQUIRE(a < b);
    REQUIRE(b > a);
    REQUIRE(a <= b);
    REQUIRE(b >= a);
    REQUIRE(a != b);
    REQUIRE(a == a);
    REQUIRE(c > a);
}

TEST_CASE("tupleインターフェース(get, tuple_size, tuple_element)", "[pair]") {
    pair<int, double> p(7, 8.5);
    REQUIRE(bluestl::get<0>(p) == 7);
    REQUIRE(bluestl::get<1>(p) == 8.5);
    static_assert(bluestl::tuple_size<pair<int, double>>::value == 2);
    static_assert(std::is_same_v<bluestl::tuple_element<0, pair<int, double>>::type, int>);
    static_assert(std::is_same_v<bluestl::tuple_element<1, pair<int, double>>::type, double>);
}

TEST_CASE("pairのムーブ・コピー代入", "[pair]") {
    pair<std::string, int> p1("abc", 1);
    pair<std::string, int> p2;
    p2 = p1;
    REQUIRE(p2.first == "abc");
    REQUIRE(p2.second == 1);
    pair<std::string, int> p3;
    p3 = std::move(p1);
    REQUIRE(p3.first == "abc");
    REQUIRE(p3.second == 1);
}
