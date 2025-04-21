#include <catch2/catch_test_macros.hpp>
#include "../include/stl/fixed_vector.h"

TEST_CASE("fixed_vector 拡張機能テスト", "[vector][fixed][ext]") {
    using bluestl::fixed_vector;
    
    SECTION("基本操作 (push_back・front/back/empty)") {
        fixed_vector<int, 5> v1;
        REQUIRE(v1.empty());
        
        v1.push_back(1);
        v1.push_back(2);
        v1.push_back(3);
        
        REQUIRE(v1.size() == 3);
        REQUIRE(v1.front() == 1);
        REQUIRE(v1.back() == 3);
    }
    
    SECTION("assign操作") {
        fixed_vector<int, 5> v1;
        v1.push_back(1);
        v1.push_back(2);
        
        v1.assign(4, 7);
        REQUIRE(v1.size() == 4);
        
        for (size_t i = 0; i < v1.size(); ++i) {
            REQUIRE(v1[i] == 7);
        }
    }

    SECTION("insert/erase操作") {
        fixed_vector<int, 5> v1;
        v1.assign(4, 7);
        
        auto it = v1.insert(v1.begin() + 2, 42);
        REQUIRE(v1.size() == 5);
        REQUIRE(v1[2] == 42);
        
        it = v1.erase(v1.begin() + 1);
        REQUIRE(v1.size() == 4);
        REQUIRE(v1[1] == 42);    }
    
    SECTION("swap操作") {
        fixed_vector<int, 5> v1;
        v1.assign(4, 7);
        v1[1] = 42;
        
        fixed_vector<int, 5> v2;
        v2.push_back(100);
        v2.push_back(200);
        
        v1.swap(v2);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1[0] == 100);
        REQUIRE(v1[1] == 200);
        REQUIRE(v2.size() == 4);
        REQUIRE(v2[1] == 42);
    }

    SECTION("比較演算子") {
        fixed_vector<int, 5> v1 = {100, 200};
        fixed_vector<int, 5> v3 = {100, 200};
        
        REQUIRE(v1 == v3);
        REQUIRE_FALSE(v1 != v3);
        
        v3.push_back(300);
        REQUIRE(v1 < v3);
        REQUIRE(v3 > v1);
        REQUIRE(v1 <= v3);
        REQUIRE(v3 >= v1);
    }

    SECTION("コピー・ムーブセマンティクス") {
        fixed_vector<int, 5> v1 = {100, 200};
        
        // コピーコンストラクタ
        fixed_vector<int, 5> v4(v1);
        REQUIRE(v4 == v1);
        
        // ムーブコンストラクタ
        fixed_vector<int, 5> v5(std::move(v4));
        REQUIRE(v5 == v1);
        
        // コピー代入
        fixed_vector<int, 5> v6;
        v6 = v1;
        REQUIRE(v6 == v1);
        
        // ムーブ代入
        fixed_vector<int, 5> v7;
        v7 = std::move(v6);
        REQUIRE(v7 == v1);
    }

    SECTION("イニシャライザリスト") {
        fixed_vector<int, 5> v8{1, 2, 3, 4, 5};
        REQUIRE(v8.size() == 5);
        
        for (int i = 0; i < 5; ++i) {
            REQUIRE(v8[i] == i+1);
        }
    }
}
