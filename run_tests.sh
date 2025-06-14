#!/bin/bash

# BlueStl テストスクリプト
# CMakeを使用してプロジェクトをビルドし、テストを実行します

set -e  # エラー時に即座に終了

echo "BlueStl テストを開始します..."

# 元のディレクトリを保存
ORIG_DIR=$(pwd)

# クリーンアップ関数（スクリプト終了時に必ず元のディレクトリに戻る）
cleanup() {
    cd "$ORIG_DIR"
}
trap cleanup EXIT

# ビルドディレクトリ作成
echo "ビルドディレクトリを作成中..."
mkdir -p build
cd build

# CMakeでプロジェクト構成
echo "CMakeでプロジェクトを構成中..."
if ! cmake .. 2>&1; then
    echo "CMake構成が失敗しました。"
    exit 1
fi

# ビルド
echo "プロジェクトをビルド中..."
if ! cmake --build . 2>&1; then
    echo "ビルドが失敗しました。"
    exit 1
fi

# テスト実行
echo "テストを実行中..."
if [ -f Debug/test_all ]; then
    echo "Debug/test_all を実行します..."
    Debug/test_all
elif [ -f test_all ]; then
    echo "test_all を実行します..."
    ./test_all
else
    echo "test_all が見つかりませんでした。"
    exit 1
fi

echo "全てのテストが成功しました！"
