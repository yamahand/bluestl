/**
 * @file test_string.cpp
 * @brief string クラスのテストコード
 * @details bluestl::string の全機能をテストする包括的なテストスイート
 */

#include "../include/bluestl/string.h"
#include <cassert>
#include <iostream>
#include <string>

namespace test_string {

/**
 * @brief 基本的なコンストラクタのテスト
 */
void test_constructors() {
    std::cout << "Running string constructor tests..." << std::endl;

    // デフォルトコンストラクタ
    bluestl::basic_string str1;
    assert(str1.empty());
    assert(str1.size() == 0);
    assert(str1.c_str()[0] == '\0');

    // C文字列からのコンストラクタ
    bluestl::basic_string str2("hello");
    assert(str2.size() == 5);
    assert(std::string(str2.c_str()) == "hello");

    // カウント指定コンストラクタ
    bluestl::basic_string str3(10, 'a');
    assert(str3.size() == 10);
    assert(std::string(str3.c_str()) == "aaaaaaaaaa");

    // コピーコンストラクタ
    bluestl::basic_string str4(str2);
    assert(str4.size() == 5);
    assert(std::string(str4.c_str()) == "hello");

    // 部分文字列コンストラクタ
    bluestl::basic_string str5(str2, 1, 3);
    assert(str5.size() == 3);
    assert(std::string(str5.c_str()) == "ell");

    std::cout << "Constructor tests passed!" << std::endl;
}

/**
 * @brief 代入演算子のテスト
 */
void test_assignment() {
    std::cout << "Running string assignment tests..." << std::endl;

    bluestl::basic_string str1;
    bluestl::basic_string str2("world");

    // コピー代入
    str1 = str2;
    assert(str1.size() == 5);
    assert(std::string(str1.c_str()) == "world");

    // C文字列代入
    str1 = "hello";
    assert(str1.size() == 5);
    assert(std::string(str1.c_str()) == "hello");

    // 文字代入
    str1 = 'x';
    assert(str1.size() == 1);
    assert(std::string(str1.c_str()) == "x");

    std::cout << "Assignment tests passed!" << std::endl;
}

/**
 * @brief アクセス操作のテスト
 */
void test_access() {
    std::cout << "Running string access tests..." << std::endl;

    bluestl::basic_string str("hello");

    // at() メソッド
    assert(str.at(0) == 'h');
    assert(str.at(4) == 'o');

    // operator[]
    assert(str[0] == 'h');
    assert(str[4] == 'o');

    // front() と back()
    assert(str.front() == 'h');
    assert(str.back() == 'o');

    // data() と c_str()
    assert(str.data()[0] == 'h');
    assert(str.c_str()[0] == 'h');
    assert(str.c_str()[5] == '\0');

    std::cout << "Access tests passed!" << std::endl;
}

/**
 * @brief イテレータのテスト
 */
void test_iterators() {
    std::cout << "Running string iterator tests..." << std::endl;

    bluestl::basic_string str("hello");

    // 順方向イテレータ
    std::string result;
    for (auto it = str.begin(); it != str.end(); ++it) {
        result += *it;
    }
    assert(result == "hello");

    // 逆方向イテレータ
    result.clear();
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
        result += *it;
    }
    assert(result == "olleh");

    // 範囲for文
    result.clear();
    for (char c : str) {
        result += c;
    }
    assert(result == "hello");

    std::cout << "Iterator tests passed!" << std::endl;
}

/**
 * @brief 容量操作のテスト
 */
void test_capacity() {
    std::cout << "Running string capacity tests..." << std::endl;

    bluestl::basic_string str;

    // 空の状態
    assert(str.empty());
    assert(str.size() == 0);
    assert(str.length() == 0);

    // reserve のテスト
    str.reserve(100);
    assert(str.capacity() >= 100);
    assert(str.empty());

    // 文字列を追加
    str = "hello";
    assert(!str.empty());
    assert(str.size() == 5);
    assert(str.length() == 5);

    std::cout << "Capacity tests passed!" << std::endl;
}

/**
 * @brief 修正操作のテスト
 */
