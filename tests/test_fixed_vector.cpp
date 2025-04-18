#include "../include/stl/fixed_vector.h"
#include <iostream>
#include <cassert>

void test_fixed_vector() {
    bluestl::fixed_vector<int, 5> vec;

    // 初期状態のテスト
    assert(vec.size() == 0);
    assert(vec.capacity() == 5);

    // 要素の追加
    assert(vec.push_back(1));
    assert(vec.push_back(2));
    assert(vec.push_back(3));
    assert(vec.size() == 3);

    // イテレータのテスト
    auto it = vec.begin();
    assert(*it == 1);
    ++it;
    assert(*it == 2);

    // 要素の削除
    vec.pop_back();
    assert(vec.size() == 2);
    assert(vec.end()[-1] == 2);

    // 容量オーバーのテストの前に要素を追加して容量を満たす
    assert(vec.push_back(3));

    // 容量オーバーのテスト
    assert(vec.push_back(4));
    assert(vec.push_back(5));
    assert(!vec.push_back(6)); // 容量を超える

    // 空の状態でのpop_backのテスト
    vec.clear(); // 全ての要素を削除
    vec.pop_back(); // 何も起こらないがクラッシュしない
    assert(vec.size() == 0);

    // atメソッドのテスト
    vec.push_back(10);
    vec.push_back(20);
    assert(vec.at(0) == 10);
    assert(vec.at(1) == 20);

    // コピーコンストラクタのテスト
    bluestl::fixed_vector<int, 5> vec_copy = vec;
    assert(vec_copy.size() == vec.size());
    assert(vec_copy.at(0) == vec.at(0));

    // 代入演算子のテスト
    bluestl::fixed_vector<int, 5> vec_assign;
    vec_assign = vec;
    assert(vec_assign.size() == vec.size());
    assert(vec_assign.at(1) == vec.at(1));

    // リバースイテレータのテスト
    vec.clear();
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    auto rit = vec.rbegin();
    assert(*rit == 3);
    ++rit;
    assert(*rit == 2);
    ++rit;
    assert(*rit == 1);
    ++rit;
    assert(rit == vec.rend());

    std::cout << "All fixed_vector tests passed!" << std::endl;
}
