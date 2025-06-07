// filepath: f:\dev\github\bluestl\tests\test_vector.cpp
#include "bluestl/vector.h"
#include <catch2/catch_test_macros.hpp>
#include <string>

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
        int arr[] = {1, 2, 3, 4, 5};
        vector<int> v(arr, arr + 5);
        REQUIRE(v.size() == 5);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(v[i] == arr[i]);
        }
    }

    SECTION("コピーコンストラクタ") {
        vector<int> v1 = {1, 2, 3};
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
        vector<int> v1 = {1, 2, 3};
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
        vector<int> v = {5, 4, 3, 2, 1};
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
        vector<int> v1 = {1, 2, 3};
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
        vector<int> v1 = {1, 2, 3};
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
        v = {10, 20, 30};
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 20);
        REQUIRE(v[2] == 30);
    }
}

TEST_CASE("bluestl::vector 要素アクセス", "[vector]") {
    using bluestl::vector;
    vector<int> v = {10, 20, 30, 40, 50};

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
    vector<int> v = {10, 20, 30, 40, 50};

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

        vector<int> v2 = {1, 2, 3};
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
        v.reserve(5); // 小さい値では縮小しない
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
        vector<int> v = {10, 20, 30};
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
        vector<int> v = {10, 20, 30};

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
        int arr[] = {100, 200};
        v.insert(v.begin() + 4, arr, arr + 2);
        REQUIRE(v.size() == 10);
        REQUIRE(v[4] == 100);
        REQUIRE(v[5] == 200);
    }

    SECTION("emplace()") {
        vector<std::string> v = {"hello", "world"};
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
        vector<int> v = {10, 20, 30, 40, 50};

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
        vector<int> v = {1, 2, 3, 4, 5};
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
        vector<int> v = {1, 2, 3};

        // 拡大
        v.resize(5);
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 0); // デフォルト値で初期化
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
        vector<int> v = {1, 2, 3};

        // カウント+値でassign
        v.assign(4, 10);
        REQUIRE(v.size() == 4);
        for (int i = 0; i < 4; ++i) {
            REQUIRE(v[i] == 10);
        }

        // イテレータ範囲でassign
        int arr[] = {5, 6, 7, 8, 9};
        v.assign(arr, arr + 5);
        REQUIRE(v.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(v[i] == arr[i]);
        }

        // 初期化リストでassign
        v.assign({100, 200, 300});
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 100);
        REQUIRE(v[1] == 200);
        REQUIRE(v[2] == 300);
    }
}

TEST_CASE("bluestl::vector swap()と比較演算子", "[vector]") {
    using bluestl::vector;

    SECTION("swap()") {
        vector<int> v1 = {1, 2, 3};
        vector<int> v2 = {4, 5, 6, 7};

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
        vector<int> v1 = {1, 2, 3};
        vector<int> v2 = {1, 2, 3};
        vector<int> v3 = {1, 2, 4};
        vector<int> v4 = {1, 2, 3, 4};

        REQUIRE(v1 == v2);
        REQUIRE(v1 != v3);
        REQUIRE(v1 != v4);

        REQUIRE(v1 < v3);
        REQUIRE(v1 < v4);
        REQUIRE(v3 > v1);

        REQUIRE(v1 <= v2);
        REQUIRE(v1 <= v3);
        REQUIRE(v2 >= v1);

        // 比較演算子の順序関係
        REQUIRE((v1 <=> v2) == 0);
        REQUIRE((v1 <=> v3) < 0);
        REQUIRE((v3 <=> v1) > 0);
        REQUIRE((v1 <=> v4) < 0);
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
        } // ここでvのデストラクタが呼ばれる
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
        v = {1, 2, 3, 4};

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
    using bluestl::vector;
    using bluestl::allocator;

    SECTION("get_allocator()") {
        vector<int> v;
        allocator<int> a = v.get_allocator();

        vector<int, allocator<int>> v2(a);
        REQUIRE(v2.get_allocator() == a);
    }
}
