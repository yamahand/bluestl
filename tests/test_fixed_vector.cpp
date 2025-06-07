#include <catch2/catch_test_macros.hpp>
#include "bluestl/fixed_vector.h"
#include <string>
#include <utility>

TEST_CASE("fixed_vector の基本・拡張機能", "[vector][fixed]") {
    using bluestl::fixed_vector;

    SECTION("初期状態") {
        fixed_vector<int, 5> vec;
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.capacity() == 5);
        REQUIRE(vec.empty());
    }

    SECTION("要素の追加(push_back, ムーブ, emplace_back)") {
        fixed_vector<std::string, 3> v;
        std::string s = "abc";
        REQUIRE(v.push_back(s));             // コピー
        REQUIRE(v.push_back(std::move(s)));  // ムーブ
        REQUIRE(v.emplace_back(3, 'x'));
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == "abc");
        REQUIRE(v[1] == "abc");
        REQUIRE(v[2] == "xxx");
        REQUIRE_FALSE(v.push_back("overflow"));  // 容量超過
    }

    SECTION("front/back/atの正常系") {
        fixed_vector<int, 2> v;
        v.push_back(10);
        v.push_back(20);
        REQUIRE(v.front() == 10);
        REQUIRE(v.back() == 20);
        REQUIRE(v.at(0) == 10);
        REQUIRE(v.at(1) == 20);
    }

    // 範囲外アクセスのアサートは直接テストできないためコメントで注意
    SECTION("front/back/atの範囲外アクセスはアサート") {
        fixed_vector<int, 1> v;
        //[[maybe_unused]] auto f = v.front(); // アサート発生
        //[[maybe_unused]] auto b = v.back();  // アサート発生
        //[[maybe_unused]] auto a = v.at(1);   // アサート発生
    }

    SECTION("イテレータ・リバースイテレータ") {
        fixed_vector<int, 3> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto it = v.begin();
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        auto rit = v.rbegin();
        REQUIRE(*rit == 3);
        ++rit;
        REQUIRE(*rit == 2);
        ++rit;
        REQUIRE(*rit == 1);
        ++rit;
        REQUIRE(rit == v.rend());
    }

    SECTION("要素の削除・クリア") {
        fixed_vector<int, 3> v;
        v.push_back(1);
        v.push_back(2);
        v.pop_back();
        REQUIRE(v.size() == 1);
        v.clear();
        REQUIRE(v.size() == 0);
        v.pop_back();  // 空でもクラッシュしない
        REQUIRE(v.size() == 0);
    }

    SECTION("assign操作") {
        fixed_vector<int, 5> v;
        v.push_back(1);
        v.push_back(2);
        v.assign(4, 7);
        REQUIRE(v.size() == 4);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(v[i] == 7);
        }
    }

    SECTION("insert/erase操作") {
        fixed_vector<int, 5> v;
        v.assign(4, 7);
        auto it = v.insert(v.begin() + 2, 42);
        REQUIRE(v.size() == 5);
        REQUIRE(v[2] == 42);
        it = v.erase(v.begin() + 1);
        REQUIRE(v.size() == 4);
        REQUIRE(v[1] == 42);
    }

    SECTION("swap操作") {
        fixed_vector<int, 5> v1;
        v1.assign(4, 7);
        v1[1] = 42;
        fixed_vector<int, 5> v2;
        v2.push_back(100);
        v2.push_back(200);
        v1.swap(v2);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1[0] == 100);
        REQUIRE(v1[1] == 200);
        REQUIRE(v2.size() == 4);
        REQUIRE(v2[1] == 42);
    }

    SECTION("比較演算子") {
        fixed_vector<int, 5> v1 = { 100, 200 };
        fixed_vector<int, 5> v3 = { 100, 200 };
        REQUIRE(v1 == v3);
        REQUIRE_FALSE(v1 != v3);
        v3.push_back(300);
        REQUIRE(v1 < v3);
        REQUIRE(v3 > v1);
        REQUIRE(v1 <= v3);
        REQUIRE(v3 >= v1);
    }

    SECTION("コピー・ムーブセマンティクス") {
        fixed_vector<int, 5> v1 = { 100, 200 };
        fixed_vector<int, 5> v4(v1);
        REQUIRE(v4 == v1);
        fixed_vector<int, 5> v5(std::move(v4));
        REQUIRE(v5 == v1);
        fixed_vector<int, 5> v6;
        v6 = v1;
        REQUIRE(v6 == v1);
        fixed_vector<int, 5> v7;
        v7 = std::move(v6);
        REQUIRE(v7 == v1);
    }

    SECTION("イニシャライザリスト") {
        fixed_vector<int, 5> v8{ 1, 2, 3, 4, 5 };
        REQUIRE(v8.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(v8[i] == i + 1);
        }
    }

    SECTION("ムーブ専用型でのpush_back(T&&)テスト") {
        struct MoveOnly {
            int v;
            MoveOnly(int x) : v(x) {}
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly& operator=(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&& o) noexcept : v(o.v) {
                o.v = -1;
            }
            MoveOnly& operator=(MoveOnly&& o) noexcept {
                v = o.v;
                o.v = -1;
                return *this;
            }
        };
        fixed_vector<MoveOnly, 2> v;
        v.emplace_back(1);
        v.push_back(MoveOnly(2));
        REQUIRE(v.size() == 2);
        REQUIRE(v[0].v == 1);
        REQUIRE(v[1].v == 2);
    }

    SECTION("境界チェック強化テスト") {
        fixed_vector<int, 5> v;
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);

        // 正常なアクセス
        CHECK(v.at(0) == 10);
        CHECK(v.at(1) == 20);
        CHECK(v.at(2) == 30);

        // 範囲外アクセスは本来アサーションが発生するが、テストでは確認できない
        // 代わりに、有効な範囲のテストのみ実行

        // front/backの正常動作
        CHECK(v.front() == 10);
        CHECK(v.back() == 30);

        // 空のベクターでfront/backを呼ぶとアサーション発生（テストでは確認できない）
        fixed_vector<int, 5> empty_v;
        CHECK(empty_v.empty() == true);
        CHECK(empty_v.size() == 0);
    }

    SECTION("メモリ効率とアライメントテスト") {
        // 固定サイズベクターのメモリ使用量確認
        fixed_vector<int, 10> v;

        // 要素を追加していても capacity は固定
        CHECK(v.capacity() == 10);

        for (int i = 0; i < 5; ++i) {
            v.push_back(i * 2);
        }

        CHECK(v.size() == 5);
        CHECK(v.capacity() == 10); // 容量は変わらない

        // データポインタが適切にアライメントされていることを確認
        auto* data_ptr = v.data();
        CHECK(data_ptr != nullptr);
        CHECK(reinterpret_cast<uintptr_t>(data_ptr) % alignof(int) == 0);
    }

    SECTION("容量超過時の動作確認") {
        fixed_vector<int, 3> v;

        // 容量以内は正常に動作
        CHECK(v.push_back(1) == true);
        CHECK(v.push_back(2) == true);
        CHECK(v.push_back(3) == true);
        CHECK(v.size() == 3);

        // 容量超過は失敗
        CHECK(v.push_back(4) == false);
        CHECK(v.size() == 3); // サイズは変わらない

        // emplace_backでも同様
        CHECK(v.emplace_back(5) == false);
        CHECK(v.size() == 3);
    }
}
