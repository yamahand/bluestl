// filepath: c:\Users\a1829\source\blueStl\include\stl\log_macros.h
#pragma once

#include "log_interface.h"   // ログ機能
#include "assert_handler.h"  // アサート機能

// ログマクロ
#ifndef CONTAINER_LOGF
#define CONTAINER_LOGF(level, fmt, ...) ::bluestl::logf(level, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

// 両方のアサートマクロ形式をサポート
// assert_handler.h で既にCONTAINER_ASSERTが定義されているため、ここでは再定義しない
// BLUESTL_ASSERT は assert_handler.h で既に定義されている
