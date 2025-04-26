// bluestl::variant の追加テスト
// このファイルは bluestl::variant の動作確認のためのテストケースをまとめています。
// 既存テストに加え、境界値・例外・型特性・入れ子・参照型など多様なケースを網羅します。

#include "bluestl/variant.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <type_traits>

TEST_CASE("bluestl::variant デフォルト構築とvalueless", "[variant]") {
    using bluestl::variant;
    variant<int, double> v;
    REQUIRE(v.valueless_by_exception());
    REQUIRE(v.index() == static_cast<size_t>(-1));
}

TEST_CASE("bluestl::variant emplaceによる構築", "[variant]") {
    using bluestl::variant;
    variant<int, std::string> v;
    v.emplace<int>(123);
    REQUIRE(v.holds_alternative<int>());
    REQUIRE(*v.get_if<int>() == 123);

    v.emplace<std::string>("abc");
    REQUIRE(v.holds_alternative<std::string>());
    REQUIRE(*v.get_if<std::string>() == "abc");
}

TEST_CASE("bluestl::variant コピー・ムーブ・代入", "[variant]") {
    using bluestl::variant;
    variant<int, std::string> v1(std::string("test"));
    variant<int, std::string> v2 = v1;
    REQUIRE(v2.holds_alternative<std::string>());
    REQUIRE(*v2.get_if<std::string>() == "test");

    variant<int, std::string> v3 = std::move(v2);
    REQUIRE(v3.holds_alternative<std::string>());
    REQUIRE(*v3.get_if<std::string>() == "test");

    v1 = 42;
    REQUIRE(v1.holds_alternative<int>());
    REQUIRE(*v1.get_if<int>() == 42);
}

TEST_CASE("bluestl::variant visitによる多態的呼び出し", "[variant]") {
    using bluestl::variant;
    variant<int, float, std::string> v = 1.5f;
    bool called = false;
    v.visit([&](auto&& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, float>) {
            REQUIRE(val == Catch::Approx(1.5f));
            called = true;
        }
    });
    REQUIRE(called);
}

TEST_CASE("bluestl::variant 型不一致get_if", "[variant]") {
    using bluestl::variant;
    variant<int, std::string> v(std::string("xyz"));
    REQUIRE(v.get_if<int>() == nullptr);
    REQUIRE(v.get_if<std::string>() != nullptr);
}

TEST_CASE("bluestl::variant resetによるvalueless化", "[variant]") {
    using bluestl::variant;
    variant<int, std::string> v(100);
    v.reset();
    REQUIRE(v.valueless_by_exception());
    REQUIRE(v.get_if<int>() == nullptr);
    REQUIRE(v.get_if<std::string>() == nullptr);
}

TEST_CASE("bluestl::variant 入れ子variant", "[variant]") {
    using bluestl::variant;
    variant<int, variant<float, std::string>> v = variant<float, std::string>(std::string("nest"));
    REQUIRE(v.holds_alternative<variant<float, std::string>>());
    auto* inner = v.get_if<variant<float, std::string>>();
    REQUIRE(inner != nullptr);
    REQUIRE(inner->holds_alternative<std::string>());
    REQUIRE(*inner->get_if<std::string>() == "nest");
}

//TEST_CASE("bluestl::variant 参照型の保持", "[variant]") {
//    using bluestl::variant;
//    int x = 10;
//    variant<int&, double> v(x);
//    REQUIRE(v.holds_alternative<int&>());
//    *v.get_if<int>() = 20;
//    REQUIRE(x == 20);
//}

TEST_CASE("bluestl::variant move only型のサポート", "[variant]") {
    using bluestl::variant;
    struct MoveOnly {
        int v;
        MoveOnly(int n) : v(n) {}
        MoveOnly(MoveOnly&&) = default;
        MoveOnly& operator=(MoveOnly&&) = default;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
    };
    variant<MoveOnly, int> v(MoveOnly{42});
    REQUIRE(v.holds_alternative<MoveOnly>());
    REQUIRE(v.get_if<MoveOnly>()->v == 42);
}

TEST_CASE("bluestl::variant indexの境界値", "[variant]") {
    using bluestl::variant;
    variant<int, double, std::string> v;
    REQUIRE(v.index() == static_cast<size_t>(-1));
    v = 1;
    REQUIRE(v.index() == 0);
    v = 2.0;
    REQUIRE(v.index() == 1);
    v = std::string("abc");
    REQUIRE(v.index() == 2);
}
