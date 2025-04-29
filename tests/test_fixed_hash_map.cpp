/**
 * @file test_fixed_hash_map.cpp
 * @brief bluestl::fixed_hash_map の総合テスト
 *
 * - C++20準拠
 * - Doxygen形式コメント
 * - Catch2を利用
 * - test_main.cppから実行される
 */
#include <catch2/catch_test_macros.hpp>
#include "bluestl/fixed_hash_map.h"
#include <string>

using namespace bluestl;

/**
 * @brief fixed_hash_mapの基本操作テスト
 */
TEST_CASE("fixed_hash_map の基本操作", "[fixed_hash_map]") {
    fixed_hash_map<int, std::string, 8> m;
    SECTION("insertとfind") {
        auto [it1, ok1] = m.insert(1, "one");
        REQUIRE(ok1);
        REQUIRE(it1 != m.end());
        REQUIRE(it1->first == 1);
        REQUIRE(it1->second == "one");
        auto it2 = m.find(1);
        REQUIRE(it2 != m.end());
        REQUIRE(it2->second == "one");
        auto it3 = m.find(2);
        REQUIRE(it3 == m.end());
    }
    SECTION("operator[]とat") {
        m[2] = "two";
        REQUIRE(m[2] == "two");
        REQUIRE(m.at(2) == "two");
        REQUIRE(m.at(999) == ""); // dummy値
    }
    SECTION("try_get/contains") {
        m.insert(3, "three");
        auto opt = m.try_get(3);
        REQUIRE(opt.has_value());
        REQUIRE(*opt == "three");
        REQUIRE(m.contains(3));
        REQUIRE_FALSE(m.contains(999));
    }
    SECTION("erase(key)とclear") {
        m.insert(4, "four");
        m.insert(5, "five");
        auto it = m.find(4);
        REQUIRE(it != m.end());
        // 擬似的なerase(key)（イテレータで実現）
        m.erase(m.find(4));
        REQUIRE(m.find(4) == m.end());
        m.clear();
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
    }
    SECTION("empty/size/capacity") {
        REQUIRE(m.empty());
        m.insert(6, "six");
        REQUIRE_FALSE(m.empty());
        REQUIRE(m.size() == 1);
        REQUIRE(m.capacity() == 8);
    }

    SECTION("emplaceの動作") {
        fixed_hash_map<int, std::string, 8> m;
        auto [it, ok] = m.emplace(10, "ten");
        REQUIRE(ok);
        REQUIRE(it->first == 10);
        REQUIRE(it->second == "ten");
        auto [it2, ok2] = m.emplace(10, "xxx");
        REQUIRE_FALSE(ok2);
        REQUIRE(it2 == m.find(10));
    }
    SECTION("swapの動作") {
        fixed_hash_map<int, std::string, 8> m1, m2;
        m1.insert(1, "a");
        m2.insert(2, "b");
        m1.swap(m2);
        REQUIRE(m1.size() == 1);
        REQUIRE(m1.begin()->first == 2);
        REQUIRE(m2.size() == 1);
        REQUIRE(m2.begin()->first == 1);
    }
    SECTION("比較演算子の動作") {
        fixed_hash_map<int, std::string, 8> m1, m2;
        m1.insert(1, "a");
        m2.insert(1, "a");
        REQUIRE(m1 == m2);
        m2.insert(2, "b");
        REQUIRE(m1 != m2);
        REQUIRE(m1 < m2);
        REQUIRE(m2 > m1);
        REQUIRE(m1 <= m2);
        REQUIRE(m2 >= m1);
    }
    SECTION("erase(iterator)の動作") {
        fixed_hash_map<int, std::string, 8> m;
        m.insert(1, "a");
        m.insert(2, "b");
        auto it = m.find(1);
        it = m.erase(it);
        REQUIRE(m.find(1) == m.end());
        REQUIRE(it == m.find(2));
    }
    SECTION("構造化束縛の動作") {
        fixed_hash_map<int, std::string, 8> m;
        m.insert(100, "x");
        // value_type（pair<const Key, T>）での構造化束縛を推奨
        for (const auto& [k, v] : m) {
            REQUIRE(k == 100);
            REQUIRE(v == "x");
        }
        // イテレータのデリファレンスでもpair型なので分解可能
        auto it = m.begin();
        auto [k, v] = *it;
        REQUIRE(k == 100);
        REQUIRE(v == "x");
    }
    SECTION("イテレータでの範囲for") {
        fixed_hash_map<int, std::string, 8> m;
        m.insert(1, "a");
        m.insert(2, "b");
        int count = 0;
        for (auto it = m.begin(); it != m.end(); ++it) {
            ++count;
            REQUIRE((it->first == 1 || it->first == 2));
        }
        REQUIRE(count == 2);
    }
    SECTION("constイテレータの動作") {
        fixed_hash_map<int, std::string, 8> m;
        m.insert(1, "a");
        m.insert(2, "b");
        const auto& cm = m;
        int count = 0;
        for (auto it = cm.cbegin(); it != cm.cend(); ++it) {
            ++count;
            REQUIRE((it->first == 1 || it->first == 2));
        }
        REQUIRE(count == 2);
    }
}
