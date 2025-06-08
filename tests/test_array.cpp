/**
 * @file test_array.cpp
 * @brief arrayクラスのテストコード
 * @author BlueStl
 * @date 2025
 */

#include "bluestl/array.h"
#include <catch2/catch_test_macros.hpp>
#include <compare>
#include <type_traits>
#include <algorithm>

TEST_CASE("bluestl::array 基本操作", "[array]") {
    SECTION("基本的な構築と初期化") {
        bluestl::array<int, 5> arr1{};
        REQUIRE(arr1.size() == 5);
        REQUIRE(!arr1.empty());

        bluestl::array<int, 5> arr2 = { 1, 2, 3, 4, 5 };
        REQUIRE(arr2.size() == 5);
        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[4] == 5);
    }

    SECTION("集約初期化") {
        bluestl::array<int, 3> arr = { 10, 20, 30 };
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 20);
        REQUIRE(arr[2] == 30);
    }

    SECTION("部分初期化") {
        bluestl::array<int, 5> arr = { 1, 2 };
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 0);  // 残りは値初期化される
        REQUIRE(arr[3] == 0);
        REQUIRE(arr[4] == 0);
    }
}

TEST_CASE("bluestl::array 要素アクセス", "[array]") {
    bluestl::array<int, 4> arr = { 10, 20, 30, 40 };

    SECTION("operator[]") {
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 20);
        REQUIRE(arr[2] == 30);
        REQUIRE(arr[3] == 40);

        arr[1] = 25;
        REQUIRE(arr[1] == 25);
    }

    SECTION("at()メソッド") {
        REQUIRE(arr.at(0) == 10);
        REQUIRE(arr.at(3) == 40);

        arr.at(2) = 35;
        REQUIRE(arr.at(2) == 35);
    }

    SECTION("front()とback()") {
        REQUIRE(arr.front() == 10);
        REQUIRE(arr.back() == 40);

        arr.front() = 15;
        arr.back() = 45;
        REQUIRE(arr.front() == 15);
        REQUIRE(arr.back() == 45);
    }

    SECTION("data()ポインタアクセス") {
        int* ptr = arr.data();
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == 10);
        REQUIRE(*(ptr + 1) == 20);

        const auto& const_arr = arr;
        const int* const_ptr = const_arr.data();
        REQUIRE(const_ptr != nullptr);
        REQUIRE(*const_ptr == 10);
    }
}

TEST_CASE("bluestl::array イテレータ", "[array]") {
    bluestl::array<int, 5> arr = { 1, 2, 3, 4, 5 };

    SECTION("前方イテレータ") {
        int expected = 1;
        for (auto it = arr.begin(); it != arr.end(); ++it, ++expected) {
            REQUIRE(*it == expected);
        }
    }

    SECTION("範囲ベースfor") {
        int expected = 1;
        for (auto& elem : arr) {
            REQUIRE(elem == expected);
            ++expected;
        }
    }

    SECTION("const イテレータ") {
        const auto& const_arr = arr;
        int expected = 1;
        for (auto it = const_arr.cbegin(); it != const_arr.cend(); ++it, ++expected) {
            REQUIRE(*it == expected);
            static_assert(std::is_same_v<decltype(*it), const int&>);
        }
    }

    SECTION("逆方向イテレータ") {
        int expected = 5;
        for (auto it = arr.rbegin(); it != arr.rend(); ++it, --expected) {
            REQUIRE(*it == expected);
        }
    }

    SECTION("const 逆方向イテレータ") {
        const auto& const_arr = arr;
        int expected = 5;
        for (auto it = const_arr.crbegin(); it != const_arr.crend(); ++it, --expected) {
            REQUIRE(*it == expected);
            static_assert(std::is_same_v<decltype(*it), const int&>);
        }
    }
}

TEST_CASE("bluestl::array 容量", "[array]") {
    SECTION("通常の配列") {
        bluestl::array<int, 10> arr;
        REQUIRE(arr.size() == 10);
        REQUIRE(arr.max_size() == 10);
        REQUIRE(!arr.empty());
    }

    SECTION("サイズ1の配列") {
        bluestl::array<double, 1> arr = { 3.14 };
        REQUIRE(arr.size() == 1);
        REQUIRE(arr.max_size() == 1);
        REQUIRE(!arr.empty());
        REQUIRE(arr[0] == 3.14);
    }
}

