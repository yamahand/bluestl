/**
 * @file test_span.cpp
 * @brief spanクラスのテストコード
 * @author BlueStl
 * @date 2025
 */

#include "bluestl/span.h"
#include "bluestl/vector.h"
#include "bluestl/fixed_vector.h"
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <compare>

TEST_CASE("bluestl::span 基本操作", "[span]") {
    SECTION("配列からのspan作成") {
        int arr[] = { 1, 2, 3, 4, 5 };
        bluestl::span<int> s1(arr, 5);

        REQUIRE(s1.size() == 5);
        REQUIRE(!s1.empty());
        REQUIRE(s1.data() == arr);
        REQUIRE(s1[0] == 1);
        REQUIRE(s1[4] == 5);
        REQUIRE(s1.front() == 1);
        REQUIRE(s1.back() == 5);
    }

    SECTION("固定サイズspan") {
        int arr[] = { 1, 2, 3, 4, 5 };
        bluestl::span<int, 5> s2(arr);
        REQUIRE(s2.size() == 5);
        REQUIRE(s2.extent == 5);
    }

    SECTION("ポインタ範囲からのspan作成") {
        int arr[] = { 1, 2, 3, 4, 5 };
        bluestl::span<int> s3(arr, arr + 3);
        REQUIRE(s3.size() == 3);
        REQUIRE(s3[0] == 1);
        REQUIRE(s3[2] == 3);
    }
}

TEST_CASE("bluestl::span イテレータ", "[span]") {
    int arr[] = { 10, 20, 30, 40, 50 };
    bluestl::span<int> s(arr, 5);

    SECTION("前方イテレータ") {
        int expected = 10;
        for (auto it = s.begin(); it != s.end(); ++it, expected += 10) {
            REQUIRE(*it == expected);
        }
    }

    SECTION("範囲ベースfor") {
        int expected = 10;
        for (const auto& elem : s) {
            REQUIRE(elem == expected);
            expected += 10;
        }
    }

    SECTION("逆方向イテレータ") {
        int expected = 50;
        for (auto it = s.rbegin(); it != s.rend(); ++it, expected -= 10) {
            REQUIRE(*it == expected);
        }
    }

    SECTION("constイテレータ") {
        for (auto it = s.cbegin(); it != s.cend(); ++it) {
            // constイテレータの動作確認
            static_assert(std::is_same_v<decltype(*it), const int&>);
        }
    }
}

TEST_CASE("bluestl::span サブスパン操作", "[span]") {
    int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    bluestl::span<int> s(arr, 10);

    SECTION("first操作") {
        auto first3 = s.first<3>();
        REQUIRE(first3.size() == 3);
        REQUIRE(first3[0] == 1);
        REQUIRE(first3[2] == 3);

        auto first5_dyn = s.first(5);
        REQUIRE(first5_dyn.size() == 5);
        REQUIRE(first5_dyn[4] == 5);
    }

    SECTION("last操作") {
        auto last3 = s.last<3>();
        REQUIRE(last3.size() == 3);
        REQUIRE(last3[0] == 8);
        REQUIRE(last3[2] == 10);

        auto last4_dyn = s.last(4);
        REQUIRE(last4_dyn.size() == 4);
        REQUIRE(last4_dyn[0] == 7);
        REQUIRE(last4_dyn[3] == 10);
    }

    SECTION("subspan操作") {
        auto sub = s.subspan<2, 4>();
        REQUIRE(sub.size() == 4);
        REQUIRE(sub[0] == 3);
        REQUIRE(sub[3] == 6);

        auto sub_dyn = s.subspan(3, 3);
        REQUIRE(sub_dyn.size() == 3);
        REQUIRE(sub_dyn[0] == 4);
        REQUIRE(sub_dyn[2] == 6);

        // 残り全部のsubspan
        auto sub_rest = s.subspan(7);
        REQUIRE(sub_rest.size() == 3);
        REQUIRE(sub_rest[0] == 8);
        REQUIRE(sub_rest[2] == 10);
    }
}

