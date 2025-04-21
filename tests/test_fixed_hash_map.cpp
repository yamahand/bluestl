#include <catch2/catch_test_macros.hpp>
#include "../include/stl/fixed_hash_map.h"

using namespace bluestl;

#if 0

TEST_CASE("fixed_hash_map の基本操作", "[hash_map][fixed]") {
    fixed_hash_map<int, int, 8> m;
    
    SECTION("初期状態") {
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
    }
    
    SECTION("要素の追加と取得") {
        m.insert(1, 100);
        m.insert(2, 200);
        
        REQUIRE(m.size() == 2);
        REQUIRE(m[1] == 100);
        REQUIRE(m[2] == 200);
    }
    
    SECTION("要素の更新") {
        m.insert(1, 100);
        m[1] = 111;
        
        REQUIRE(m[1] == 111);
    }
    
    SECTION("要素の検索") {
        m.insert(1, 100);
        m.insert(2, 200);
        
        REQUIRE(m.find(2)->second == 200);
        REQUIRE(m.find(3) == nullptr);
    }
    
    SECTION("要素の削除") {
        m.insert(1, 100);
        m.erase(1);
        
        REQUIRE(m.find(1) == nullptr);
    }
    
    SECTION("全要素の削除") {
        m.insert(1, 100);
        m.insert(2, 200);
        m.clear();
        
        REQUIRE(m.size() == 0);
        REQUIRE(m.empty());
    }
}

#endif
