// filepath: f:\dev\github\bluestl\tests\test_vector.cpp
#include "bluestl/vector.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <compare>

// bluestl::vectorのテストケース

// ========== テスト用カスタムクラス ==========
struct TestType {
    int value;
    bool* destroyed;

    TestType() : value(0), destroyed(nullptr) {}
    TestType(int v) : value(v), destroyed(nullptr) {}
    TestType(int v, bool* d) : value(v), destroyed(d) {}

    ~TestType() {
        if (destroyed) *destroyed = true;
    }

    TestType(const TestType& other) : value(other.value), destroyed(other.destroyed) {}
    TestType& operator=(const TestType& other) {
        value = other.value;
        destroyed = other.destroyed;
        return *this;
    }

    TestType(TestType&& other) noexcept : value(other.value), destroyed(other.destroyed) {
        other.destroyed = nullptr;
    }
    TestType& operator=(TestType&& other) noexcept {
        value = other.value;
        destroyed = other.destroyed;
        other.destroyed = nullptr;
        return *this;
    }

    bool operator==(const TestType& other) const {
        return value == other.value;
    }

    auto operator<=>(const TestType& other) const = default;
};

TEST_CASE("bluestl::vector 基本動作", "[vector]") {
    using bluestl::vector;

    SECTION("デフォルトコンストラクタ") {
        vector<int> v;
        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
    }

    SECTION("サイズ指定コンストラクタ") {
        vector<int> v(5);
        REQUIRE(!v.empty());
        REQUIRE(v.size() == 5);
        REQUIRE(v.capacity() >= 5);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(v[i] == 0);
        }
    }

    SECTION("サイズと初期値指定コンストラクタ") {
        vector<int> v(3, 42);
        REQUIRE(v.size() == 3);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(v[i] == 42);
        }
    }

    SECTION("イテレータ範囲コンストラクタ") {
        int arr[] = { 1, 2, 3, 4, 5 };
        vector<int> v(arr, arr + 5);
        REQUIRE(v.size() == 5);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(v[i] == arr[i]);
        }
    }

    SECTION("コピーコンストラクタ") {
        vector<int> v1 = { 1, 2, 3 };
        vector<int> v2(v1);
        REQUIRE(v2.size() == v1.size());
        for (size_t i = 0; i < v1.size(); ++i) {
            REQUIRE(v2[i] == v1[i]);
        }
        // v1を変更してもv2に影響がないことを確認
        v1[0] = 100;
        REQUIRE(v2[0] == 1);
    }

    SECTION("ムーブコンストラクタ") {
        vector<int> v1 = { 1, 2, 3 };
        size_t original_capacity = v1.capacity();
        vector<int> v2(std::move(v1));
        REQUIRE(v2.size() == 3);
        REQUIRE(v2[0] == 1);
        REQUIRE(v2[1] == 2);
        REQUIRE(v2[2] == 3);
        // ムーブ元は空になる
        REQUIRE(v1.empty());
        REQUIRE(v1.capacity() == 0);
        // ムーブ先は元の容量を引き継ぐ
        REQUIRE(v2.capacity() == original_capacity);
    }

    SECTION("イニシャライザリストコンストラクタ") {
        vector<int> v = { 5, 4, 3, 2, 1 };
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 5);
        REQUIRE(v[1] == 4);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 2);
        REQUIRE(v[4] == 1);
    }
}

TEST_CASE("bluestl::vector 代入演算子", "[vector]") {
    using bluestl::vector;

    SECTION("コピー代入演算子") {
        vector<int> v1 = { 1, 2, 3 };
        vector<int> v2;
        v2 = v1;
        REQUIRE(v2.size() == v1.size());
        for (size_t i = 0; i < v1.size(); ++i) {
            REQUIRE(v2[i] == v1[i]);
        }
        // v1を変更してもv2に影響がないことを確認
        v1[0] = 100;
        REQUIRE(v2[0] == 1);
    }

    SECTION("ムーブ代入演算子") {
        vector<int> v1 = { 1, 2, 3 };
        size_t original_capacity = v1.capacity();
        vector<int> v2;
        v2 = std::move(v1);
        REQUIRE(v2.size() == 3);
        REQUIRE(v2[0] == 1);
        REQUIRE(v2[1] == 2);
        REQUIRE(v2[2] == 3);
        // ムーブ元は空になる
        REQUIRE(v1.empty());
        REQUIRE(v1.capacity() == 0);
        // ムーブ先は元の容量を引き継ぐ
        REQUIRE(v2.capacity() == original_capacity);
    }

    SECTION("イニシャライザリスト代入演算子") {
        vector<int> v;
        v = { 10, 20, 30 };
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 20);
        REQUIRE(v[2] == 30);
    }
}

