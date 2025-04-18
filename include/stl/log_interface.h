#pragma once

#include <string_view>
#include <string>
#include <format>
#include <source_location>
#include <cstdio>
#include <cstdlib>

namespace bluestl {

enum class LogLevel {
    Info,
    Warn,
    Error,
    Debug,
};

using FormattedLogFn = void(*)(LogLevel level, std::string_view message);
using AssertHandlerFn = void(*)(const char* expr, const char* file, int line);

// 内部静的関数ポインタ（inline変数でヘッダオンリー化）
inline FormattedLogFn g_log_fn = nullptr;
inline AssertHandlerFn g_assert_fn = nullptr;

inline void set_log_function(FormattedLogFn fn) {
    g_log_fn = fn;
}

inline void set_assert_handler(AssertHandlerFn fn) {
    g_assert_fn = fn;
}

inline void log(LogLevel level, std::string_view msg) {
    if (g_log_fn) {
        g_log_fn(level, msg);
    } else {
        std::fprintf(stderr, "[%d] %.*s\n", static_cast<int>(level), static_cast<int>(msg.size()), msg.data());
    }
}

template<typename... Args>
inline void logf(
    LogLevel level,
    const std::source_location loc,
    std::format_string<Args...> fmt,
    Args&&... args
) {
    std::string message = std::format(fmt, std::forward<Args>(args)...);
    std::string with_loc = std::format("[{}:{}] {}", loc.file_name(), loc.line(), message);
    log(level, with_loc);
}

[[noreturn]] inline void assert_fail(const char* expr, const char* file, int line) {
    if (g_assert_fn) {
        g_assert_fn(expr, file, line);
    } else {
        std::fprintf(stderr, "Assertion failed: %s (%s:%d)\n", expr, file, line);
        std::abort();
    }
}

} // namespace fixed_vector
