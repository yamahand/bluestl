#include "../include/stl/hash_fnv1a.h"
#include "../include/stl/hash_xx.h"
#include "../include/stl/hash_murmur.h"
#include "../include/stl/hash.h"
#include "../include/stl/hash_map.h"
#include "../include/stl/fixed_hash_map.h"
#include <cassert>
#include <string>

using namespace bluestl;

void test_hash_functions2() {
    int a = 42;
    int b = 43;
    assert(hash_fnv1a(a) != hash_fnv1a(b));
    assert(hash_xx(a) != hash_xx(b));
    assert(hash_murmur(a) != hash_murmur(b));
    assert(hash_fnv1a64(a) != hash_fnv1a64(b));
    assert(hash_xx64(a) != hash_xx64(b));
    assert(hash_murmur64(a) != hash_murmur64(b));
    std::string s1 = "abc", s2 = "def";
    assert(hash_fnv1a(s1) != hash_fnv1a(s2));
    assert(hash_xx(s1) != hash_xx(s2));
    assert(hash_murmur(s1) != hash_murmur(s2));
}
