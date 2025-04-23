#include <catch2/catch_test_macros.hpp>
#include "bluestl/vector.h"
#include "test_allocator.h"

TEST_CASE("vector の基本操作", "[vector]") {
    TestAllocator allocator("test_vector");
    bluestl::vector<int> vec(allocator);

    SECTION("初期状態") {
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.capacity() == 0);
    }

    SECTION("要素の追加") {
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
        
        // 容量の確認
        REQUIRE(vec.capacity() >= vec.size());
    }

    SECTION("要素の削除") {
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        
        vec.pop_back();
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
    }

    SECTION("クリア操作") {
        vec.push_back(1);
        vec.push_back(2);
        
        vec.clear();
        REQUIRE(vec.size() == 0);
    }

    SECTION("容量確保") {
        vec.reserve(10);
        REQUIRE(vec.capacity() >= 10);
        
        // 要素追加が正常に行われるか
        vec.push_back(4);
        vec.push_back(5);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 4);
        REQUIRE(vec[1] == 5);    }

    SECTION("イテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);
        
        int sum = 0;
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            sum += *it;
        }
        REQUIRE(sum == 60);
    }

    SECTION("constイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);
        
        const auto& cvec = vec;
        int csum = 0;
        for (auto it = cvec.cbegin(); it != cvec.cend(); ++it) {
            csum += *it;
        }
        REQUIRE(csum == 60);
    }

    SECTION("リバースイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);
        
        int rsum = 0;
        for (auto rit = vec.rbegin(); rit != vec.rend(); ++rit) {
            rsum += *rit;
        }
        REQUIRE(rsum == 60);
    }

    SECTION("constリバースイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);
        
        const auto& cvec = vec;
        int crsum = 0;
        for (auto rit = cvec.crbegin(); rit != cvec.crend(); ++rit) {
            crsum += *rit;
        }
        REQUIRE(crsum == 60);
    }

    SECTION("front/back/empty/at") {
        vec.clear();
        REQUIRE(vec.empty());
        
        vec.push_back(100);
        vec.push_back(200);
        vec.push_back(300);
        
        REQUIRE(vec.front() == 100);
        REQUIRE(vec.back() == 300);
        REQUIRE(vec.at(1) == 200);
          vec.at(1) = 250;
        REQUIRE(vec.at(1) == 250);
        REQUIRE(vec[1] == 250);
    }

    SECTION("resizeのテスト") {
        vec.clear();
        vec.push_back(100);
        vec.push_back(250);
        
        vec.resize(5, 42);
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[3] == 42);
        REQUIRE(vec[4] == 42);
        
        vec.resize(2);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec.back() == 250);
    }

    SECTION("swapのテスト") {
        vec.clear();
        vec.push_back(100);
        vec.push_back(250);
        
        bluestl::vector<int> vec2(allocator);
        vec2.push_back(999);
        vec.swap(vec2);
        
        REQUIRE(vec.size() == 1);
        REQUIRE(vec.front() == 999);
        REQUIRE(vec2.size() == 2);
        REQUIRE(vec2.front() == 100);
        REQUIRE(vec2.back() == 250);
    }

    SECTION("assignのテスト") {
        vec.assign(3, 7);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 7);
        REQUIRE(vec[1] == 7);
        REQUIRE(vec[2] == 7);
    }
}