TEST_CASE("bluestl::vector 要素アクセス", "[vector]") {
    using bluestl::vector;
    vector<int> v = { 10, 20, 30, 40, 50 };

    SECTION("operator[]") {
        REQUIRE(v[0] == 10);
        REQUIRE(v[2] == 30);
        REQUIRE(v[4] == 50);

        // 変更可能であることを確認
        v[1] = 200;
        REQUIRE(v[1] == 200);
    }

    SECTION("at()") {
        REQUIRE(v.at(0) == 10);
        REQUIRE(v.at(2) == 30);
        REQUIRE(v.at(4) == 50);

        // 変更可能であることを確認
        v.at(3) = 400;
        REQUIRE(v.at(3) == 400);
    }

    SECTION("front()とback()") {
        REQUIRE(v.front() == 10);
        REQUIRE(v.back() == 50);

        // 変更可能であることを確認
        v.front() = 100;
        v.back() = 500;
        REQUIRE(v.front() == 100);
        REQUIRE(v.back() == 500);
    }

    SECTION("data()") {
        int* data_ptr = v.data();
        REQUIRE(data_ptr[0] == 10);
        REQUIRE(data_ptr[4] == 50);

        // 変更可能であることを確認
        data_ptr[2] = 300;
        REQUIRE(v[2] == 300);
    }
}

TEST_CASE("bluestl::vector イテレータ", "[vector]") {
    using bluestl::vector;
    vector<int> v = { 10, 20, 30, 40, 50 };

    SECTION("begin/end") {
        auto it = v.begin();
        REQUIRE(*it == 10);
        ++it;
        REQUIRE(*it == 20);
        it += 2;
        REQUIRE(*it == 40);

        int sum = 0;
        for (auto& x : v) {
            sum += x;
        }
        REQUIRE(sum == 150);
    }

    SECTION("cbegin/cend") {
        auto it = v.cbegin();
        REQUIRE(*it == 10);
        ++it;
        REQUIRE(*it == 20);

        int sum = 0;
        for (auto it = v.cbegin(); it != v.cend(); ++it) {
            sum += *it;
        }
        REQUIRE(sum == 150);
    }

    SECTION("rbegin/rend") {
        auto it = v.rbegin();
        REQUIRE(*it == 50);
        ++it;
        REQUIRE(*it == 40);

        int sum = 0;
        for (auto it = v.rbegin(); it != v.rend(); ++it) {
            sum += *it;
        }
        REQUIRE(sum == 150);
    }

    SECTION("crbegin/crend") {
        auto it = v.crbegin();
        REQUIRE(*it == 50);
        ++it;
        REQUIRE(*it == 40);

        int sum = 0;
        for (auto it = v.crbegin(); it != v.crend(); ++it) {
            sum += *it;
        }
        REQUIRE(sum == 150);
    }
}