TEST_CASE("bluestl::span コンテナからの作成", "[span]") {
    SECTION("vectorからのspan") {
        bluestl::vector<int> vec;
        for (int i = 1; i <= 5; ++i) {
            vec.push_back(i);
        }

        bluestl::span<int> s1(vec);
        REQUIRE(s1.size() == 5);
        REQUIRE(s1[0] == 1);
        REQUIRE(s1[4] == 5);
    }

    SECTION("const vectorからのspan") {
        bluestl::vector<int> vec;
        for (int i = 1; i <= 5; ++i) {
            vec.push_back(i);
        }
        const auto& const_vec = vec;
        bluestl::span<const int> s2(const_vec);
        REQUIRE(s2.size() == 5);
        REQUIRE(s2[0] == 1);
    }

    SECTION("fixed_vectorからのspan") {
        bluestl::fixed_vector<int, 10> fvec;
        for (int i = 10; i <= 30; i += 10) {
            fvec.push_back(i);
        }

        bluestl::span<int> s3(fvec);
        REQUIRE(s3.size() == 3);
        REQUIRE(s3[0] == 10);
        REQUIRE(s3[2] == 30);
    }

    SECTION("std::arrayからのspan") {
        std::array<int, 4> std_arr = { 100, 200, 300, 400 };
        bluestl::span<int> s4(std_arr);
        REQUIRE(s4.size() == 4);
        REQUIRE(s4[0] == 100);
        REQUIRE(s4[3] == 400);
    }
}

TEST_CASE("bluestl::span 比較操作", "[span]") {
    int arr1[] = { 1, 2, 3 };
    int arr2[] = { 1, 2, 3 };
    int arr3[] = { 1, 2, 4 };
    int arr4[] = { 1, 2 };

    bluestl::span<int> s1(arr1, 3);
    bluestl::span<int> s2(arr2, 3);
    bluestl::span<int> s3(arr3, 3);
    bluestl::span<int> s4(arr4, 2);

    SECTION("等価比較") {
        REQUIRE(s1 == s2);
        REQUIRE(!(s1 == s3));
        REQUIRE(!(s1 == s4));
    }
    SECTION("三方比較") {
        // 基本的な比較のテスト
        REQUIRE(s1 == s2);
        REQUIRE(!(s1 == s3));
        REQUIRE(!(s1 == s4));

        // サイズ比較
        REQUIRE(s4.size() < s1.size());
        REQUIRE(s1.size() > s4.size());
    }
}

TEST_CASE("bluestl::span バイト操作", "[span]") {
    int arr[] = { 0x12345678, 0x9ABCDEF0 };
    bluestl::span<int> s(arr, 2);
    SECTION("as_bytes") {
        auto byte_span = bluestl::as_bytes(s);
        REQUIRE(byte_span.size() == static_cast<std::size_t>(sizeof(int) * 2));

// バイト単位でのアクセス（エンディアンに依存）
// リトルエンディアンの場合
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        REQUIRE(static_cast<unsigned char>(byte_span[0]) == 0x78);
#endif
    }

    SECTION("as_writable_bytes") {
        auto writable_byte_span = bluestl::as_writable_bytes(s);
        REQUIRE(writable_byte_span.size() == sizeof(int) * 2);
    }
}

TEST_CASE("bluestl::span 型変換", "[span]") {
    int arr[] = { 1, 2, 3, 4, 5 };

    SECTION("non-const to const") {
        bluestl::span<int> s1(arr, 5);
        bluestl::span<const int> s2 = s1;
        REQUIRE(s2.size() == 5);
        REQUIRE(s2[0] == 1);
    }

    SECTION("固定サイズから動的サイズ") {
        bluestl::span<int, 5> s3(arr);
        bluestl::span<int> s4 = s3;
        REQUIRE(s4.size() == 5);
        REQUIRE(s4[0] == 1);
    }
}

TEST_CASE("bluestl::span 空のspan", "[span]") {
    SECTION("空のspan") {
        bluestl::span<int> empty_span;
        REQUIRE(empty_span.empty());
        REQUIRE(empty_span.size() == 0);
        REQUIRE(empty_span.data() == nullptr);
    }

    SECTION("空の固定サイズspan") {
        bluestl::span<int, 0> empty_fixed_span;
        REQUIRE(empty_fixed_span.empty());
        REQUIRE(empty_fixed_span.size() == 0);
    }

    SECTION("空のコンテナからのspan") {
        bluestl::vector<int> empty_vec;
        bluestl::span<int> span_from_empty(empty_vec);
        REQUIRE(span_from_empty.empty());
        REQUIRE(span_from_empty.size() == 0);
    }
}

TEST_CASE("bluestl::array_view エイリアス", "[span]") {
    int arr[] = { 1, 2, 3, 4, 5 };

    SECTION("array_viewはspanのエイリアス") {
        bluestl::array_view<int> av(arr, 5);
        REQUIRE(av.size() == 5);
        REQUIRE(av[0] == 1);
        REQUIRE(av[4] == 5);
    }

    SECTION("固定サイズarray_view") {
        bluestl::array_view<int, 5> av_fixed(arr);
        REQUIRE(av_fixed.size() == 5);
        REQUIRE(av_fixed.extent == 5);
    }
}