TEST_CASE("bluestl::array fill操作", "[array]") {
    bluestl::array<int, 5> arr;

    SECTION("fill操作") {
        arr.fill(42);
        for (std::size_t i = 0; i < arr.size(); ++i) {
            REQUIRE(arr[i] == 42);
        }
    }

    SECTION("fill後の変更") {
        arr.fill(10);
        arr[2] = 20;
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 10);
        REQUIRE(arr[2] == 20);
        REQUIRE(arr[3] == 10);
        REQUIRE(arr[4] == 10);
    }
}

TEST_CASE("bluestl::array swap操作", "[array]") {
    bluestl::array<int, 3> arr1 = { 1, 2, 3 };
    bluestl::array<int, 3> arr2 = { 10, 20, 30 };

    SECTION("メンバ関数swap") {
        arr1.swap(arr2);

        REQUIRE(arr1[0] == 10);
        REQUIRE(arr1[1] == 20);
        REQUIRE(arr1[2] == 30);

        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[1] == 2);
        REQUIRE(arr2[2] == 3);
    }

    SECTION("非メンバ関数swap") {
        using bluestl::swap;
        swap(arr1, arr2);

        REQUIRE(arr1[0] == 10);
        REQUIRE(arr1[1] == 20);
        REQUIRE(arr1[2] == 30);

        REQUIRE(arr2[0] == 1);
        REQUIRE(arr2[1] == 2);
        REQUIRE(arr2[2] == 3);
    }
}

TEST_CASE("bluestl::array 比較操作", "[array]") {
    bluestl::array<int, 3> arr1 = { 1, 2, 3 };
    bluestl::array<int, 3> arr2 = { 1, 2, 3 };
    bluestl::array<int, 3> arr3 = { 1, 2, 4 };
    bluestl::array<int, 3> arr4 = { 1, 1, 3 };

    SECTION("等価比較") {
        REQUIRE(arr1 == arr2);
        REQUIRE(!(arr1 == arr3));
        REQUIRE(!(arr1 == arr4));
    }

    SECTION("三方比較") {
        REQUIRE((arr1 <=> arr2) == std::strong_ordering::equal);
        REQUIRE((arr1 <=> arr3) == std::strong_ordering::less);
        REQUIRE((arr3 <=> arr1) == std::strong_ordering::greater);
        REQUIRE((arr1 <=> arr4) == std::strong_ordering::greater);
    }
}

TEST_CASE("bluestl::array get関数とタプル風アクセス", "[array]") {
    bluestl::array<int, 4> arr = { 10, 20, 30, 40 };

    SECTION("get関数") {
        REQUIRE(get<0>(arr) == 10);
        REQUIRE(get<1>(arr) == 20);
        REQUIRE(get<2>(arr) == 30);
        REQUIRE(get<3>(arr) == 40);

        get<1>(arr) = 25;
        REQUIRE(get<1>(arr) == 25);
    }

    SECTION("const get関数") {
        const auto& const_arr = arr;
        REQUIRE(get<0>(const_arr) == 10);
        REQUIRE(get<3>(const_arr) == 40);
        static_assert(std::is_same_v<decltype(get<0>(const_arr)), const int&>);
    }

    SECTION("move get関数") {
        auto moved_arr = std::move(arr);
        auto value = get<0>(std::move(moved_arr));
        REQUIRE(value == 10);
    }
}

TEST_CASE("bluestl::array 型特性", "[array]") {
    using ArrayType = bluestl::array<int, 5>;

    SECTION("型エイリアス") {
        static_assert(std::is_same_v<ArrayType::value_type, int>);
        static_assert(std::is_same_v<ArrayType::size_type, std::size_t>);
        static_assert(std::is_same_v<ArrayType::reference, int&>);
        static_assert(std::is_same_v<ArrayType::const_reference, const int&>);
        static_assert(std::is_same_v<ArrayType::pointer, int*>);
        static_assert(std::is_same_v<ArrayType::const_pointer, const int*>);
    }

    SECTION("イテレータ型") {
        static_assert(std::is_same_v<ArrayType::iterator, int*>);
        static_assert(std::is_same_v<ArrayType::const_iterator, const int*>);
    }

    SECTION("tuple_size と tuple_element") {
        static_assert(std::tuple_size_v<ArrayType> == 5);
        static_assert(std::is_same_v<std::tuple_element_t<0, ArrayType>, int>);
        static_assert(std::is_same_v<std::tuple_element_t<4, ArrayType>, int>);
    }
}