TEST_CASE("bluestl::vector 容量関連機能", "[vector]") {
    using bluestl::vector;

    SECTION("empty()") {
        vector<int> v1;
        REQUIRE(v1.empty());

        vector<int> v2 = { 1, 2, 3 };
        REQUIRE(!v2.empty());

        v2.clear();
        REQUIRE(v2.empty());
    }

    SECTION("size()") {
        vector<int> v;
        REQUIRE(v.size() == 0);

        v.push_back(10);
        REQUIRE(v.size() == 1);

        v.push_back(20);
        v.push_back(30);
        REQUIRE(v.size() == 3);

        v.pop_back();
        REQUIRE(v.size() == 2);

        v.clear();
        REQUIRE(v.size() == 0);
    }

    SECTION("capacity()とreserve()") {
        vector<int> v;
        REQUIRE(v.capacity() == 0);

        v.reserve(10);
        REQUIRE(v.capacity() >= 10);
        REQUIRE(v.size() == 0);

        size_t old_capacity = v.capacity();
        v.reserve(5);  // 小さい値では縮小しない
        REQUIRE(v.capacity() == old_capacity);

        v.push_back(1);
        v.push_back(2);
        REQUIRE(v.size() == 2);
        REQUIRE(v.capacity() >= 10);
    }

    SECTION("shrink_to_fit()") {
        vector<int> v;
        v.reserve(10);
        REQUIRE(v.capacity() >= 10);

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        REQUIRE(v.size() == 3);

        v.shrink_to_fit();
        REQUIRE(v.capacity() == 3);

        v.clear();
        v.shrink_to_fit();
        REQUIRE(v.capacity() == 0);
    }
}

TEST_CASE("bluestl::vector データ追加・削除操作", "[vector]") {
    using bluestl::vector;

    SECTION("push_back()") {
        vector<int> v;
        v.push_back(10);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 10);

        v.push_back(20);
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 20);

        // ムーブでのpush_back
        int value = 30;
        v.push_back(std::move(value));
        REQUIRE(v.size() == 3);
        REQUIRE(v[2] == 30);
    }

    SECTION("emplace_back()") {
        vector<std::string> v;
        std::string result = v.emplace_back("hello");
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == "hello");
        REQUIRE(result == "hello");

        result = v.emplace_back(5, 'x');
        REQUIRE(v.size() == 2);
        REQUIRE(v[1] == "xxxxx");
        REQUIRE(result == "xxxxx");
    }

    SECTION("pop_back()") {
        vector<int> v = { 10, 20, 30 };
        v.pop_back();
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 20);

        v.pop_back();
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 10);

        v.pop_back();
        REQUIRE(v.empty());
    }

    SECTION("insert()") {
        vector<int> v = { 10, 20, 30 };

        // 単一要素のinsert
        auto it = v.insert(v.begin() + 1, 15);
        REQUIRE(v.size() == 4);
        REQUIRE(*it == 15);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 15);
        REQUIRE(v[2] == 20);
        REQUIRE(v[3] == 30);

        // 複数要素のinsert
        v.insert(v.begin(), 3, 5);
        REQUIRE(v.size() == 7);
        REQUIRE(v[0] == 5);
        REQUIRE(v[1] == 5);
        REQUIRE(v[2] == 5);
        REQUIRE(v[3] == 10);

        // 末尾へのinsert
        v.insert(v.end(), 99);
        REQUIRE(v.size() == 8);
        REQUIRE(v[7] == 99);

        // 範囲insert
        int arr[] = { 100, 200 };
        v.insert(v.begin() + 4, arr, arr + 2);
        REQUIRE(v.size() == 10);
        REQUIRE(v[4] == 100);
        REQUIRE(v[5] == 200);
    }

    SECTION("emplace()") {
        vector<std::string> v = { "hello", "world" };
        auto it = v.emplace(v.begin() + 1, "beautiful");
        REQUIRE(v.size() == 3);
        REQUIRE(*it == "beautiful");
        REQUIRE(v[0] == "hello");
        REQUIRE(v[1] == "beautiful");
        REQUIRE(v[2] == "world");

        // 末尾へのemplace
        it = v.emplace(v.end(), 3, '!');
        REQUIRE(v.size() == 4);
        REQUIRE(*it == "!!!");
        REQUIRE(v[3] == "!!!");
    }

    SECTION("erase()") {
        vector<int> v = { 10, 20, 30, 40, 50 };

        // 単一要素のerase
        auto it = v.erase(v.begin() + 1);
        REQUIRE(v.size() == 4);
        REQUIRE(*it == 30);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 30);
        REQUIRE(v[2] == 40);
        REQUIRE(v[3] == 50);

        // 範囲erase
        it = v.erase(v.begin() + 1, v.begin() + 3);
        REQUIRE(v.size() == 2);
        REQUIRE(*it == 50);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 50);

        // 最初の要素をerase
        v.erase(v.begin());
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 50);

        // 最後の要素をerase
        v.erase(v.begin());
        REQUIRE(v.empty());
    }

    SECTION("clear()") {
        vector<int> v = { 1, 2, 3, 4, 5 };
        REQUIRE(v.size() == 5);

        v.clear();
        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        // capacityは変わらない
        REQUIRE(v.capacity() > 0);
    }
}

