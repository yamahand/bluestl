#include <catch2/catch_test_macros.hpp>
#include "bluestl/fixed_hash_map.h"

using namespace bluestl;

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

TEST_CASE("fixed_hash_map の try_get/contains/at のテスト", "[hash_map][fixed]") {
    fixed_hash_map<int, std::string, 8> m;
    m.insert(1, "one");
    m.insert(2, "two");

    SECTION("try_getで値が取得できる場合") {
        auto opt = m.try_get(1);
        REQUIRE(opt.has_value());
        REQUIRE(*opt == "one");
        *opt = "ONE";
        REQUIRE(m[1] == "ONE");
    }
    SECTION("try_getで値が取得できない場合") {
        auto opt = m.try_get(3);
        REQUIRE_FALSE(opt.has_value());
    }
    SECTION("const fixed_hash_mapでのtry_get") {
        const auto& cm = m;
        auto opt = cm.try_get(2);
        REQUIRE(opt.has_value());
        REQUIRE(*opt == "two");
    }
    SECTION("containsの動作確認") {
        REQUIRE(m.contains(1));
        REQUIRE(m.contains(2));
        REQUIRE_FALSE(m.contains(3));
    }
    SECTION("at(const)の動作確認") {
        const auto& cm = m;
        REQUIRE(cm.at(1) == "one");
        REQUIRE(cm.at(2) == "two");
        REQUIRE(cm.at(999) == ""); // dummy値
    }
}
