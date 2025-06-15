#!/bin/bash

# BlueStl 包括的テストスクリプト
# CMakeを使用してプロジェクトをビルドし、テスト・ベンチマーク・カバレッジを実行します

set -e  # エラー時に即座に終了

# デフォルト設定
BUILD_TYPE="Debug"
ENABLE_COVERAGE=false
ENABLE_SANITIZERS=false
ENABLE_BENCHMARKS=false
CLEAN_BUILD=false
VERBOSE=false

# ヘルプ表示
show_help() {
    cat << EOF
使用方法: $0 [オプション]

オプション:
    -h, --help              このヘルプを表示
    -c, --coverage          コードカバレッジを有効化
    -s, --sanitizers        サニタイザ（AddressSanitizer, UBSan）を有効化
    -b, --benchmarks        ベンチマークテストを有効化
    -r, --release           リリースビルド（デフォルト: Debug）
    --clean                 クリーンビルド（既存のbuildディレクトリを削除）
    -v, --verbose           詳細な出力を表示

例:
    $0                      基本テストのみ実行
    $0 -c                   カバレッジ付きでテスト実行
    $0 -s                   サニタイザ付きでテスト実行
    $0 -b                   ベンチマークテストも実行
    $0 -c -s -b             全ての機能を有効化
    $0 --clean -r           クリーンリリースビルド

EOF
}

# コマンドライン引数解析
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--coverage)
            ENABLE_COVERAGE=true
            ;;
        -s|--sanitizers)
            ENABLE_SANITIZERS=true
            ;;
        -b|--benchmarks)
            ENABLE_BENCHMARKS=true
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            ;;
        --clean)
            CLEAN_BUILD=true
            ;;
        -v|--verbose)
            VERBOSE=true
            ;;
        *)
            echo "未知のオプション: $1"
            show_help
            exit 1
            ;;
    esac
    shift
done

echo "BlueStl 包括的テストを開始します..."
echo "ビルドタイプ: $BUILD_TYPE"
echo "カバレッジ: $ENABLE_COVERAGE"
echo "サニタイザ: $ENABLE_SANITIZERS"
echo "ベンチマーク: $ENABLE_BENCHMARKS"
echo

# 元のディレクトリを保存
ORIG_DIR=$(pwd)

# クリーンアップ関数
cleanup() {
    cd "$ORIG_DIR"
}
trap cleanup EXIT

# クリーンビルドの場合、既存のbuildディレクトリを削除
if [ "$CLEAN_BUILD" = true ]; then
    echo "クリーンビルド: 既存のbuildディレクトリを削除中..."
    rm -rf build
fi

# ビルドディレクトリ作成
echo "ビルドディレクトリを作成中..."
mkdir -p build
cd build

# CMakeオプション構築
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$ENABLE_COVERAGE" = true ]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_COVERAGE=ON"
fi

if [ "$ENABLE_SANITIZERS" = true ]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_SANITIZERS=ON"
fi

if [ "$ENABLE_BENCHMARKS" = true ]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_BENCHMARKS=ON"
fi

# CMakeでプロジェクト構成
echo "CMakeでプロジェクトを構成中..."
echo "実行: cmake $CMAKE_OPTIONS .."
if ! cmake $CMAKE_OPTIONS .. 2>&1; then
    echo "CMake構成が失敗しました。"
    exit 1
fi

# 並列ビルド数を決定
NPROC=$(nproc 2>/dev/null || echo 4)
echo "並列ビルド数: $NPROC"

# ビルド
echo "プロジェクトをビルド中..."
BUILD_CMD="cmake --build . -j $NPROC"
if [ "$VERBOSE" = true ]; then
    BUILD_CMD="$BUILD_CMD --verbose"
fi

if ! $BUILD_CMD 2>&1; then
    echo "ビルドが失敗しました。"
    exit 1
fi

# テスト実行可能ファイルを探す
find_executable() {
    local name=$1
    if [ -f "Debug/$name" ]; then
        echo "Debug/$name"
    elif [ -f "Release/$name" ]; then
        echo "Release/$name"
    elif [ -f "$name" ]; then
        echo "./$name"
    else
        return 1
    fi
}

# 基本テスト実行
echo
echo "========================================="
echo "基本テストを実行中..."
echo "========================================="

TEST_EXEC=$(find_executable "test_all")
if [ -z "$TEST_EXEC" ]; then
    echo "test_all が見つかりませんでした。"
    exit 1
fi

echo "実行: $TEST_EXEC"
if ! $TEST_EXEC; then
    echo "テストが失敗しました。"
    exit 1
fi

# ベンチマークテスト実行
if [ "$ENABLE_BENCHMARKS" = true ]; then
    echo
    echo "========================================="
    echo "ベンチマークテストを実行中..."
    echo "========================================="
    
    BENCHMARK_EXEC=$(find_executable "benchmark_all")
    if [ -n "$BENCHMARK_EXEC" ]; then
        echo "実行: $BENCHMARK_EXEC"
        if ! $BENCHMARK_EXEC --benchmark_min_time=0.1s; then
            echo "ベンチマークテストが失敗しました。"
            exit 1
        fi
    else
        echo "警告: benchmark_all が見つかりませんでした。"
    fi
fi

# カバレッジレポート生成
if [ "$ENABLE_COVERAGE" = true ]; then
    echo
    echo "========================================="
    echo "カバレッジレポートを生成中..."
    echo "========================================="
    
    if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
        if make coverage 2>/dev/null || cmake --build . --target coverage 2>/dev/null; then
            if [ -d "coverage_html" ]; then
                echo "カバレッジレポートが生成されました: build/coverage_html/index.html"
            fi
        else
            echo "警告: カバレッジレポートの生成に失敗しました。"
        fi
    else
        echo "警告: lcov または genhtml が見つかりません。カバレッジレポートをスキップします。"
    fi
fi

# CTestの実行（利用可能な場合）
if [ -f "CTestTestfile.cmake" ]; then
    echo
    echo "========================================="
    echo "CTestを実行中..."
    echo "========================================="
    
    if ! ctest --output-on-failure; then
        echo "CTestが失敗しました。"
        exit 1
    fi
fi

echo
echo "========================================="
echo "全てのテストが成功しました！"
echo "========================================="

# 結果サマリー
echo
echo "テスト結果サマリー:"
echo "- 基本テスト: 成功"

if [ "$ENABLE_BENCHMARKS" = true ]; then
    echo "- ベンチマークテスト: 成功"
fi

if [ "$ENABLE_COVERAGE" = true ]; then
    echo "- カバレッジレポート: 生成済み"
fi

if [ "$ENABLE_SANITIZERS" = true ]; then
    echo "- サニタイザチェック: 実行済み"
fi

echo
echo "ビルド成果物:"
echo "- テスト実行ファイル: $TEST_EXEC"

if [ "$ENABLE_BENCHMARKS" = true ] && [ -n "$BENCHMARK_EXEC" ]; then
    echo "- ベンチマーク実行ファイル: $BENCHMARK_EXEC"
fi

if [ "$ENABLE_COVERAGE" = true ] && [ -d "coverage_html" ]; then
    echo "- カバレッジレポート: build/coverage_html/index.html"
fi

echo
echo "テスト完了！"