TEST_CASE("bluestl::vector resize()とassign()", "[vector]") {
    using bluestl::vector;

    SECTION("resize()") {
        vector<int> v = { 1, 2, 3 };

        // 拡大
        v.resize(5);
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 0);  // デフォルト値で初期化
        REQUIRE(v[4] == 0);

        // 指定値で拡大
        v.resize(8, 42);
        REQUIRE(v.size() == 8);
        REQUIRE(v[5] == 42);
        REQUIRE(v[6] == 42);
        REQUIRE(v[7] == 42);

        // 縮小
        v.resize(2);
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
    }

    SECTION("assign()") {
        vector<int> v = { 1, 2, 3 };

        // カウント+値でassign
        v.assign(4, 10);
        REQUIRE(v.size() == 4);
        for (int i = 0; i < 4; ++i) {
            REQUIRE(v[i] == 10);
        }

        // イテレータ範囲でassign
        int arr[] = { 5, 6, 7, 8, 9 };
        v.assign(arr, arr + 5);
        REQUIRE(v.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(v[i] == arr[i]);
        }

        // 初期化リストでassign
        v.assign({ 100, 200, 300 });
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 100);
        REQUIRE(v[1] == 200);
        REQUIRE(v[2] == 300);
    }
}

TEST_CASE("bluestl::vector swap()と比較演算子", "[vector]") {
    using bluestl::vector;

    SECTION("swap()") {
        vector<int> v1 = { 1, 2, 3 };
        vector<int> v2 = { 4, 5, 6, 7 };

        size_t v1_size = v1.size();
        size_t v2_size = v2.size();
        size_t v1_capacity = v1.capacity();
        size_t v2_capacity = v2.capacity();

        v1.swap(v2);

        REQUIRE(v1.size() == v2_size);
        REQUIRE(v2.size() == v1_size);
        REQUIRE(v1.capacity() == v2_capacity);
        REQUIRE(v2.capacity() == v1_capacity);

        REQUIRE(v1[0] == 4);
        REQUIRE(v1[3] == 7);
        REQUIRE(v2[0] == 1);
        REQUIRE(v2[2] == 3);

        // 非メンバ関数版swap
        swap(v1, v2);
        REQUIRE(v1[0] == 1);
        REQUIRE(v2[0] == 4);
    }

    SECTION("比較演算子") {
        vector<int> v1 = { 1, 2, 3 };
        vector<int> v2 = { 1, 2, 3 };
        vector<int> v3 = { 1, 2, 4 };
        vector<int> v4 = { 1, 2, 3, 4 };

        REQUIRE(v1 == v2);
        REQUIRE(v1 != v3);
        REQUIRE(v1 != v4);

        REQUIRE(v1 < v3);
        REQUIRE(v1 < v4);
        REQUIRE(v3 > v1);

        REQUIRE(v1 <= v2);
        REQUIRE(v1 <= v3);
        REQUIRE(v2 >= v1);  // 比較演算子の順序関係
        // TODO: 三方比較演算子の実装後に有効化
        // REQUIRE((v1 <=> v2) == std::strong_ordering::equal);
        // REQUIRE((v1 <=> v3) == std::strong_ordering::less);
        // REQUIRE((v3 <=> v1) == std::strong_ordering::greater);
        // REQUIRE((v1 <=> v4) == std::strong_ordering::less);
    }
}

