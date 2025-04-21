#include <catch2/catch_test_macros.hpp>
#include "../include/stl/fixed_vector.h"

TEST_CASE("fixed_vector の基本操作", "[vector][fixed]") {
    bluestl::fixed_vector<int, 5> vec;

    SECTION("初期状態") {
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.capacity() == 5);
    }

    SECTION("要素の追加") {
        REQUIRE(vec.push_back(1));
        REQUIRE(vec.push_back(2));
        REQUIRE(vec.push_back(3));
        REQUIRE(vec.size() == 3);
    }

    SECTION("イテレータ") {
        vec.push_back(1);
        vec.push_back(2);
        
        auto it = vec.begin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
    }

    SECTION("要素の削除") {
        vec.push_back(1);
        vec.push_back(2);
        
        vec.pop_back();
        REQUIRE(vec.size() == 1);
        REQUIRE(vec.end()[-1] == 1);
    }    
    
    SECTION("容量オーバーのテスト") {
        REQUIRE(vec.push_back(1));
        REQUIRE(vec.push_back(2));
        REQUIRE(vec.push_back(3));
        REQUIRE(vec.push_back(4));
        REQUIRE(vec.push_back(5));
        REQUIRE_FALSE(vec.push_back(6)); // 容量を超える
    }

    SECTION("空の状態でのpop_back") {
        vec.clear(); // 全ての要素を削除
        vec.pop_back(); // 何も起こらないがクラッシュしない
        REQUIRE(vec.size() == 0);
    }

    SECTION("atメソッド") {
        vec.push_back(10);
        vec.push_back(20);
        REQUIRE(vec.at(0) == 10);
        REQUIRE(vec.at(1) == 20);
    }

    SECTION("コピーコンストラクタ") {
        vec.push_back(10);
        vec.push_back(20);
        
        bluestl::fixed_vector<int, 5> vec_copy = vec;
        REQUIRE(vec_copy.size() == vec.size());
        REQUIRE(vec_copy.at(0) == vec.at(0));
    }

    SECTION("代入演算子") {
        vec.push_back(10);
        vec.push_back(20);
        
        bluestl::fixed_vector<int, 5> vec_assign;
        vec_assign = vec;
        REQUIRE(vec_assign.size() == vec.size());
        REQUIRE(vec_assign.at(1) == vec.at(1));
    }

    SECTION("リバースイテレータ") {
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);

        auto rit = vec.rbegin();
        REQUIRE(*rit == 3);
        ++rit;
        REQUIRE(*rit == 2);
        ++rit;
        REQUIRE(*rit == 1);
        ++rit;
        REQUIRE(rit == vec.rend());
    }
}
