#include "bluestl/optional.h"
#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("bluestl::optional 基本動作", "[optional]") {
    using bluestl::optional;
    // デフォルト構築
    optional<int> o1;
    REQUIRE(!o1.has_value());

    // 値の構築
    optional<int> o2(42);
    REQUIRE(o2.has_value());
    REQUIRE(*o2 == 42);

    // コピー構築
    optional<int> o3(o2);
    REQUIRE(o3.has_value());
    REQUIRE(*o3 == 42);

    // ムーブ構築
    optional<int> o4(std::move(o2));
    REQUIRE(o4.has_value());
    REQUIRE(*o4 == 42);

    // emplace
    o1.emplace(100);
    REQUIRE(o1.has_value());
    REQUIRE(*o1 == 100);

    // reset
    o1.reset();
    REQUIRE(!o1.has_value());

    // operator=
    o1 = o3;
    REQUIRE(o1.has_value());
    REQUIRE(*o1 == 42);

    o1 = optional<int>(77);
    REQUIRE(o1.has_value());
    REQUIRE(*o1 == 77);

    // string型
    optional<std::string> os1;
    REQUIRE(!os1.has_value());
    os1.emplace("hello");
    REQUIRE(os1.has_value());
    REQUIRE(*os1 == "hello");
}