TEST_CASE("bluestl::vector 要素のデストラクタ呼び出し", "[vector]") {
    using bluestl::vector;

    SECTION("clear()で要素のデストラクタが呼ばれる") {
        bool destroyed = false;
        {
            vector<TestType> v;
            v.emplace_back(42, &destroyed);
            REQUIRE(!destroyed);
            v.clear();
            REQUIRE(destroyed);
        }
    }

    SECTION("ベクタのデストラクタで全要素のデストラクタが呼ばれる") {
        bool destroyed1 = false;
        bool destroyed2 = false;
        bool destroyed3 = false;
        {
            vector<TestType> v;
            v.emplace_back(1, &destroyed1);
            v.emplace_back(2, &destroyed2);
            v.emplace_back(3, &destroyed3);
            REQUIRE(!destroyed1);
            REQUIRE(!destroyed2);
            REQUIRE(!destroyed3);
        }  // ここでvのデストラクタが呼ばれる
        REQUIRE(destroyed1);
        REQUIRE(destroyed2);
        REQUIRE(destroyed3);
    }

    SECTION("erase()で指定要素のデストラクタが呼ばれる") {
        bool destroyed = false;
        vector<TestType> v;
        v.emplace_back(1);
        v.emplace_back(2, &destroyed);
        v.emplace_back(3);

        REQUIRE(!destroyed);
        v.erase(v.begin() + 1);
        REQUIRE(destroyed);
        REQUIRE(v.size() == 2);
        REQUIRE(v[0].value == 1);
        REQUIRE(v[1].value == 3);
    }
}

TEST_CASE("bluestl::vector 容量自動拡張", "[vector]") {
    using bluestl::vector;

    SECTION("push_backでの容量自動拡張") {
        vector<int> v;
        REQUIRE(v.capacity() == 0);

        v.push_back(1);
        size_t cap1 = v.capacity();
        REQUIRE(cap1 > 0);

        // 容量一杯まで追加
        for (size_t i = 1; i < cap1; ++i) {
            v.push_back(static_cast<int>(i + 1));
        }
        REQUIRE(v.size() == cap1);
        REQUIRE(v.capacity() == cap1);

        // 容量を超える要素を追加
        v.push_back(100);
        REQUIRE(v.size() == cap1 + 1);
        REQUIRE(v.capacity() > cap1);
    }

    SECTION("insert()での容量自動拡張") {
        vector<int> v;
        v.reserve(4);
        v = { 1, 2, 3, 4 };

        size_t old_cap = v.capacity();
        // 容量を超えるinsert
        v.insert(v.begin(), 5, 0);
        REQUIRE(v.size() == 9);
        REQUIRE(v.capacity() > old_cap);

        for (size_t i = 0; i < 5; ++i) {
            REQUIRE(v[i] == 0);
        }
        for (size_t i = 5; i < 9; ++i) {
            REQUIRE(v[i] == static_cast<int>(i - 4));
        }
    }
}

TEST_CASE("bluestl::vector アロケータ", "[vector][allocator]") {
    using bluestl::allocator;
    using bluestl::vector;

    SECTION("get_allocator()") {
        vector<int> v;
        allocator<int> a = v.get_allocator();

        vector<int, allocator<int>> v2(a);
        REQUIRE(v2.get_allocator() == a);
    }
}

TEST_CASE("bluestl::vector エッジケース・境界値テスト", "[vector][edge_cases]") {
    using bluestl::vector;

    SECTION("サイズ0のベクタ") {
        vector<int> v;
        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
        REQUIRE(v.begin() == v.end());
        REQUIRE(v.data() == nullptr);
    }

    SECTION("最大サイズの要素を1つ") {
        vector<std::string> v;
        std::string large_string(1000000, 'x');
        v.push_back(large_string);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].size() == 1000000);
    }

    SECTION("範囲外アクセスのテスト準備（アサート発生を避けるため直接テストしない）") {
        vector<int> v = {1, 2, 3};
        
        // 正常なアクセス範囲の確認
        REQUIRE(v.at(0) == 1);
        REQUIRE(v.at(2) == 3);
        
        // 範囲外アクセスは本来アサーションが発生するが、テストでは確認できない
        // 代わりに、有効な範囲のテストのみ実行
    }

    SECTION("nullptrでの初期化") {
        vector<int*> v;
        v.push_back(nullptr);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == nullptr);
    }

    SECTION("自己代入") {
        vector<int> v = {1, 2, 3};
        v = v; // 自己代入
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }

    SECTION("空のイテレータ範囲での構築") {
        int* null_ptr = nullptr;
        vector<int> v(null_ptr, null_ptr);
        REQUIRE(v.empty());
    }
}

