// -----------------------------------------------------------------------------
// test_string_view.cpp
// Bluestl string_viewクラスのCatch2テストケース
// -----------------------------------------------------------------------------

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "../include/bluestl/string_view.h"

TEST_CASE("string_view basic constructor tests", "[string_view][constructor]") {
    SECTION("default constructor") {
        bluestl::string_view sv;
        REQUIRE(sv.empty());
        REQUIRE(sv.size() == 0);
        REQUIRE(sv.data() == nullptr);
    }

    SECTION("null pointer constructor") {
        bluestl::string_view sv(nullptr);
        REQUIRE(sv.empty());
        REQUIRE(sv.size() == 0);
        REQUIRE(sv.data() == nullptr);
    }

    SECTION("const char* constructor") {
        const char* str = "hello";
        bluestl::string_view sv(str);
        REQUIRE_FALSE(sv.empty());
        REQUIRE(sv.size() == 5);
        REQUIRE(sv.data() == str);
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv[4] == 'o');
    }

    SECTION("const char* with length constructor") {
        const char* str = "hello world";
        bluestl::string_view sv(str, 5);
        REQUIRE_FALSE(sv.empty());
        REQUIRE(sv.size() == 5);
        REQUIRE(sv.data() == str);
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv[4] == 'o');
    }

    SECTION("copy constructor") {
        const char* str = "test";
        bluestl::string_view sv1(str);
        bluestl::string_view sv2(sv1);
        REQUIRE(sv2.size() == sv1.size());
        REQUIRE(sv2.data() == sv1.data());
        REQUIRE(sv2 == sv1);
    }
}

TEST_CASE("string_view assignment tests", "[string_view][assignment]") {
    const char* str1 = "hello";
    const char* str2 = "world";

    SECTION("copy assignment") {
        bluestl::string_view sv1(str1);
        bluestl::string_view sv2(str2);

        sv2 = sv1;
        REQUIRE(sv2.size() == sv1.size());
        REQUIRE(sv2.data() == sv1.data());
        REQUIRE(sv2 == sv1);
    }

    SECTION("move assignment") {
        bluestl::string_view sv1(str1);
        bluestl::string_view sv2(str2);

        sv2 = std::move(sv1);
        REQUIRE(sv2.size() == 5);
        REQUIRE(sv2.data() == str1);
    }
}

TEST_CASE("string_view iterators", "[string_view][iterators]") {
    const char* str = "hello";
    bluestl::string_view sv(str);

    SECTION("forward iterators") {
        auto it = sv.begin();
        REQUIRE(*it == 'h');
        ++it;
        REQUIRE(*it == 'e');

        size_t count = 0;
        for (auto c : sv) {
            REQUIRE(c == str[count]);
            ++count;
        }
        REQUIRE(count == 5);
    }

    SECTION("const iterators") {
        auto it = sv.cbegin();
        REQUIRE(*it == 'h');
        REQUIRE(sv.cbegin() == sv.begin());
        REQUIRE(sv.cend() == sv.end());
    }

    SECTION("reverse iterators") {
        auto rit = sv.rbegin();
        REQUIRE(*rit == 'o');
        ++rit;
        REQUIRE(*rit == 'l');

        std::string reversed;
        for (auto it = sv.rbegin(); it != sv.rend(); ++it) {
            reversed += *it;
        }
        REQUIRE(reversed == "olleh");
    }
}

TEST_CASE("string_view element access", "[string_view][access]") {
    const char* str = "hello";
    bluestl::string_view sv(str);

    SECTION("operator[]") {
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv[1] == 'e');
        REQUIRE(sv[4] == 'o');
    }

    SECTION("at method") {
        REQUIRE(sv.at(0) == 'h');
        REQUIRE(sv.at(1) == 'e');
        REQUIRE(sv.at(4) == 'o');
    }

    SECTION("front and back") {
        REQUIRE(sv.front() == 'h');
        REQUIRE(sv.back() == 'o');
    }

    SECTION("data method") {
        REQUIRE(sv.data() == str);
        REQUIRE(std::strncmp(sv.data(), "hello", 5) == 0);
    }
}

TEST_CASE("string_view capacity", "[string_view][capacity]") {
    SECTION("empty string") {
        bluestl::string_view sv;
        REQUIRE(sv.empty());
        REQUIRE(sv.size() == 0);
        REQUIRE(sv.length() == 0);
    }

    SECTION("non-empty string") {
        bluestl::string_view sv("test");
        REQUIRE_FALSE(sv.empty());
        REQUIRE(sv.size() == 4);
        REQUIRE(sv.length() == 4);
    }

    SECTION("max_size") {
        bluestl::string_view sv;
        REQUIRE(sv.max_size() > 0);
    }
}

