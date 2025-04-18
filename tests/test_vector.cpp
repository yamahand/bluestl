#include "../include/stl/vector.h"
#include <iostream>
#include <cassert>

class TestVectorAllocator : public bluestl::allocator {
public:
    TestVectorAllocator() : bluestl::allocator("TestVectorAllocator") {}

    void* allocate(size_t n) override {
        return ::operator new(n);
    }

    void deallocate(void* p, size_t) override {
        ::operator delete(p);
    }
};

void test_vector() {
    auto allocator = TestVectorAllocator();
    bluestl::vector<int, TestVectorAllocator> vec(allocator);

    // 初期状態のテスト
    assert(vec.size() == 0);
    assert(vec.capacity() == 0);

    // 要素の追加
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    assert(vec.size() == 3);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);

    // 容量の確認
    assert(vec.capacity() >= vec.size());

    // 要素の削除
    vec.pop_back();
    assert(vec.size() == 2);
    assert(vec[0] == 1);
    assert(vec[1] == 2);

    // クリアのテスト
    vec.clear();
    assert(vec.size() == 0);

    // reserveのテスト
    vec.reserve(10);
    assert(vec.capacity() >= 10);

    // 再度要素を追加
    vec.push_back(4);
    vec.push_back(5);
    assert(vec.size() == 2);
    assert(vec[0] == 4);
    assert(vec[1] == 5);

    // イテレータのテスト
    vec.clear();
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    int sum = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    assert(sum == 60);

    // constイテレータのテスト
    const auto& cvec = vec;
    int csum = 0;
    for (auto it = cvec.cbegin(); it != cvec.cend(); ++it) {
        csum += *it;
    }
    assert(csum == 60);

    // リバースイテレータのテスト
    int rsum = 0;
    for (auto rit = vec.rbegin(); rit != vec.rend(); ++rit) {
        rsum += *rit;
    }
    assert(rsum == 60);

    // constリバースイテレータのテスト
    int crsum = 0;
    for (auto rit = cvec.crbegin(); rit != cvec.crend(); ++rit) {
        crsum += *rit;
    }
    assert(crsum == 60);

    // front/back/empty/atのテスト
    vec.clear();
    assert(vec.empty());
    vec.push_back(100);
    vec.push_back(200);
    vec.push_back(300);
    assert(vec.front() == 100);
    assert(vec.back() == 300);
    assert(vec.at(1) == 200);
    vec.at(1) = 250;
    assert(vec[1] == 250);

    // resizeのテスト
    vec.resize(5, 42);
    assert(vec.size() == 5);
    assert(vec[3] == 42);
    assert(vec[4] == 42);
    vec.resize(2);
    assert(vec.size() == 2);
    assert(vec.back() == 250);

    // swapのテスト
    bluestl::vector<int, TestVectorAllocator> vec2(allocator);
    vec2.push_back(999);
    vec.swap(vec2);
    assert(vec.size() == 1);
    assert(vec.front() == 999);
    assert(vec2.size() == 2);
    assert(vec2.front() == 100);
    assert(vec2.back() == 250);

    // assignのテスト
    vec.assign(3, 7);
    assert(vec.size() == 3);
    assert(vec[0] == 7 && vec[1] == 7 && vec[2] == 7);

    // insertのテスト
    auto it = vec.insert(vec.begin() + 1, 42);
    assert(vec.size() == 4);
    assert(vec[1] == 42);
    assert(*it == 42);

    // eraseのテスト
    it = vec.erase(vec.begin() + 2);
    assert(vec.size() == 3);
    assert(vec[2] == 7);
    assert(*it == 7);


    std::cout << "All vector tests passed!" << std::endl;
}