TEST_CASE("bluestl::array 0サイズ配列", "[array]") {
    bluestl::array<int, 0> empty_arr;

    SECTION("基本プロパティ") {
        REQUIRE(empty_arr.size() == 0);
        REQUIRE(empty_arr.max_size() == 0);
        REQUIRE(empty_arr.empty());
        REQUIRE(empty_arr.data() == nullptr);
    }

    SECTION("イテレータ") {
        REQUIRE(empty_arr.begin() == empty_arr.end());
        REQUIRE(empty_arr.cbegin() == empty_arr.cend());
        REQUIRE(empty_arr.rbegin() == empty_arr.rend());
        REQUIRE(empty_arr.crbegin() == empty_arr.crend());
    }

    SECTION("比較") {
        bluestl::array<int, 0> other_empty;
        REQUIRE(empty_arr == other_empty);
        REQUIRE((empty_arr <=> other_empty) == std::strong_ordering::equal);
    }

    SECTION("操作") {
        empty_arr.fill(42);  // 何もしない
        bluestl::array<int, 0> other_empty;
        empty_arr.swap(other_empty);  // 何もしない
    }
}

TEST_CASE("bluestl::array 推論ガイド", "[array]") {
    SECTION("推論ガイドの動作") {
        auto arr1 = bluestl::array{ 1, 2, 3, 4, 5 };
        static_assert(std::is_same_v<decltype(arr1), bluestl::array<int, 5>>);
        REQUIRE(arr1.size() == 5);
        REQUIRE(arr1[0] == 1);
        REQUIRE(arr1[4] == 5);

        auto arr2 = bluestl::array{ 1.0, 2.0, 3.0 };
        static_assert(std::is_same_v<decltype(arr2), bluestl::array<double, 3>>);
        REQUIRE(arr2.size() == 3);
    }

    SECTION("単一要素の推論") {
        auto arr = bluestl::array{ 42 };
        static_assert(std::is_same_v<decltype(arr), bluestl::array<int, 1>>);
        REQUIRE(arr.size() == 1);
        REQUIRE(arr[0] == 42);
    }
}

TEST_CASE("bluestl::array 標準アルゴリズムとの互換性", "[array]") {
    bluestl::array<int, 5> arr = { 5, 2, 8, 1, 9 };

    SECTION("std::sort") {
        std::sort(arr.begin(), arr.end());
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 5);
        REQUIRE(arr[3] == 8);
        REQUIRE(arr[4] == 9);
    }

    SECTION("std::find") {
        auto it = std::find(arr.begin(), arr.end(), 8);
        REQUIRE(it != arr.end());
        REQUIRE(*it == 8);
    }

    SECTION("std::copy") {
        bluestl::array<int, 5> dest;
        std::copy(arr.begin(), arr.end(), dest.begin());
        REQUIRE(dest == arr);
    }
}

TEST_CASE("bluestl::array constexpr サポート", "[array]") {
    SECTION("constexpr 構築") {
        constexpr bluestl::array<int, 3> arr = { 1, 2, 3 };
        static_assert(arr.size() == 3);
        static_assert(arr[0] == 1);
        static_assert(arr[2] == 3);
    }

    SECTION("constexpr 操作") {
        constexpr bluestl::array<int, 3> arr = { 10, 20, 30 };
        static_assert(!arr.empty());
        static_assert(arr.front() == 10);
        static_assert(arr.back() == 30);
    }

    SECTION("constexpr 比較") {
        constexpr bluestl::array<int, 2> arr1 = { 1, 2 };
        constexpr bluestl::array<int, 2> arr2 = { 1, 2 };
        constexpr bluestl::array<int, 2> arr3 = { 1, 3 };

        static_assert(arr1 == arr2);
        static_assert(!(arr1 == arr3));
    }
}
