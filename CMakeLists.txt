cmake_minimum_required(VERSION 3.15)
project(BlueStl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ソースファイルとヘッダファイルの追加
include_directories(include)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1  # 使用したいバージョンを指定
)
FetchContent_MakeAvailable(Catch2)


# テスト用の実行ファイル
file(GLOB TEST_SOURCES tests/*.cpp tests/*.h include/**/*.h)

add_executable(test_all ${TEST_SOURCES})
target_link_libraries(test_all PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_all PRIVATE include)
