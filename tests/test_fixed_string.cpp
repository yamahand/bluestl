/**
 * @file test_fixed_string.cpp
 * @brief bluestl::fixed_string の総合テスト
 *
 * - C++20準拠
 * - Doxygen形式コメント
 * - Catch2を利用
 * - test_main.cppから実行される
 */


#include <catch2/catch_test_macros.hpp>
#include <bluestl/fixed_string.h>
#include <string_view>

/**
 * @brief fixed_stringのコンストラクタテスト
 */
TEST_CASE("fixed_string constructors", "[fixed_string]") {
    SECTION("デフォルトコンストラクタ") {
        bluestl::fixed_string<10> str;
        REQUIRE(str.size() == 0);
        REQUIRE(str.empty());
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("C文字列からの構築") {
        bluestl::fixed_string<10> str("hello");
        REQUIRE(str.size() == 5);
        REQUIRE(!str.empty());
        REQUIRE(std::string_view(str.c_str()) == "hello");
    }

    SECTION("空のC文字列からの構築") {
        bluestl::fixed_string<10> str("");
        REQUIRE(str.size() == 0);
        REQUIRE(str.empty());
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("nullptrからの構築") {
        bluestl::fixed_string<10> str(nullptr);
        REQUIRE(str.size() == 0);
        REQUIRE(str.empty());
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("string_viewからの構築") {
        std::string_view sv("world");
        bluestl::fixed_string<10> str(sv);
        REQUIRE(str.size() == 5);
        REQUIRE(std::string_view(str.c_str()) == "world");
    }

    SECTION("コピーコンストラクタ") {
        bluestl::fixed_string<10> original("test");
        bluestl::fixed_string<10> copy(original);
        REQUIRE(copy.size() == 4);
        REQUIRE(std::string_view(copy.c_str()) == "test");
    }

    SECTION("ムーブコンストラクタ") {
        bluestl::fixed_string<10> original("move");
        bluestl::fixed_string<10> moved(std::move(original));
        REQUIRE(moved.size() == 4);
        REQUIRE(std::string_view(moved.c_str()) == "move");
    }

    SECTION("容量上限のテスト") {
        bluestl::fixed_string<5> str("12345");
        REQUIRE(str.size() == 5);
        REQUIRE(str.capacity() == 5);
        REQUIRE(std::string_view(str.c_str()) == "12345");
    }
}

/**
 * @brief fixed_stringの代入演算子テスト
 */
TEST_CASE("fixed_string assignment", "[fixed_string]") {
    SECTION("C文字列の代入") {
        bluestl::fixed_string<10> str;
        str = "assign";
        REQUIRE(str.size() == 6);
        REQUIRE(std::string_view(str.c_str()) == "assign");
    }

    SECTION("string_viewの代入") {
        bluestl::fixed_string<10> str;
        std::string_view sv("view");
        str = sv;
        REQUIRE(str.size() == 4);
        REQUIRE(std::string_view(str.c_str()) == "view");
    }

    SECTION("コピー代入") {
        bluestl::fixed_string<10> original("original");
        bluestl::fixed_string<10> target;
        target = original;
        REQUIRE(target.size() == 8);
        REQUIRE(std::string_view(target.c_str()) == "original");
    }

    SECTION("ムーブ代入") {
        bluestl::fixed_string<10> original("moveme");
        bluestl::fixed_string<10> target;
        target = std::move(original);
        REQUIRE(target.size() == 6);
        REQUIRE(std::string_view(target.c_str()) == "moveme");
    }
}

/**
 * @brief fixed_stringの要素アクセステスト
 */
TEST_CASE("fixed_string element access", "[fixed_string]") {
    SECTION("operator[]によるアクセス") {
        bluestl::fixed_string<10> str("access");
        REQUIRE(str[0] == 'a');
        REQUIRE(str[1] == 'c');
        REQUIRE(str[5] == 's');

        str[0] = 'A';
        REQUIRE(str[0] == 'A');
        REQUIRE(std::string_view(str.c_str()) == "Access");
    }

    SECTION("at()によるアクセス") {
        bluestl::fixed_string<10> str("access");
        REQUIRE(str.at(0) == 'a');
        REQUIRE(str.at(1) == 'c');
        REQUIRE(str.at(5) == 's');

        str.at(1) = 'C';
        REQUIRE(str.at(1) == 'C');
        REQUIRE(std::string_view(str.c_str()) == "aCcess");
    }

    SECTION("front()とback()") {
        bluestl::fixed_string<10> str("access");
        REQUIRE(str.front() == 'a');
        REQUIRE(str.back() == 's');

        str.front() = 'A';
        str.back() = 'S';
        REQUIRE(str.front() == 'A');
        REQUIRE(str.back() == 'S');
        REQUIRE(std::string_view(str.c_str()) == "AccesS");
    }

    SECTION("const版のアクセス") {
        const bluestl::fixed_string<10> const_str("const");
        REQUIRE(const_str[0] == 'c');
        REQUIRE(const_str.at(1) == 'o');
        REQUIRE(const_str.front() == 'c');
        REQUIRE(const_str.back() == 't');
    }
}

/**
 * @brief fixed_stringのイテレータテスト
 */
TEST_CASE("fixed_string iterators", "[fixed_string]") {
    bluestl::fixed_string<10> str("iter");

    SECTION("begin()とend()") {
        auto it = str.begin();
        REQUIRE(*it == 'i');
        ++it;
        REQUIRE(*it == 't');
        ++it;
        REQUIRE(*it == 'e');
        ++it;
        REQUIRE(*it == 'r');
        ++it;
        REQUIRE(it == str.end());
    }

    SECTION("range-based for loop") {
        std::string result;
        for (char c : str) {
            result += c;
        }
        REQUIRE(result == "iter");
    }

    SECTION("reverse iterators") {
        std::string result;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            result += *it;
        }
        REQUIRE(result == "reti");
    }

    SECTION("const iterators") {
        const bluestl::fixed_string<10> const_str("const");
        std::string result;
        for (auto it = const_str.cbegin(); it != const_str.cend(); ++it) {
            result += *it;
        }
        REQUIRE(result == "const");
    }
}

/**
 * @brief fixed_stringの容量・サイズテスト
 */
TEST_CASE("fixed_string capacity and size", "[fixed_string]") {
    SECTION("基本的な容量とサイズ") {
        bluestl::fixed_string<20> str("hello");
        REQUIRE(str.size() == 5);
        REQUIRE(str.length() == 5);
        REQUIRE(str.capacity() == 20);
        REQUIRE(str.max_size() == 20);
        REQUIRE(!str.empty());
    }

    SECTION("空の文字列") {
        bluestl::fixed_string<10> str;
        REQUIRE(str.size() == 0);
        REQUIRE(str.length() == 0);
        REQUIRE(str.capacity() == 10);
        REQUIRE(str.max_size() == 10);
        REQUIRE(str.empty());
    }

    SECTION("満杯の文字列") {
        bluestl::fixed_string<5> str("12345");
        REQUIRE(str.size() == 5);
        REQUIRE(str.capacity() == 5);
        REQUIRE(str.size() == str.capacity());
        REQUIRE(!str.empty());
    }
}

/**
 * @brief fixed_stringの変更操作テスト
 */
TEST_CASE("fixed_string modifiers", "[fixed_string]") {
    SECTION("clear()") {
        bluestl::fixed_string<10> str("clear");
        REQUIRE(str.size() == 5);

        str.clear();
        REQUIRE(str.size() == 0);
        REQUIRE(str.empty());
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("push_back()") {
        bluestl::fixed_string<10> str("push");
        str.push_back('!');
        REQUIRE(str.size() == 5);
        REQUIRE(std::string_view(str.c_str()) == "push!");
    }

    SECTION("pop_back()") {
        bluestl::fixed_string<10> str("pop!");
        str.pop_back();
        REQUIRE(str.size() == 3);
        REQUIRE(std::string_view(str.c_str()) == "pop");
    }

    SECTION("append() - C文字列") {
        bluestl::fixed_string<20> str("hello");
        str.append(" world");
        REQUIRE(str.size() == 11);
        REQUIRE(std::string_view(str.c_str()) == "hello world");
    }

    SECTION("append() - string_view") {
        bluestl::fixed_string<20> str("hello");
        std::string_view sv(" test");
        str.append(sv);
        REQUIRE(str.size() == 10);
        REQUIRE(std::string_view(str.c_str()) == "hello test");
    }

    SECTION("operator+=") {
        bluestl::fixed_string<20> str("test");
        str += " case";
        REQUIRE(str.size() == 9);
        REQUIRE(std::string_view(str.c_str()) == "test case");
    }

    SECTION("assign() - C文字列") {
        bluestl::fixed_string<10> str("old");
        str.assign("new");
        REQUIRE(str.size() == 3);
        REQUIRE(std::string_view(str.c_str()) == "new");
    }

    SECTION("assign() - string_view") {
        bluestl::fixed_string<10> str("old");
        std::string_view sv("newer");
        str.assign(sv);
        REQUIRE(str.size() == 5);
        REQUIRE(std::string_view(str.c_str()) == "newer");
    }

    SECTION("resize() - 縮小") {
        bluestl::fixed_string<10> str("resize");
        str.resize(3);
        REQUIRE(str.size() == 3);
        REQUIRE(std::string_view(str.c_str()) == "res");
    }

    SECTION("resize() - 拡大") {
        bluestl::fixed_string<10> str("hi");
        str.resize(5, 'x');
        REQUIRE(str.size() == 5);
        REQUIRE(std::string_view(str.c_str()) == "hixxx");
    }
}

/**
 * @brief fixed_stringの検索機能テスト
 */
TEST_CASE("fixed_string search operations", "[fixed_string]") {
    bluestl::fixed_string<20> str("hello world test");

    SECTION("find() - 文字検索") {
        REQUIRE(str.find('o') == 4);
        REQUIRE(str.find('w') == 6);
        REQUIRE(str.find('z') == bluestl::fixed_string<20>::npos);
    }

    SECTION("find() - 部分文字列検索") {
        REQUIRE(str.find("world") == 6);
        REQUIRE(str.find("test") == 12);
        REQUIRE(str.find("xyz") == bluestl::fixed_string<20>::npos);
    }

    SECTION("find() - 開始位置指定") {
        REQUIRE(str.find('o', 5) == 7);
        REQUIRE(str.find("o", 8) == bluestl::fixed_string<20>::npos);
    }

    SECTION("substr()") {
        auto sub1 = str.substr(0, 5);
        REQUIRE(std::string_view(sub1.c_str()) == "hello");

        auto sub2 = str.substr(6, 5);
        REQUIRE(std::string_view(sub2.c_str()) == "world");

        auto sub3 = str.substr(12);
        REQUIRE(std::string_view(sub3.c_str()) == "test");
    }

    SECTION("starts_with()") {
        REQUIRE(str.starts_with("hello"));
        REQUIRE(str.starts_with("h"));
        REQUIRE(!str.starts_with("world"));
        REQUIRE(!str.starts_with("Hello"));
    }

    SECTION("ends_with()") {
        REQUIRE(str.ends_with("test"));
        REQUIRE(str.ends_with("t"));
        REQUIRE(!str.ends_with("hello"));
        REQUIRE(!str.ends_with("Test"));
    }

    SECTION("contains()") {
        REQUIRE(str.contains("world"));
        REQUIRE(str.contains("hello"));
        REQUIRE(str.contains("test"));
        REQUIRE(str.contains("o w"));
        REQUIRE(!str.contains("xyz"));
    }
}

/**
 * @brief fixed_stringの比較演算子テスト
 */
TEST_CASE("fixed_string comparison operators", "[fixed_string]") {
    bluestl::fixed_string<10> str1("abc");
    bluestl::fixed_string<10> str2("abc");
    bluestl::fixed_string<10> str3("def");
    bluestl::fixed_string<10> str4("ab");

    SECTION("等価比較") {
        REQUIRE(str1 == str2);
        REQUIRE(!(str1 == str3));
        REQUIRE(str1 != str3);
        REQUIRE(!(str1 != str2));
    }

    SECTION("順序比較") {
        REQUIRE(str1 < str3);
        REQUIRE(str4 < str1);
        REQUIRE(str3 > str1);
        REQUIRE(str1 > str4);
        REQUIRE(str1 <= str2);
        REQUIRE(str1 <= str3);
        REQUIRE(str1 >= str2);
        REQUIRE(str3 >= str1);
    }

    SECTION("C文字列との比較") {
        REQUIRE(str1 == "abc");
        REQUIRE("abc" == str1);
        REQUIRE(str1 != "def");
        REQUIRE("def" != str1);
        REQUIRE(str1 < "def");
        REQUIRE("abc" <= str1);
    }

    SECTION("string_viewとの比較") {
        std::string_view sv("abc");
        REQUIRE(str1 == sv);
        REQUIRE(sv == str1);

        std::string_view sv2("def");
        REQUIRE(str1 != sv2);
        REQUIRE(str1 < sv2);
    }
}

/**
 * @brief fixed_stringのエッジケース・境界値テスト
 */
TEST_CASE("fixed_string edge cases", "[fixed_string]") {
    SECTION("容量ゼロの文字列") {
        bluestl::fixed_string<0> str;
        REQUIRE(str.size() == 0);
        REQUIRE(str.capacity() == 0);
        REQUIRE(str.empty());
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("容量1の文字列") {
        bluestl::fixed_string<1> str;
        str.push_back('x');
        REQUIRE(str.size() == 1);
        REQUIRE(str.capacity() == 1);
        REQUIRE(std::string_view(str.c_str()) == "x");
    }

    SECTION("容量上限での操作") {
        bluestl::fixed_string<3> str("abc");
        REQUIRE(str.size() == 3);
        REQUIRE(str.capacity() == 3);

        // 容量超過時の動作確認（実装によっては切り捨てまたは無視）
        str.push_back('d'); // 容量超過
        // 実装次第でサイズが変わらないか、切り捨てられる
    }

    SECTION("空文字列での操作") {
        bluestl::fixed_string<10> str;
        REQUIRE(str.find('x') == bluestl::fixed_string<10>::npos);
        REQUIRE(!str.starts_with("x"));
        REQUIRE(!str.ends_with("x"));
        REQUIRE(!str.contains("x"));

        auto sub = str.substr(0, 5);
        REQUIRE(sub.empty());
    }

    SECTION("長い文字列の切り捨て") {
        bluestl::fixed_string<5> str("1234567890");
        REQUIRE(str.size() <= 5);
        REQUIRE(str.capacity() == 5);
    }
}

/**
 * @brief fixed_stringのconstexpr機能テスト
 */
TEST_CASE("fixed_string constexpr", "[fixed_string]") {
    SECTION("constexpr構築とアクセス") {
        constexpr bluestl::fixed_string<10> str("const");
        static_assert(str.size() == 5);
        static_assert(str.capacity() == 10);
        static_assert(!str.empty());

        // コンパイル時の要素アクセス
        static_assert(str[0] == 'c');
        static_assert(str[4] == 't');
    }
}

/**
 * @brief fixed_stringとstd::string_viewの相互運用テスト
 */
TEST_CASE("fixed_string string_view interop", "[fixed_string]") {
    SECTION("string_viewへの変換") {
        bluestl::fixed_string<10> str("convert");
        std::string_view sv = str.c_str();
        REQUIRE(sv == "convert");
        REQUIRE(sv.size() == 7);
    }

    SECTION("string_viewからの構築と代入") {
        std::string_view sv("from_view");
        bluestl::fixed_string<15> str(sv);
        REQUIRE(str.size() == 9);
        REQUIRE(std::string_view(str.c_str()) == "from_view");

        bluestl::fixed_string<15> str2;
        str2 = sv;
        REQUIRE(str2.size() == 9);
        REQUIRE(std::string_view(str2.c_str()) == "from_view");
    }
}
