/**
 * @file test_string_catch2.cpp
 * @brief string クラスのCatch2テストコード
 * @details bluestl::string の全機能をCatch2フレームワークでテストする
 */

#include <catch2/catch_test_macros.hpp>
#include "../include/bluestl/string.h"
#include <string>

TEST_CASE("string constructors", "[string]") {
    SECTION("default constructor") {
        bluestl::basic_string str;
        REQUIRE(str.empty());
        REQUIRE(str.size() == 0);
        REQUIRE(str.c_str()[0] == '\0');
    }

    SECTION("C-string constructor") {
        bluestl::basic_string str("hello");
        REQUIRE(str.size() == 5);
        REQUIRE(std::string(str.c_str()) == "hello");
    }

    SECTION("fill constructor") {
        bluestl::basic_string str(10, 'a');
        REQUIRE(str.size() == 10);
        REQUIRE(std::string(str.c_str()) == "aaaaaaaaaa");
    }

    SECTION("copy constructor") {
        bluestl::basic_string original("hello");
        bluestl::basic_string copy(original);
        REQUIRE(copy.size() == 5);
        REQUIRE(std::string(copy.c_str()) == "hello");
    }

    SECTION("substring constructor") {
        bluestl::basic_string original("hello world");
        bluestl::basic_string sub(original, 6, 5);
        REQUIRE(sub.size() == 5);
        REQUIRE(std::string(sub.c_str()) == "world");
    }
}

TEST_CASE("string assignment", "[string]") {
    bluestl::basic_string str;

    SECTION("copy assignment") {
        bluestl::basic_string other("world");
        str = other;
        REQUIRE(str.size() == 5);
        REQUIRE(std::string(str.c_str()) == "world");
    }

    SECTION("C-string assignment") {
        str = "hello";
        REQUIRE(str.size() == 5);
        REQUIRE(std::string(str.c_str()) == "hello");
    }

    SECTION("character assignment") {
        str = 'x';
        REQUIRE(str.size() == 1);
        REQUIRE(std::string(str.c_str()) == "x");
    }
}

TEST_CASE("string element access", "[string]") {
    bluestl::basic_string str("hello");

    SECTION("at method") {
        REQUIRE(str.at(0) == 'h');
        REQUIRE(str.at(4) == 'o');
    }

    SECTION("subscript operator") {
        REQUIRE(str[0] == 'h');
        REQUIRE(str[4] == 'o');
    }

    SECTION("front and back") {
        REQUIRE(str.front() == 'h');
        REQUIRE(str.back() == 'o');
    }

    SECTION("data and c_str") {
        REQUIRE(str.data()[0] == 'h');
        REQUIRE(str.c_str()[0] == 'h');
        REQUIRE(str.c_str()[5] == '\0');
    }
}

TEST_CASE("string iterators", "[string]") {
    bluestl::basic_string str("hello");

    SECTION("forward iteration") {
        std::string result;
        for (auto it = str.begin(); it != str.end(); ++it) {
            result += *it;
        }
        REQUIRE(result == "hello");
    }

    SECTION("reverse iteration") {
        std::string result;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            result += *it;
        }
        REQUIRE(result == "olleh");
    }

    SECTION("range-based for") {
        std::string result;
        for (char c : str) {
            result += c;
        }
        REQUIRE(result == "hello");
    }
}

TEST_CASE("string capacity", "[string]") {
    bluestl::basic_string str;

    SECTION("empty string") {
        REQUIRE(str.empty());
        REQUIRE(str.size() == 0);
        REQUIRE(str.length() == 0);
    }

    SECTION("reserve") {
        str.reserve(100);
        REQUIRE(str.capacity() >= 100);
        REQUIRE(str.empty());
    }

    SECTION("non-empty string") {
        str = "hello";
        REQUIRE_FALSE(str.empty());
        REQUIRE(str.size() == 5);
        REQUIRE(str.length() == 5);
    }
}

TEST_CASE("string modifiers", "[string]") {
    bluestl::basic_string str;

    SECTION("clear") {
        str = "hello";
        str.clear();
        REQUIRE(str.empty());
    }

    SECTION("push_back and pop_back") {
        str.push_back('a');
        str.push_back('b');
        REQUIRE(str.size() == 2);
        REQUIRE(std::string(str.c_str()) == "ab");

        str.pop_back();
        REQUIRE(str.size() == 1);
        REQUIRE(std::string(str.c_str()) == "a");
    }

    SECTION("append") {
        str.append("hello");
        str.append(" world");
        REQUIRE(std::string(str.c_str()) == "hello world");
    }

    SECTION("resize") {
        str = "hello world";
        str.resize(5);
        REQUIRE(str.size() == 5);
        REQUIRE(std::string(str.c_str()) == "hello");

        str.resize(10, 'x');
        REQUIRE(str.size() == 10);
        REQUIRE(std::string(str.c_str()) == "helloxxxxx");
    }
}

TEST_CASE("string operations", "[string]") {
    bluestl::basic_string str("hello world");

    SECTION("substr") {
        auto sub = str.substr(0, 5);
        REQUIRE(std::string(sub.c_str()) == "hello");

        sub = str.substr(6);
        REQUIRE(std::string(sub.c_str()) == "world");
    }

    SECTION("find") {
        REQUIRE(str.find("world") == 6);
        REQUIRE(str.find("xyz") == bluestl::basic_string::npos);
        REQUIRE(str.find('o') == 4);
    }

    SECTION("starts_with") {
        REQUIRE(str.starts_with("hello"));
        REQUIRE_FALSE(str.starts_with("world"));
    }

    SECTION("ends_with") {
        REQUIRE(str.ends_with("world"));
        REQUIRE_FALSE(str.ends_with("hello"));
    }

    SECTION("contains") {
        REQUIRE(str.contains("llo"));
        REQUIRE_FALSE(str.contains("xyz"));
    }
}

TEST_CASE("string comparison", "[string]") {
    bluestl::basic_string str1("abc");
    bluestl::basic_string str2("abc");
    bluestl::basic_string str3("def");

    SECTION("equality") {
        REQUIRE(str1 == str2);
        REQUIRE_FALSE(str1 == str3);
        REQUIRE(str1 != str3);
        REQUIRE_FALSE(str1 != str2);
    }

    SECTION("ordering") {
        REQUIRE(str1 < str3);
        REQUIRE(str1 <= str2);
        REQUIRE(str3 > str1);
        REQUIRE(str2 >= str1);
    }

    SECTION("C-string comparison") {
        REQUIRE(str1 == "abc");
        REQUIRE("abc" == str1);
        REQUIRE(str1 != "def");
        REQUIRE("def" != str1);
    }
}

TEST_CASE("string performance", "[string]") {
    bluestl::basic_string str;

    SECTION("bulk character addition") {
        for (int i = 0; i < 1000; ++i) {
            str.push_back('a');
        }
        REQUIRE(str.size() == 1000);
    }

    SECTION("bulk string append") {
        str.clear();
        for (int i = 0; i < 100; ++i) {
            str.append("hello");
        }
        REQUIRE(str.size() == 500);
    }
}