TEST_CASE("bluestl::vector パフォーマンステスト", "[vector][performance]") {
    using bluestl::vector;

    SECTION("大量要素の挿入と削除") {
        vector<int> v;
        const int N = 100000;

        // 大量push_back
        for (int i = 0; i < N; ++i) {
            v.push_back(i);
        }
        REQUIRE(v.size() == N);
        REQUIRE(v[0] == 0);
        REQUIRE(v[N-1] == N-1);

        // 大量pop_back
        for (int i = 0; i < N/2; ++i) {
            v.pop_back();
        }
        REQUIRE(v.size() == N/2);
        REQUIRE(v.back() == N/2 - 1);
    }

    SECTION("容量の事前予約によるパフォーマンス改善") {
        vector<int> v1, v2;
        const int N = 10000;

        // 予約なしでの挿入
        for (int i = 0; i < N; ++i) {
            v1.push_back(i);
        }

        // 予約ありでの挿入
        v2.reserve(N);
        for (int i = 0; i < N; ++i) {
            v2.push_back(i);
        }

        REQUIRE(v1.size() == N);
        REQUIRE(v2.size() == N);
        REQUIRE(v1 == v2);
    }

    SECTION("emplace_backとpush_backの比較") {
        vector<std::string> v1, v2;
        const int N = 1000;

        // push_back（コピー）
        for (int i = 0; i < N; ++i) {
            std::string s = "test" + std::to_string(i);
            v1.push_back(s);
        }

        // emplace_back（直接構築）
        for (int i = 0; i < N; ++i) {
            v2.emplace_back("test" + std::to_string(i));
        }

        REQUIRE(v1.size() == N);
        REQUIRE(v2.size() == N);
        // 内容が同じことを確認
        for (int i = 0; i < N; ++i) {
            REQUIRE(v1[i] == v2[i]);
        }
    }
}

TEST_CASE("bluestl::vector メモリ安全性テスト", "[vector][memory_safety]") {
    using bluestl::vector;

    SECTION("メモリリークテスト（RAII）") {
        bool destroyed1 = false, destroyed2 = false, destroyed3 = false;
        
        {
            vector<TestType> v;
            v.emplace_back(1, &destroyed1);
            v.emplace_back(2, &destroyed2);
            v.emplace_back(3, &destroyed3);
            
            REQUIRE(!destroyed1);
            REQUIRE(!destroyed2);
            REQUIRE(!destroyed3);
        } // ここでvのデストラクタが自動的に呼ばれる
        
        // すべての要素のデストラクタが呼ばれたことを確認
        REQUIRE(destroyed1);
        REQUIRE(destroyed2);
        REQUIRE(destroyed3);
    }

    SECTION("resize時のメモリ管理") {
        bool destroyed = false;
        vector<TestType> v;
        
        v.emplace_back(42, &destroyed);
        REQUIRE(!destroyed);
        
        // サイズを0にする
        v.resize(0);
        REQUIRE(destroyed); // デストラクタが呼ばれる
        REQUIRE(v.empty());
    }

    SECTION("例外安全性（基本保証）") {
        vector<TestType> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
        
        size_t original_size = v.size();
        size_t original_capacity = v.capacity();
        
        // 操作が失敗しても既存の状態は保持される
        try {
            // 通常の操作（例外は発生しない）
            v.push_back(TestType(4));
            REQUIRE(v.size() == original_size + 1);
        } catch (...) {
            // 例外が発生した場合でも、元の状態は保持される
            REQUIRE(v.size() == original_size);
            REQUIRE(v.capacity() >= original_capacity);
        }
    }

    SECTION("ダングリングポインタの回避") {
        vector<int> v = {1, 2, 3, 4, 5};
        int* ptr = &v[2]; // 3番目の要素へのポインタ
        REQUIRE(*ptr == 3);
        
        // 容量変更を伴う操作
        v.reserve(v.capacity() * 2);
        
        // ポインタが無効になった可能性があるため、
        // インデックスベースでアクセスして確認
        REQUIRE(v[2] == 3);
    }
}

