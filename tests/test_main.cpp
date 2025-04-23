#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "bluestl/assert_handler.h"

// BlueStlのメインテストファイル
// Catch2 v3を使用してテストを実行する

// テスト用のカスタムアサートハンドラ
// このハンドラは標準エラーにメッセージを出力し、Catch2がキャッチできるように例外をスローする代わりに
// 特定のエラーコードで終了します
void test_assert_handler(const char* condition, const char* file, int line, const char* msg) {
    std::cerr << "\nアサート失敗: " << (condition ? condition : "不明な条件") 
              << " [ファイル: " << (file ? file : "不明") 
              << ", 行: " << line << "]";
    
    if (msg && *msg) {
        std::cerr << "\nメッセージ: " << msg;
    }
    std::cerr << std::endl;
    
}

// テスト実行前にカスタムアサートハンドラを設定
struct AssertHandlerSetter {
    AssertHandlerSetter() {
        // テスト開始前にカスタムアサートハンドラを設定
        bluestl::set_assert_handler(test_assert_handler);
        std::cout << "カスタムアサートハンドラを設定しました。" << std::endl;
    }
};

// グローバルインスタンスを作成してテスト実行前に初期化されるようにする
static AssertHandlerSetter g_assert_handler_setter;
