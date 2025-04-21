#include <catch2/catch_test_macros.hpp>
#include "../include/stl/hash_fnv1a.h"
#include "../include/stl/hash_xx.h"
#include "../include/stl/hash_murmur.h"
#include "../include/stl/hash.h"
#include "../include/stl/hash_map.h"
#include "../include/stl/fixed_hash_map.h"
#include <string>

using namespace bluestl;

TEST_CASE("ハッシュ関数テスト2 - 異なる値に対して異なるハッシュ値を返す", "[hash][core]") {
    int a = 42;
    int b = 43;
    
    SECTION("整数値のハッシュ") {
        REQUIRE(hash_fnv1a(a) != hash_fnv1a(b));
        REQUIRE(hash_xx(a) != hash_xx(b));
        REQUIRE(hash_murmur(a) != hash_murmur(b));
        REQUIRE(hash_fnv1a64(a) != hash_fnv1a64(b));
        REQUIRE(hash_xx64(a) != hash_xx64(b));
        REQUIRE(hash_murmur64(a) != hash_murmur64(b));
    }
    
    SECTION("文字列のハッシュ") {
        std::string s1 = "abc";
        std::string s2 = "def";
        
        REQUIRE(hash_fnv1a(s1) != hash_fnv1a(s2));
        REQUIRE(hash_xx(s1) != hash_xx(s2));
        REQUIRE(hash_murmur(s1) != hash_murmur(s2));
    }
}
