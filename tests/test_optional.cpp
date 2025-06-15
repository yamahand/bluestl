/**
 * @file test_optional.cpp
 * @brief bluestl::optional の包括的テスト
 * 
 * エッジケース、パフォーマンス、メモリ安全性のテストを含む
 */

#include "bluestl/optional.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>

// テスト用のカスタム型
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
};

TEST_CASE("bluestl::optional 基本動作", "[optional]") {
    using bluestl::optional;
    
    SECTION("デフォルト構築") {
        optional<int> o1;
        REQUIRE(!o1.has_value());
        REQUIRE(!o1);
    }

    SECTION("値の構築") {
        optional<int> o2(42);
        REQUIRE(o2.has_value());
        REQUIRE(o2);
        REQUIRE(*o2 == 42);
        REQUIRE(o2.value() == 42);
    }

    SECTION("コピー構築") {
        optional<int> o2(42);
        optional<int> o3(o2);
        REQUIRE(o3.has_value());
        REQUIRE(*o3 == 42);
    }

    SECTION("ムーブ構築") {
        optional<int> o2(42);
        optional<int> o4(std::move(o2));
        REQUIRE(o4.has_value());
        REQUIRE(*o4 == 42);
    }

    SECTION("emplace") {
        optional<int> o1;
        o1.emplace(100);
        REQUIRE(o1.has_value());
        REQUIRE(*o1 == 100);
    }

    SECTION("reset") {
        optional<int> o1(42);
        REQUIRE(o1.has_value());
        o1.reset();
        REQUIRE(!o1.has_value());
    }

    SECTION("コピー代入") {
        optional<int> o1;
        optional<int> o3(42);
        o1 = o3;
        REQUIRE(o1.has_value());
        REQUIRE(*o1 == 42);
    }

    SECTION("ムーブ代入") {
        optional<int> o1;
        o1 = optional<int>(77);
        REQUIRE(o1.has_value());
        REQUIRE(*o1 == 77);
    }

    SECTION("string型") {
        optional<std::string> os1;
        REQUIRE(!os1.has_value());
        os1.emplace("hello");
        REQUIRE(os1.has_value());
        REQUIRE(*os1 == "hello");
    }
}

TEST_CASE("bluestl::optional アクセス操作", "[optional]") {
    using bluestl::optional;

    SECTION("operator* と operator->") {
        optional<std::string> opt("test");
        REQUIRE(*opt == "test");
        REQUIRE(opt->size() == 4);
        REQUIRE(opt->at(0) == 't');
    }

    SECTION("value()") {
        optional<int> opt(42);
        REQUIRE(opt.value() == 42);
        
        // 空のoptionalでvalue()を呼ぶとアサーション（テストでは確認できない）
        // optional<int> empty_opt;
        // empty_opt.value(); // アサーション発生
    }

    SECTION("value_or()") {
        optional<int> opt(42);
        REQUIRE(opt.value_or(0) == 42);
        
        optional<int> empty_opt;
        REQUIRE(empty_opt.value_or(0) == 0);
        REQUIRE(empty_opt.value_or(999) == 999);
    }

    SECTION("const版のアクセス") {
        const optional<std::string> const_opt("const_test");
        REQUIRE(*const_opt == "const_test");
        REQUIRE(const_opt->size() == 10);
        REQUIRE(const_opt.value() == "const_test");
        REQUIRE(const_opt.value_or("default") == "const_test");
    }
}

TEST_CASE("bluestl::optional 比較演算子", "[optional]") {
    using bluestl::optional;

    SECTION("optional同士の比較") {
        optional<int> opt1(42);
        optional<int> opt2(42);
        optional<int> opt3(100);
        optional<int> empty1;
        optional<int> empty2;

        // 等価性
        REQUIRE(opt1 == opt2);
        REQUIRE(opt1 != opt3);
        REQUIRE(empty1 == empty2);
        REQUIRE(opt1 != empty1);

        // 順序比較
        REQUIRE(opt1 < opt3);
        REQUIRE(opt3 > opt1);
        REQUIRE(opt1 <= opt2);
        REQUIRE(opt1 >= opt2);
        REQUIRE(empty1 < opt1);
        REQUIRE(opt1 > empty1);
    }

    SECTION("値との比較") {
        optional<int> opt(42);
        optional<int> empty;

        REQUIRE(opt == 42);
        REQUIRE(42 == opt);
        REQUIRE(opt != 100);
        REQUIRE(100 != opt);
        REQUIRE(empty != 42);
        REQUIRE(42 != empty);

        REQUIRE(opt < 100);
        REQUIRE(42 <= opt);
        REQUIRE(opt > 0);
        REQUIRE(100 >= opt);
    }
}

TEST_CASE("bluestl::optional エッジケース", "[optional]") {
    using bluestl::optional;

    SECTION("空のoptionalでの操作") {
        optional<int> empty;
        
        REQUIRE(!empty.has_value());
        REQUIRE(!empty);
        REQUIRE(empty.value_or(42) == 42);
        
        // アクセス操作は本来アサーション発生（テストでは確認できない）
    }

    SECTION("自己代入") {
        optional<int> opt(42);
        opt = opt;
        REQUIRE(opt.has_value());
        REQUIRE(*opt == 42);
    }

    SECTION("nullopt での初期化と代入") {
        optional<int> opt1(bluestl::nullopt);
        REQUIRE(!opt1.has_value());

        optional<int> opt2(42);
        opt2 = bluestl::nullopt;
        REQUIRE(!opt2.has_value());
    }

    SECTION("make_optional") {
        auto opt1 = bluestl::make_optional(42);
        REQUIRE(opt1.has_value());
        REQUIRE(*opt1 == 42);

        auto opt2 = bluestl::make_optional<std::string>("hello");
        REQUIRE(opt2.has_value());
        REQUIRE(*opt2 == "hello");
    }
}