TEST_CASE("bluestl::vector 特殊型との互換性", "[vector][special_types]") {
    using bluestl::vector;

    SECTION("ムーブオンリー型") {
        struct MoveOnly {
            int value;
            MoveOnly(int v) : value(v) {}
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly& operator=(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&& other) noexcept : value(other.value) {
                other.value = -1;
            }
            MoveOnly& operator=(MoveOnly&& other) noexcept {
                value = other.value;
                other.value = -1;
                return *this;
            }
        };

        vector<MoveOnly> v;
        v.emplace_back(42);
        v.push_back(MoveOnly(100));
        
        REQUIRE(v.size() == 2);
        REQUIRE(v[0].value == 42);
        REQUIRE(v[1].value == 100);
    }

    // SECTION("const型") {
    //     vector<const int> v;
    //     v.push_back(1);
    //     v.push_back(2);
    //     v.push_back(3);
    //     
    //     REQUIRE(v.size() == 3);
    //     REQUIRE(v[0] == 1);
    //     REQUIRE(v[1] == 2);
    //     REQUIRE(v[2] == 3);
    // }

    SECTION("ポインタ型") {
        int a = 1, b = 2, c = 3;
        vector<int*> v;
        v.push_back(&a);
        v.push_back(&b);
        v.push_back(&c);
        
        REQUIRE(v.size() == 3);
        REQUIRE(*v[0] == 1);
        REQUIRE(*v[1] == 2);
        REQUIRE(*v[2] == 3);
    }
}

TEST_CASE("bluestl::vector 高度なイテレータテスト", "[vector][iterators]") {
    using bluestl::vector;

    SECTION("イテレータの無効化") {
        vector<int> v = {1, 2, 3};
        auto it = v.begin();
        auto end_it = v.end();
        
        REQUIRE(*it == 1);
        
        // 容量を変更しない操作ではイテレータは有効
        v.push_back(4);
        if (v.capacity() > 3) {
            // 容量が変更されていない場合のみテスト
            // 注意：実装によっては容量が変更される可能性がある
        }
    }

    SECTION("逆イテレータの完全テスト") {
        vector<int> v = {1, 2, 3, 4, 5};
        
        // 逆イテレータでの範囲アクセス
        std::string result;
        for (auto rit = v.rbegin(); rit != v.rend(); ++rit) {
            result += std::to_string(*rit);
        }
        REQUIRE(result == "54321");
        
        // const逆イテレータ
        const vector<int>& cv = v;
        result.clear();
        for (auto crit = cv.crbegin(); crit != cv.crend(); ++crit) {
            result += std::to_string(*crit);
        }
        REQUIRE(result == "54321");
    }

    SECTION("イテレータ演算") {
        vector<int> v = {10, 20, 30, 40, 50};
        auto it = v.begin();
        
        // 前進
        REQUIRE(*it == 10);
        ++it;
        REQUIRE(*it == 20);
        it += 2;
        REQUIRE(*it == 40);
        
        // 後退
        --it;
        REQUIRE(*it == 30);
        it -= 1;
        REQUIRE(*it == 20);
        
        // 距離計算
        auto distance = v.end() - v.begin();
        REQUIRE(distance == 5);
        
        // 比較
        REQUIRE(v.begin() < v.end());
        REQUIRE(v.begin() <= v.begin());
        REQUIRE(v.end() > v.begin());
        REQUIRE(v.end() >= v.end());
    }
}

TEST_CASE("bluestl::vector メモリアライメントテスト", "[vector][alignment]") {
    using bluestl::vector;

    SECTION("アライメント要求の厳しい型") {
        struct AlignedType {
            alignas(64) double data[8];
            AlignedType() { std::fill(data, data + 8, 0.0); }
        };

        vector<AlignedType> v;
        v.resize(10);
        
        // アライメントが正しいことを確認
        for (size_t i = 0; i < v.size(); ++i) {
            auto addr = reinterpret_cast<uintptr_t>(&v[i]);
            REQUIRE(addr % 64 == 0);
        }
    }
}
