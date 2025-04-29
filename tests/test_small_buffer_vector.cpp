// -----------------------------------------------------------------------------
// Bluestl test_small_buffer_vector.cpp
// small_buffer_vector の動作確認テスト
// -----------------------------------------------------------------------------
/**
 * @file test_small_buffer_vector.cpp
 * @brief bluestl::small_buffer_vector のテストコード
 *
 * - C++20準拠
 * - Catch2を利用
 * - test_main.cppから実行される
 */
#include <catch2/catch_test_macros.hpp>
#include "bluestl/small_buffer_vector.h"
#include "test_allocator.h"
#include <string>
#include <utility>

TEST_CASE("small_buffer_vector 基本操作", "[small_buffer_vector]") {
    using bluestl::small_buffer_vector;
    TestAllocator alloc("test_small_buffer_vector");
    small_buffer_vector<int, 4> vec(alloc);
    REQUIRE(vec.size() == 0);
    REQUIRE(vec.capacity() == 4);
    REQUIRE(vec.empty());

    SECTION("push_back/emplace_back/at/[]") {
        vec.push_back(1);
        vec.push_back(2);
        vec.emplace_back(3);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec.at(2) == 3);
    }

    SECTION("pop_back/clear") {
        vec.push_back(10);
        vec.push_back(20);
        vec.pop_back();
        REQUIRE(vec.size() == 1);
        vec.clear();
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.empty());
    }
}

TEST_CASE("small_buffer_vector 固定バッファ超過時の動作", "[small_buffer_vector]") {
    using bluestl::small_buffer_vector;
    TestAllocator alloc("test_small_buffer_vector");
    small_buffer_vector<int, 2> vec(alloc);
    vec.push_back(1);
    vec.push_back(2);
    REQUIRE(vec.capacity() == 2);
    vec.push_back(3);  // ここでヒープ確保
    REQUIRE(vec.size() == 3);
    REQUIRE(vec.capacity() > 2);
    REQUIRE(vec[2] == 3);
    vec.shrink_to_fit();
    REQUIRE((vec.capacity() == 3 || vec.capacity() == 2));  // shrink_to_fitでスタックに戻るかヒープ縮小
}

TEST_CASE("small_buffer_vector swap/shrink_to_fit", "[small_buffer_vector]") {
    using bluestl::small_buffer_vector;
    TestAllocator alloc1("test_small_buffer_vector1");
    small_buffer_vector<std::string, 2> v1(alloc1);
    small_buffer_vector<std::string, 2> v2(alloc1);
    v1.push_back("a");
    v1.push_back("b");
    v2.push_back("x");
    v2.push_back("y");
    v2.push_back("z");  // v2はヒープ確保
    v1.swap(v2);
    REQUIRE(v1.size() == 3);
    REQUIRE(v1[0] == "x");
    REQUIRE(v2.size() == 2);
    REQUIRE(v2[1] == "b");
    v1.shrink_to_fit();
    v2.shrink_to_fit();
    REQUIRE((v1.capacity() == 3 || v1.capacity() == 2));
    REQUIRE(v2.capacity() == 2);
}
