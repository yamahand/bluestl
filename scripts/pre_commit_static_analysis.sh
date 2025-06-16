#!/bin/bash
# BlueStl Pre-commit Static Analysis Hook
# コミット前の軽量静的解析チェック

set -e

echo "🔍 BlueStl pre-commit 静的解析を実行中..."

# 引数から変更されたファイルを取得
CHANGED_FILES="$@"

if [ -z "$CHANGED_FILES" ]; then
    echo "変更されたファイルがありません。スキップします。"
    exit 0
fi

echo "対象ファイル:"
echo "$CHANGED_FILES" | tr ' ' '\n' | sed 's/^/  - /'

# 一時ディレクトリの作成
TEMP_DIR=$(mktemp -d)
cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

# BlueStl固有のチェック項目
ISSUES_FOUND=0

echo ""
echo "📋 BlueStl固有チェックを実行中..."

for file in $CHANGED_FILES; do
    if [[ ! -f "$file" ]]; then
        continue
    fi
    
    echo "  チェック中: $file"
    
    # 1. namespace bluestl の使用チェック
    if ! grep -q "namespace bluestl" "$file"; then
        echo "    ❌ namespace bluestl が見つかりません"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
    
    # 2. pragma once の使用チェック
    if ! grep -q "#pragma once" "$file"; then
        echo "    ❌ #pragma once が見つかりません"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
    
    # 3. C++20機能の適切な使用チェック
    if grep -q "std::enable_if" "$file"; then
        echo "    ⚠️  std::enable_if の代わりに C++20 concept の使用を検討してください"
    fi
    
    # 4. BLUESTL_ASSERT の使用チェック（assertの代わり）
    if grep -q "assert(" "$file" && ! grep -q "BLUESTL_ASSERT" "$file"; then
        echo "    ⚠️  assert() の代わりに BLUESTL_ASSERT の使用を検討してください"
    fi
    
    # 5. 未使用のインクルード（簡易チェック）
    INCLUDES=$(grep "^#include" "$file" | wc -l)
    if [ "$INCLUDES" -gt 10 ]; then
        echo "    ⚠️  インクルード数が多いです（$INCLUDES個）。不要なインクルードを確認してください"
    fi
    
    # 6. 長い行のチェック（120文字制限）
    LONG_LINES=$(awk 'length > 120 { print NR ": " $0 }' "$file")
    if [ -n "$LONG_LINES" ]; then
        echo "    ⚠️  120文字を超える行があります:"
        echo "$LONG_LINES" | head -3 | sed 's/^/      /'
        if [ $(echo "$LONG_LINES" | wc -l) -gt 3 ]; then
            echo "      ... および他 $(($(echo "$LONG_LINES" | wc -l) - 3)) 行"
        fi
    fi
    
    # 7. TODO/FIXME コメントのチェック
    TODOS=$(grep -n "TODO\|FIXME\|XXX\|HACK" "$file" || true)
    if [ -n "$TODOS" ]; then
        echo "    📝 TODO/FIXME コメントが見つかりました:"
        echo "$TODOS" | head -2 | sed 's/^/      /'
    fi
done

echo ""
echo "🔧 軽量静的解析を実行中..."

# 軽量 clang-tidy チェック（高速化のため限定的なチェック）
CLANG_TIDY_CHECKS="readability-*,bugprone-*,modernize-*"

for file in $CHANGED_FILES; do
    if [[ "$file" =~ \.h$ ]]; then
        echo "  clang-tidy: $file"
        
        # 高速化のため、重要なチェックのみ実行
        clang-tidy "$file" \
            --checks="$CLANG_TIDY_CHECKS" \
            --header-filter=".*include/bluestl.*" \
            --quiet \
            -- -std=c++20 -Iinclude 2>/dev/null || {
            echo "    ⚠️  clang-tidy でエラーが発生しました（継続します）"
        }
    fi
done

echo ""
echo "📊 結果サマリー:"

if [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ BlueStl固有チェック: 問題なし"
else
    echo "⚠️  BlueStl固有チェック: $ISSUES_FOUND 件の問題"
fi

echo ""
echo "💡 ヒント:"
echo "  - 完全な静的解析を実行: ./scripts/static_analysis.sh"
echo "  - Python版解析ツール: python3 scripts/run_static_analysis.py"
echo "  - フォーマット修正: clang-format -i <ファイル>"

# 現在は警告レベルで終了（将来的にはエラーで停止も可能）
if [ "$ISSUES_FOUND" -gt 10 ]; then
    echo ""
    echo "❌ 問題が多すぎます（$ISSUES_FOUND件）。修正してから再度コミットしてください。"
    exit 1
fi

echo ""
echo "✅ pre-commit 静的解析チェック完了"
exit 0