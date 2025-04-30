// filepath: c:\Users\a1829\source\blueStl\include\stl\log_macros.h
#pragma once

#include "log_interface.h"   // ログ機能
#include "assert_handler.h"  // アサート機能

// ログマクロ
#ifndef CONTAINER_LOGF
#define CONTAINER_LOGF(level, fmt, ...) ::bluestl::logf(level, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

#ifndef BLUESTL_LOG_ERROR
#define BLUESTL_LOG_ERROR(fmt, ...) ::bluestl::logf(::bluestl::LogLevel::Error, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

#ifndef BLUESTL_LOG_DEBUG
#define BLUESTL_LOG_DEBUG(fmt, ...) ::bluestl::logf(::bluestl::LogLevel::Debug, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

#ifndef BLUESTL_LOG_WARN
#define BLUESTL_LOG_WARN(fmt, ...) ::bluestl::logf(::bluestl::LogLevel::Warn, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

#ifndef BLUESTL_LOG_INFO
#define BLUESTL_LOG_INFO(fmt, ...) ::bluestl::logf(::bluestl::LogLevel::Info, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif


// 両方のアサートマクロ形式をサポート
// assert_handler.h で既にCONTAINER_ASSERTが定義されているため、ここでは再定義しない
// BLUESTL_ASSERT は assert_handler.h で既に定義されている
