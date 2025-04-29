#pragma once

#include <cstdlib>
#include <cstdio>

namespace bluestl {

// アサートが失敗した時に呼び出される関数のシグネチャ
using assert_handler_t = void (*)(const char* condition, const char* file, int line, const char* msg);

// 互換性のために古いシグネチャも定義
using AssertHandlerFn = void (*)(const char* expr, const char* file, int line);

// デフォルトのアサートハンドラ
inline void default_assert_handler(const char* condition, const char* file, int line, const char* msg) {
    // 標準出力にメッセージを表示
    if (msg && *msg) {
        std::fprintf(stderr, "Assertion failed: %s (%s:%d) - %s\n", condition, file, line, msg);
    } else {
        std::fprintf(stderr, "Assertion failed: %s (%s:%d)\n", condition, file, line);
    }
    // 標準のアボート動作
    std::abort();
}

// 現在のアサートハンドラ
inline assert_handler_t current_assert_handler = default_assert_handler;

// 後方互換性のためのハンドラ
inline AssertHandlerFn g_assert_fn = nullptr;

// アサートハンドラの設定関数
inline assert_handler_t set_assert_handler(assert_handler_t new_handler) {
    assert_handler_t old_handler = current_assert_handler;
    current_assert_handler = new_handler;
    return old_handler;
}

// 後方互換性のためのセット関数
inline void set_assert_handler(AssertHandlerFn fn) {
    g_assert_fn = fn;
    // 新しいハンドラにブリッジする
    if (fn) {
        set_assert_handler([](const char* condition, const char* file, int line, const char* /*msg*/) {
            g_assert_fn(condition, file, line);
        });
    }
}

// log_interface.h の assert_fail 関数と互換性を持たせる
[[noreturn]] inline void assert_fail(const char* expr, const char* file, int line) {
    if (g_assert_fn) {
        g_assert_fn(expr, file, line);
    } else {
        current_assert_handler(expr, file, line, nullptr);
    }
    // 念のため（通常はcurrent_assert_handlerの中でabortされる）
    std::abort();
}

// アサートマクロの実装
#define BLUESTL_ASSERT_IMPL(cond, msg)                                       \
    do {                                                                     \
        if (!(cond)) {                                                       \
            bluestl::current_assert_handler(#cond, __FILE__, __LINE__, msg); \
        }                                                                    \
    } while (0)

// メッセージ付きアサート
#define BLUESTL_ASSERT_MSG(cond, msg) BLUESTL_ASSERT_IMPL(cond, msg)

// シンプルなアサート
#define BLUESTL_ASSERT(cond) BLUESTL_ASSERT_IMPL(cond, nullptr)

}  // namespace bluestl
