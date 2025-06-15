cmake_minimum_required(VERSION 3.15)
project(BlueStl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ソースファイルとヘッダファイルの追加
include_directories(include)

# オプション定義
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_BENCHMARKS "Enable benchmark tests" OFF)

# カバレッジ設定
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# サニタイザ設定
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

include(FetchContent)

# Catch2フレームワークのセットアップ
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1  # 使用したいバージョンを指定
)
FetchContent_MakeAvailable(Catch2)

# Google Benchmarkフレームワークのセットアップ（オプション）
if(ENABLE_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.5
    )
    FetchContent_MakeAvailable(benchmark)
endif()

# テスト用の実行ファイル
file(GLOB TEST_SOURCES tests/*.cpp tests/*.h include/**/*.h)

add_executable(test_all ${TEST_SOURCES})
target_link_libraries(test_all PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_all PRIVATE include)

# ドキュメント生成の設定
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    # Doxyfileを使用してドキュメント生成
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "APIドキュメントを生成中..."
        VERBATIM
    )
    
    message(STATUS "Doxygen found: ドキュメント生成ターゲット 'docs' が利用可能です")
    message(STATUS "使用方法: cmake --build . --target docs")
else()
    message(STATUS "Doxygen not found: ドキュメント生成は利用できません")
endif()

# ベンチマークテスト用の実行ファイル（オプション）
if(ENABLE_BENCHMARKS)
    file(GLOB BENCHMARK_SOURCES benchmarks/*.cpp include/**/*.h)
    if(BENCHMARK_SOURCES)
        add_executable(benchmark_all ${BENCHMARK_SOURCES})
        target_link_libraries(benchmark_all PRIVATE benchmark::benchmark)
        target_include_directories(benchmark_all PRIVATE include)
    endif()
endif()

# カバレッジターゲット
if(ENABLE_COVERAGE)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test_all
            COMMAND ${CMAKE_CTEST_COMMAND} --verbose
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info 
                '/usr/*' '*/tests/*' '*/build/*' '*/Catch2/*' '*/benchmark/*'
                --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()

# CTestの有効化
enable_testing()
add_test(NAME bluestl_tests COMMAND test_all)
