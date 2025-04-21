#include <catch2/catch_test_macros.hpp>
#include "../include/stl/hash_fnv1a.h"
#include "../include/stl/hash_xx.h"
#include "../include/stl/hash_murmur.h"
#include "../include/stl/hash.h"
#include <string>

using namespace bluestl;

TEST_CASE("ハッシュ関数テスト - 整数値", "[hash][core]") {
    int a = 42;
    int b = 43;
    
    SECTION("FNV1Aハッシュ") {
        REQUIRE(hash_fnv1a(a) != hash_fnv1a(b));
        REQUIRE(hash_fnv1a64(a) != hash_fnv1a64(b));
    }
    
    SECTION("xxハッシュ") {
        REQUIRE(hash_xx(a) != hash_xx(b));
        REQUIRE(hash_xx64(a) != hash_xx64(b));
    }
    
    SECTION("Murmurhハッシュ") {
        REQUIRE(hash_murmur(a) != hash_murmur(b));
        REQUIRE(hash_murmur64(a) != hash_murmur64(b));
    }
}

TEST_CASE("ハッシュ関数テスト - 文字列", "[hash][core][string]") {
    std::string s1 = "abc";
    std::string s2 = "def";
    
    SECTION("文字列のハッシュ計算") {
        REQUIRE(hash_fnv1a(s1) != hash_fnv1a(s2));
        REQUIRE(hash_xx(s1) != hash_xx(s2));
        REQUIRE(hash_murmur(s1) != hash_murmur(s2));
    }
}
