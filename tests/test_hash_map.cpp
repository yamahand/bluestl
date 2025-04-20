#include "../include/stl/hash_map.h"
#include "../include/stl/fixed_hash_map.h"
#include <cassert>

#include "test_allocator.h"

using namespace bluestl;

// ハッシュマップのテスト
void test_fixed_hash_map() {
    fixed_hash_map<int, int, 8> m;
    assert(m.empty());
    m.insert(1, 100);
    m.insert(2, 200);
    assert(m.size() == 2);
    assert(m[1] == 100);
    assert(m[2] == 200);
    m[1] = 111;
    assert(m[1] == 111);
    assert(m.find(2)->second == 200);
    m.erase(1);
    assert(m.find(1) == nullptr);
    m.clear();
    assert(m.size() == 0);
}

void test_hash_map() {
	auto alloc = TestAllocator("test_hash_map");
    hash_map<int, int> m(alloc);
    assert(m.empty());
    m.insert(1, 100);
    m.insert(2, 200);
    assert(m.size() == 2);
    assert(m[1] == 100);
    assert(m[2] == 200);
    m[1] = 111;
    assert(m[1] == 111);
    assert(m.find(2)->second == 200);
    m.erase(1);
    assert(m.find(1) == nullptr);
    m.clear();
    assert(m.size() == 0);
    // 大量挿入でリハッシュも確認
    for(int i=0;i<100;++i) m.insert(i, i*10);
    for(int i=0;i<100;++i) assert(m[i] == i*10);
}