TEST_CASE("string_view modifiers", "[string_view][modifiers]") {
    const char* str = "hello world";

    SECTION("remove_prefix") {
        bluestl::string_view sv(str);
        sv.remove_prefix(6);
        REQUIRE(sv.size() == 5);
        REQUIRE(sv == "world");

        sv.remove_prefix(10);  // より多く削除
        REQUIRE(sv.empty());
    }

    SECTION("remove_suffix") {
        bluestl::string_view sv(str);
        sv.remove_suffix(6);
        REQUIRE(sv.size() == 5);
        REQUIRE(sv == "hello");

        sv.remove_suffix(10);  // より多く削除
        REQUIRE(sv.empty());
    }

    SECTION("swap") {
        bluestl::string_view sv1("hello");
        bluestl::string_view sv2("world");

        sv1.swap(sv2);
        REQUIRE(sv1 == "world");
        REQUIRE(sv2 == "hello");
    }
}

TEST_CASE("string_view substr", "[string_view][substr]") {
    bluestl::string_view sv("hello world");

    SECTION("basic substr") {
        auto sub = sv.substr(0, 5);
        REQUIRE(sub == "hello");
        REQUIRE(sub.size() == 5);
    }

    SECTION("substr with default length") {
        auto sub = sv.substr(6);
        REQUIRE(sub == "world");
        REQUIRE(sub.size() == 5);
    }

    SECTION("substr beyond range") {
        auto sub = sv.substr(20);
        REQUIRE(sub.empty());
    }

    SECTION("substr with excessive length") {
        auto sub = sv.substr(6, 100);
        REQUIRE(sub == "world");
        REQUIRE(sub.size() == 5);
    }
}

TEST_CASE("string_view compare", "[string_view][compare]") {
    bluestl::string_view sv1("hello");
    bluestl::string_view sv2("hello");
    bluestl::string_view sv3("world");
    bluestl::string_view sv4("he");

    SECTION("equal strings") {
        REQUIRE(sv1.compare(sv2) == 0);
    }

    SECTION("different strings") {
        REQUIRE(sv1.compare(sv3) < 0);
        REQUIRE(sv3.compare(sv1) > 0);
    }

    SECTION("different lengths") {
        REQUIRE(sv1.compare(sv4) > 0);
        REQUIRE(sv4.compare(sv1) < 0);
    }

    SECTION("partial compare") {
        REQUIRE(sv1.compare(0, 2, sv4) == 0);
    }
}

TEST_CASE("string_view find operations", "[string_view][find]") {
    bluestl::string_view sv("hello world hello");

    SECTION("find character") {
        REQUIRE(sv.find('h') == 0);
        REQUIRE(sv.find('o') == 4);
        REQUIRE(sv.find('x') == bluestl::string_view::npos);
        REQUIRE(sv.find('h', 1) == 12);
    }

    SECTION("find substring") {
        REQUIRE(sv.find("hello") == 0);
        REQUIRE(sv.find("world") == 6);
        REQUIRE(sv.find("xyz") == bluestl::string_view::npos);
        REQUIRE(sv.find("hello", 1) == 12);
    }

    SECTION("find empty string") {
        REQUIRE(sv.find("") == 0);
        REQUIRE(sv.find("", 5) == 5);
    }

    SECTION("rfind character") {
        REQUIRE(sv.rfind('h') == 12);
        REQUIRE(sv.rfind('o') == 15);
        REQUIRE(sv.rfind('x') == bluestl::string_view::npos);
        REQUIRE(sv.rfind('h', 5) == 0);
    }

    SECTION("rfind substring") {
        REQUIRE(sv.rfind("hello") == 12);
        REQUIRE(sv.rfind("world") == 6);
        REQUIRE(sv.rfind("xyz") == bluestl::string_view::npos);
    }
}

TEST_CASE("string_view starts_with and ends_with", "[string_view][prefix_suffix]") {
    bluestl::string_view sv("hello world");

    SECTION("starts_with string") {
        REQUIRE(sv.starts_with("hello"));
        REQUIRE(sv.starts_with(""));
        REQUIRE_FALSE(sv.starts_with("world"));
        REQUIRE_FALSE(sv.starts_with("hello world!"));
    }

    SECTION("starts_with character") {
        REQUIRE(sv.starts_with('h'));
        REQUIRE_FALSE(sv.starts_with('w'));
    }

    SECTION("ends_with string") {
        REQUIRE(sv.ends_with("world"));
        REQUIRE(sv.ends_with(""));
        REQUIRE_FALSE(sv.ends_with("hello"));
        REQUIRE_FALSE(sv.ends_with("!hello world"));
    }

    SECTION("ends_with character") {
        REQUIRE(sv.ends_with('d'));
        REQUIRE_FALSE(sv.ends_with('h'));
    }

    SECTION("empty string_view") {
        bluestl::string_view empty;
        REQUIRE(empty.starts_with(""));
        REQUIRE(empty.ends_with(""));
        REQUIRE_FALSE(empty.starts_with("a"));
        REQUIRE_FALSE(empty.ends_with("a"));
    }
}