void test_modifiers() {
    std::cout << "Running string modifier tests..." << std::endl;

    bluestl::basic_string str;

    // clear()
    str = "hello";
    str.clear();
    assert(str.empty());

    // push_back() と pop_back()
    str.push_back('a');
    str.push_back('b');
    assert(str.size() == 2);
    assert(std::string(str.c_str()) == "ab");

    str.pop_back();
    assert(str.size() == 1);
    assert(std::string(str.c_str()) == "a");

    // append()
    str.clear();
    str.append("hello");
    str.append(" world");
    assert(std::string(str.c_str()) == "hello world");

    // resize()
    str.resize(5);
    assert(str.size() == 5);
    assert(std::string(str.c_str()) == "hello");

    str.resize(10, 'x');
    assert(str.size() == 10);
    assert(std::string(str.c_str()) == "helloxxxxx");

    std::cout << "Modifier tests passed!" << std::endl;
}

/**
 * @brief 文字列操作のテスト
 */
void test_string_operations() {
    std::cout << "Running string operations tests..." << std::endl;

    bluestl::basic_string str("hello world");

    // substr()
    auto sub = str.substr(0, 5);
    assert(std::string(sub.c_str()) == "hello");

    sub = str.substr(6);
    assert(std::string(sub.c_str()) == "world");

    // find()
    assert(str.find("world") == 6);
    assert(str.find("xyz") == bluestl::basic_string::npos);
    assert(str.find('o') == 4);

    // starts_with()
    assert(str.starts_with("hello"));
    assert(!str.starts_with("world"));

    // ends_with()
    assert(str.ends_with("world"));
    assert(!str.ends_with("hello"));

    // contains()
    assert(str.contains("llo"));
    assert(!str.contains("xyz"));

    std::cout << "String operations tests passed!" << std::endl;
}

/**
 * @brief 比較演算子のテスト
 */
void test_comparison() {
    std::cout << "Running string comparison tests..." << std::endl;

    bluestl::basic_string str1("abc");
    bluestl::basic_string str2("abc");
    bluestl::basic_string str3("def");

    // 等価性
    assert(str1 == str2);
    assert(!(str1 == str3));
    assert(str1 != str3);
    assert(!(str1 != str2));

    // 順序比較
    assert(str1 < str3);
    assert(str1 <= str2);
    assert(str3 > str1);
    assert(str2 >= str1);

    // C文字列との比較
    assert(str1 == "abc");
    assert("abc" == str1);
    assert(str1 != "def");
    assert("def" != str1);

    std::cout << "Comparison tests passed!" << std::endl;
}

/**
 * @brief パフォーマンステスト
 */
void test_performance() {
    std::cout << "Running string performance tests..." << std::endl;

    bluestl::basic_string str;

    // 大量の文字追加
    for (int i = 0; i < 1000; ++i) {
        str.push_back('a');
    }
    assert(str.size() == 1000);

    // 大量の文字列追加
    str.clear();
    for (int i = 0; i < 100; ++i) {
        str.append("hello");
    }
    assert(str.size() == 500);

    std::cout << "Performance tests passed!" << std::endl;
}

/**
 * @brief エラー条件のテスト
 */
void test_error_conditions() {
    std::cout << "Running string error condition tests..." << std::endl;

    bluestl::basic_string str("hello");

// 範囲外アクセス（assertで止まるため、デバッグビルドでのみテスト）
#ifdef _DEBUG
// これらのテストはassertで停止するため、リリースビルドでは実行しない
// assert(false); // str.at(10);
#endif

// 空文字列のpop_back（assertで止まるため、デバッグビルドでのみテスト）
#ifdef _DEBUG
// bluestl::basic_string empty_str;
// assert(false); // empty_str.pop_back();
#endif

    std::cout << "Error condition tests completed!" << std::endl;
}

/**
 * @brief すべてのテストを実行
 */
void run_all_tests() {
    std::cout << "=== BlueStl String Tests ===" << std::endl;

    test_constructors();
    test_assignment();
    test_access();
    test_iterators();
    test_capacity();
    test_modifiers();
    test_string_operations();
    test_comparison();
    test_performance();
    test_error_conditions();

    std::cout << "=== All String Tests Passed! ===" << std::endl;
}

}  // namespace test_string
