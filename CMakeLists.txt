cmake_minimum_required(VERSION 3.15)
project(BlueStl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ソースファイルとヘッダファイルの追加
include_directories(include)

# テスト用の実行ファイル
file(GLOB TEST_SOURCES tests/*.cpp)
add_executable(test_all ${TEST_SOURCES})