TEST_CASE("string_view contains", "[string_view][contains]") {
    bluestl::string_view sv("hello world");

    SECTION("contains character") {
        REQUIRE(sv.contains('h'));
        REQUIRE(sv.contains('o'));
        REQUIRE(sv.contains(' '));
        REQUIRE_FALSE(sv.contains('x'));
    }

    SECTION("contains substring") {
        REQUIRE(sv.contains("hello"));
        REQUIRE(sv.contains("world"));
        REQUIRE(sv.contains("o w"));
        REQUIRE(sv.contains(""));
        REQUIRE_FALSE(sv.contains("xyz"));
        REQUIRE_FALSE(sv.contains("hello world!"));
    }
}

TEST_CASE("string_view comparison operators", "[string_view][operators]") {
    bluestl::string_view sv1("abc");
    bluestl::string_view sv2("abc");
    bluestl::string_view sv3("def");
    bluestl::string_view sv4("ab");

    SECTION("equality operators") {
        REQUIRE(sv1 == sv2);
        REQUIRE_FALSE(sv1 == sv3);
        REQUIRE_FALSE(sv1 == sv4);

        REQUIRE_FALSE(sv1 != sv2);
        REQUIRE(sv1 != sv3);
        REQUIRE(sv1 != sv4);
    }

    SECTION("ordering operators") {
        REQUIRE(sv1 < sv3);
        REQUIRE_FALSE(sv3 < sv1);
        REQUIRE_FALSE(sv1 < sv2);

        REQUIRE(sv1 <= sv2);
        REQUIRE(sv1 <= sv3);
        REQUIRE_FALSE(sv3 <= sv1);

        REQUIRE(sv3 > sv1);
        REQUIRE_FALSE(sv1 > sv3);
        REQUIRE_FALSE(sv1 > sv2);

        REQUIRE(sv1 >= sv2);
        REQUIRE(sv3 >= sv1);
        REQUIRE_FALSE(sv1 >= sv3);
    }

    SECTION("comparison with different lengths") {
        REQUIRE(sv4 < sv1);  // "ab" < "abc"
        REQUIRE(sv1 > sv4);  // "abc" > "ab"
    }
}

TEST_CASE("string_view edge cases", "[string_view][edge_cases]") {
    SECTION("empty string operations") {
        bluestl::string_view empty;

        REQUIRE(empty.substr(0, 1).empty());
        REQUIRE(empty.find('a') == bluestl::string_view::npos);
        REQUIRE(empty.rfind('a') == bluestl::string_view::npos);
        REQUIRE_FALSE(empty.contains('a'));
    }

    SECTION("single character string") {
        bluestl::string_view single("a");

        REQUIRE(single.size() == 1);
        REQUIRE(single.front() == 'a');
        REQUIRE(single.back() == 'a');
        REQUIRE(single[0] == 'a');
        REQUIRE(single.find('a') == 0);
        REQUIRE(single.rfind('a') == 0);
        REQUIRE(single.contains('a'));
        REQUIRE_FALSE(single.contains('b'));
    }

    SECTION("large position values") {
        bluestl::string_view sv("test");

        REQUIRE(sv.find('t', 1000) == bluestl::string_view::npos);
        REQUIRE(sv.substr(1000).empty());

        sv.remove_prefix(1000);
        REQUIRE(sv.empty());
    }
}

TEST_CASE("string_view with embedded nulls", "[string_view][embedded_nulls]") {
    const char data[] = { 'h', 'e', 'l', '\0', 'l', 'o' };
    bluestl::string_view sv(data, 6);

    SECTION("size and access") {
        REQUIRE(sv.size() == 6);
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv[3] == '\0');
        REQUIRE(sv[5] == 'o');
    }

    SECTION("find operations with embedded null") {
        REQUIRE(sv.find('\0') == 3);
        REQUIRE(sv.find('o') == 5);
    }

    SECTION("substr with embedded null") {
        auto sub = sv.substr(0, 3);
        REQUIRE(sub.size() == 3);
        REQUIRE(sub[2] == 'l');

        auto sub2 = sv.substr(4);
        REQUIRE(sub2.size() == 2);
        REQUIRE(sub2[0] == 'l');
        REQUIRE(sub2[1] == 'o');
    }
}

TEST_CASE("string_view non-member swap", "[string_view][swap]") {
    bluestl::string_view sv1("hello");
    bluestl::string_view sv2("world");

    bluestl::swap(sv1, sv2);

    REQUIRE(sv1 == "world");
    REQUIRE(sv2 == "hello");
}

TEST_CASE("string_view const correctness", "[string_view][const]") {
    const char* str = "hello";
    const bluestl::string_view sv(str);

    SECTION("const methods") {
        REQUIRE(sv.size() == 5);
        REQUIRE(sv.empty() == false);
        REQUIRE(sv.data() == str);
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv.at(0) == 'h');
        REQUIRE(sv.front() == 'h');
        REQUIRE(sv.back() == 'o');
    }

    SECTION("const iterators") {
        auto it = sv.begin();
        REQUIRE(*it == 'h');

        auto cit = sv.cbegin();
        REQUIRE(*cit == 'h');

        auto rit = sv.rbegin();
        REQUIRE(*rit == 'o');
    }
}
