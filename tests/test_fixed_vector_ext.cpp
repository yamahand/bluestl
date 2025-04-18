#include "../include/stl/fixed_vector.h"
#include <cassert>
#include <iostream>

void test_fixed_vector_ext() {
    using bluestl::fixed_vector;
    // デフォルトコンストラクタ・push_back・front/back/empty
    fixed_vector<int, 5> v1;
    assert(v1.empty());
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(3);
    assert(v1.size() == 3);
    assert(v1.front() == 1);
    assert(v1.back() == 3);
    
    // assign
    v1.assign(4, 7);
    assert(v1.size() == 4);
    for (size_t i = 0; i < v1.size(); ++i) assert(v1[i] == 7);

    // insert/erase
    auto it = v1.insert(v1.begin() + 2, 42);
    assert(v1.size() == 5);
    assert(v1[2] == 42);
    it = v1.erase(v1.begin() + 1);
    assert(v1.size() == 4);
    assert(v1[1] == 42);
    
    // swap
    fixed_vector<int, 5> v2;
    v2.push_back(100);
    v2.push_back(200);
    v1.swap(v2);
    assert(v1.size() == 2 && v1[0] == 100 && v1[1] == 200);
    assert(v2.size() == 4 && v2[1] == 42);

    // 比較演算子
    fixed_vector<int, 5> v3 = {100, 200};
    assert(v1 == v3);
    assert(!(v1 != v3));
    v3.push_back(300);
    assert(v1 < v3);
    assert(v3 > v1);
    assert(v1 <= v3);
    assert(v3 >= v1);

    // コピー・ムーブ
    fixed_vector<int, 5> v4(v1);
    assert(v4 == v1);
    fixed_vector<int, 5> v5(std::move(v4));
    assert(v5 == v1);
    fixed_vector<int, 5> v6;
    v6 = v1;
    assert(v6 == v1);
    fixed_vector<int, 5> v7;
    v7 = std::move(v6);
    assert(v7 == v1);

    // イニシャライザリスト
    fixed_vector<int, 5> v8{1,2,3,4,5};
    assert(v8.size() == 5);
    for (int i = 0; i < 5; ++i) assert(v8[i] == i+1);

    std::cout << "All fixed_vector extension tests passed!" << std::endl;
}