TEST_CASE("bluestl::optional メモリ管理", "[optional]") {
    using bluestl::optional;

    SECTION("デストラクタ呼び出し") {
        bool destroyed = false;
        
        {
            optional<TestType> opt;
            opt.emplace(42, &destroyed);
            REQUIRE(!destroyed);
        } // optionalのデストラクタが呼ばれる
        
        REQUIRE(destroyed);
    }

    SECTION("reset時のデストラクタ呼び出し") {
        bool destroyed = false;
        
        optional<TestType> opt;
        opt.emplace(42, &destroyed);
        REQUIRE(!destroyed);
        
        opt.reset();
        REQUIRE(destroyed);
        REQUIRE(!opt.has_value());
    }

    SECTION("再代入時のデストラクタ呼び出し") {
        bool destroyed1 = false;
        bool destroyed2 = false;
        
        optional<TestType> opt;
        opt.emplace(1, &destroyed1);
        REQUIRE(!destroyed1);
        
        opt.emplace(2, &destroyed2);
        REQUIRE(destroyed1); // 古い値のデストラクタが呼ばれる
        REQUIRE(!destroyed2);
        REQUIRE(opt->value == 2);
    }
}

TEST_CASE("bluestl::optional 特殊型との互換性", "[optional]") {
    using bluestl::optional;

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

        optional<MoveOnly> opt;
        opt.emplace(42);
        REQUIRE(opt->value == 42);

        optional<MoveOnly> opt2 = std::move(opt);
        REQUIRE(opt2->value == 42);
    }

    SECTION("参照型") {
        int x = 42;
        optional<int&> opt_ref(x);
        REQUIRE(opt_ref.has_value());
        REQUIRE(&(*opt_ref) == &x);
        REQUIRE(*opt_ref == 42);

        *opt_ref = 100;
        REQUIRE(x == 100);
    }

    SECTION("const型") {
        optional<const int> opt(42);
        REQUIRE(opt.has_value());
        REQUIRE(*opt == 42);
        // *opt = 100; // コンパイルエラー（const）
    }

    SECTION("ポインタ型") {
        int x = 42;
        optional<int*> opt(&x);
        REQUIRE(opt.has_value());
        REQUIRE(**opt == 42);

        optional<int*> null_opt(nullptr);
        REQUIRE(null_opt.has_value());
        REQUIRE(*null_opt == nullptr);
    }
}

TEST_CASE("bluestl::optional コピー・ムーブセマンティクス", "[optional]") {
    using bluestl::optional;

    SECTION("値からのコピー構築") {
        std::string str = "test";
        optional<std::string> opt(str);
        REQUIRE(*opt == "test");
        
        // 元の値を変更しても影響なし
        str = "changed";
        REQUIRE(*opt == "test");
    }

    SECTION("値からのムーブ構築") {
        std::string str = "test";
        optional<std::string> opt(std::move(str));
        REQUIRE(*opt == "test");
        // str は移動されている可能性がある
    }

    SECTION("optional間のコピー") {
        optional<std::string> opt1("original");
        optional<std::string> opt2(opt1);
        
        REQUIRE(*opt2 == "original");
        
        // opt1を変更してもopt2に影響なし
        *opt1 = "changed";
        REQUIRE(*opt2 == "original");
    }

    SECTION("optional間のムーブ") {
        optional<std::string> opt1("original");
        optional<std::string> opt2(std::move(opt1));
        
        REQUIRE(*opt2 == "original");
        // opt1の状態は未定義（実装依存）
    }
}

TEST_CASE("bluestl::optional パフォーマンステスト", "[optional]") {
    using bluestl::optional;

    SECTION("大量のoptional作成と破棄") {
        const int N = 10000;
        
        for (int i = 0; i < N; ++i) {
            optional<int> opt(i);
            REQUIRE(opt.has_value());
            REQUIRE(*opt == i);
        }
    }

    SECTION("emplace vs 代入のパフォーマンス") {
        optional<std::string> opt1, opt2;
        
        // emplace（直接構築）
        opt1.emplace(1000, 'a');
        REQUIRE(opt1->size() == 1000);
        
        // 代入（一時オブジェクト経由）
        opt2 = std::string(1000, 'a');
        REQUIRE(opt2->size() == 1000);
        
        REQUIRE(*opt1 == *opt2);
    }
}

TEST_CASE("bluestl::optional 型推論", "[optional]") {
    using bluestl::optional;

    SECTION("auto推論") {
        auto opt1 = optional<int>(42);
        static_assert(std::is_same_v<decltype(opt1), optional<int>>);
        REQUIRE(*opt1 == 42);

        auto opt2 = bluestl::make_optional(3.14);
        static_assert(std::is_same_v<decltype(opt2), optional<double>>);
        REQUIRE(*opt2 == 3.14);
    }
}
