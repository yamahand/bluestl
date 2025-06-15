#!/bin/bash

# BlueStl 静的解析スクリプト
# clang-tidy, cppcheck, その他の静的解析ツールを統合実行

set -e

echo "BlueStl 静的解析を開始します..."

# 元のディレクトリを保存
ORIG_DIR=$(pwd)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# クリーンアップ関数
cleanup() {
    cd "$ORIG_DIR"
}
trap cleanup EXIT

cd "$PROJECT_ROOT"

# オプション解析
ENABLE_CLANG_TIDY=true
ENABLE_CPPCHECK=true
ENABLE_IWYU=false
FIX_ISSUES=false
VERBOSE=false
OUTPUT_DIR="static_analysis_reports"

while [[ $# -gt 0 ]]; do
    case $1 in
        --no-clang-tidy)
            ENABLE_CLANG_TIDY=false
            shift
            ;;
        --no-cppcheck)
            ENABLE_CPPCHECK=false
            shift
            ;;
        --enable-iwyu)
            ENABLE_IWYU=true
            shift
            ;;
        --fix)
            FIX_ISSUES=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help|-h)
            echo "使用方法: $0 [オプション]"
            echo ""
            echo "オプション:"
            echo "  --no-clang-tidy    clang-tidyをスキップ"
            echo "  --no-cppcheck      cppcheckをスキップ"
            echo "  --enable-iwyu      include-what-you-useを有効化"
            echo "  --fix              可能な問題を自動修正"
            echo "  --verbose, -v      詳細出力"
            echo "  --output-dir DIR   レポート出力ディレクトリ（デフォルト: static_analysis_reports）"
            echo "  --help, -h         このヘルプを表示"
            exit 0
            ;;
        *)
            echo "不明なオプション: $1"
            echo "使用方法: $0 --help"
            exit 1
            ;;
    esac
done

# 出力ディレクトリの準備
mkdir -p "$OUTPUT_DIR"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# 解析対象ファイルの取得
HEADER_FILES=$(find include/bluestl -name "*.h" -type f)
SOURCE_FILES=$(find tests -name "*.cpp" -type f 2>/dev/null || true)
ALL_FILES="$HEADER_FILES $SOURCE_FILES"

if [ -z "$ALL_FILES" ]; then
    echo "警告: 解析対象のファイルが見つかりません。"
    exit 1
fi

echo "📂 解析対象ファイル数: $(echo $ALL_FILES | wc -w)"
if [ "$VERBOSE" = true ]; then
    echo "対象ファイル:"
    echo "$ALL_FILES" | tr ' ' '\n' | sed 's/^/  - /'
fi

# 1. clang-tidy解析
if [ "$ENABLE_CLANG_TIDY" = true ]; then
    echo ""
    echo "🔍 clang-tidy解析を実行中..."
    
    if ! command -v clang-tidy &> /dev/null; then
        echo "警告: clang-tidyが見つかりません。スキップします。"
        echo "インストール: sudo apt-get install clang-tidy"
    else
        CLANG_TIDY_REPORT="$OUTPUT_DIR/clang_tidy_${TIMESTAMP}.txt"
        CLANG_TIDY_OPTIONS=""
        
        if [ "$FIX_ISSUES" = true ]; then
            CLANG_TIDY_OPTIONS="--fix"
            echo "⚠️  自動修正モードが有効です。ファイルが変更される可能性があります。"
        fi
        
        echo "レポート出力先: $CLANG_TIDY_REPORT"
        
        {
            echo "BlueStl clang-tidy解析レポート"
            echo "生成日時: $(date)"
            echo "対象ファイル数: $(echo $HEADER_FILES | wc -w)"
            echo "================================"
            echo ""
        } > "$CLANG_TIDY_REPORT"
        
        # ヘッダファイルのみを解析（テストファイルは除外）
        CLANG_TIDY_EXIT_CODE=0
        for file in $HEADER_FILES; do
            if [ "$VERBOSE" = true ]; then
                echo "  解析中: $file"
            fi
            
            clang-tidy "$file" $CLANG_TIDY_OPTIONS \
                --config-file=.clang-tidy \
                --header-filter=".*include/bluestl.*" \
                -- -std=c++20 -Iinclude 2>&1 >> "$CLANG_TIDY_REPORT" || CLANG_TIDY_EXIT_CODE=$?
        done
        
        # 結果サマリー
        ISSUE_COUNT=$(grep -c "warning:\|error:" "$CLANG_TIDY_REPORT" || echo "0")
        echo "✅ clang-tidy解析完了: $ISSUE_COUNT 件の問題を検出"
        
        if [ "$ISSUE_COUNT" -gt 0 ]; then
            echo "主な問題のサマリー:"
            grep "warning:\|error:" "$CLANG_TIDY_REPORT" | \
                sed 's/.*\[\(.*\)\].*/\1/' | \
                sort | uniq -c | sort -nr | head -5 | \
                sed 's/^/  /'
        fi
    fi
fi

# 2. cppcheck解析
if [ "$ENABLE_CPPCHECK" = true ]; then
    echo ""
    echo "🔍 cppcheck解析を実行中..."
    
    if ! command -v cppcheck &> /dev/null; then
        echo "警告: cppcheckが見つかりません。スキップします。"
        echo "インストール: sudo apt-get install cppcheck"
    else
        CPPCHECK_REPORT="$OUTPUT_DIR/cppcheck_${TIMESTAMP}.txt"
        CPPCHECK_XML_REPORT="$OUTPUT_DIR/cppcheck_${TIMESTAMP}.xml"
        
        echo "レポート出力先: $CPPCHECK_REPORT"
        
        cppcheck \
            --enable=all \
            --std=c++20 \
            --platform=native \
            --suppress=missingIncludeSystem \
            --suppress=unusedFunction \
            --suppress=unmatchedSuppression \
            --inconclusive \
            --inline-suppr \
            --template="[{file}:{line}] ({severity}) {id}: {message}" \
            --xml \
            --xml-version=2 \
            -I include \
            include/bluestl \
            2> "$CPPCHECK_XML_REPORT" | tee "$CPPCHECK_REPORT"
        
        # XML形式の結果をテキストに変換
        if command -v xmlstarlet &> /dev/null && [ -f "$CPPCHECK_XML_REPORT" ]; then
            {
                echo ""
                echo "詳細な問題分析:"
                echo "================"
                xmlstarlet sel -t -m "//error" \
                    -v "concat(@file, ':', @line, ' (', @severity, ') ', @id, ': ', @msg)" \
                    -n "$CPPCHECK_XML_REPORT" 2>/dev/null || echo "XML解析エラー"
            } >> "$CPPCHECK_REPORT"
        fi
        
        # 結果サマリー
        ISSUE_COUNT=$(grep -c "error\|warning\|style\|performance\|portability" "$CPPCHECK_REPORT" || echo "0")
        echo "✅ cppcheck解析完了: $ISSUE_COUNT 件の問題を検出"
    fi
fi

# 3. include-what-you-use解析（オプション）
if [ "$ENABLE_IWYU" = true ]; then
    echo ""
    echo "🔍 include-what-you-use解析を実行中..."
    
    if ! command -v include-what-you-use &> /dev/null; then
        echo "警告: include-what-you-useが見つかりません。スキップします。"
        echo "インストール: sudo apt-get install iwyu"
    else
        IWYU_REPORT="$OUTPUT_DIR/iwyu_${TIMESTAMP}.txt"
        
        echo "レポート出力先: $IWYU_REPORT"
        
        {
            echo "BlueStl include-what-you-use解析レポート"
            echo "生成日時: $(date)"
            echo "========================================"
            echo ""
        } > "$IWYU_REPORT"
        
        for file in $HEADER_FILES; do
            if [ "$VERBOSE" = true ]; then
                echo "  解析中: $file"
            fi
            
            include-what-you-use \
                -std=c++20 \
                -Iinclude \
                "$file" 2>&1 >> "$IWYU_REPORT" || true
        done
        
        echo "✅ include-what-you-use解析完了"
    fi
fi

# 4. 統合レポートの生成
echo ""
echo "📊 統合レポートを生成中..."

SUMMARY_REPORT="$OUTPUT_DIR/summary_${TIMESTAMP}.md"

{
    echo "# BlueStl 静的解析統合レポート"
    echo ""
    echo "**生成日時**: $(date)"
    echo "**解析対象**: BlueStlライブラリ"
    echo "**ファイル数**: $(echo $HEADER_FILES | wc -w) ヘッダファイル"
    echo ""
    
    if [ "$ENABLE_CLANG_TIDY" = true ] && [ -f "$OUTPUT_DIR/clang_tidy_${TIMESTAMP}.txt" ]; then
        CLANG_TIDY_ISSUES=$(grep -c "warning:\|error:" "$OUTPUT_DIR/clang_tidy_${TIMESTAMP}.txt" || echo "0")
        echo "## clang-tidy結果"
        echo "- **検出問題数**: $CLANG_TIDY_ISSUES"
        echo "- **詳細レポート**: [clang_tidy_${TIMESTAMP}.txt](./clang_tidy_${TIMESTAMP}.txt)"
        echo ""
    fi
    
    if [ "$ENABLE_CPPCHECK" = true ] && [ -f "$OUTPUT_DIR/cppcheck_${TIMESTAMP}.txt" ]; then
        CPPCHECK_ISSUES=$(grep -c "error\|warning\|style\|performance\|portability" "$OUTPUT_DIR/cppcheck_${TIMESTAMP}.txt" || echo "0")
        echo "## cppcheck結果"
        echo "- **検出問題数**: $CPPCHECK_ISSUES"
        echo "- **詳細レポート**: [cppcheck_${TIMESTAMP}.txt](./cppcheck_${TIMESTAMP}.txt)"
        echo ""
    fi
    
    if [ "$ENABLE_IWYU" = true ] && [ -f "$OUTPUT_DIR/iwyu_${TIMESTAMP}.txt" ]; then
        echo "## include-what-you-use結果"
        echo "- **詳細レポート**: [iwyu_${TIMESTAMP}.txt](./iwyu_${TIMESTAMP}.txt)"
        echo ""
    fi
    
    echo "## 推奨アクション"
    echo "1. 高優先度の警告・エラーを確認し修正"
    echo "2. パフォーマンス関連の指摘を検討"
    echo "3. コードスタイルの統一"
    echo "4. 未使用インクルードの整理"
    echo ""
    
    echo "## 解析ツール情報"
    if command -v clang-tidy &> /dev/null; then
        echo "- **clang-tidy**: $(clang-tidy --version | head -1)"
    fi
    if command -v cppcheck &> /dev/null; then
        echo "- **cppcheck**: $(cppcheck --version)"
    fi
    if command -v include-what-you-use &> /dev/null; then
        echo "- **include-what-you-use**: $(include-what-you-use --version 2>&1 | head -1 || echo "バージョン情報取得不可")"
    fi
} > "$SUMMARY_REPORT"

echo "✅ 統合レポート生成完了: $SUMMARY_REPORT"

# 結果の表示
echo ""
echo "🎉 静的解析が完了しました！"
echo ""
echo "📋 レポート一覧:"
ls -la "$OUTPUT_DIR"/*${TIMESTAMP}* | sed 's/^/  /'

echo ""
echo "📖 次のステップ:"
echo "  1. 統合レポートを確認: $SUMMARY_REPORT"
echo "  2. 詳細レポートで問題を調査"
echo "  3. 重要な問題を修正"
echo "  4. 継続的な改善を実施"

# CI/CD環境での終了コード設定
if [ "${CI:-false}" = "true" ]; then
    if [ "$ENABLE_CLANG_TIDY" = true ] && [ "${CLANG_TIDY_ISSUES:-0}" -gt 0 ]; then
        echo "警告: CI環境でclang-tidyの問題が検出されました。"
    fi
    if [ "$ENABLE_CPPCHECK" = true ] && [ "${CPPCHECK_ISSUES:-0}" -gt 0 ]; then
        echo "警告: CI環境でcppcheckの問題が検出されました。"
    fi
fi

echo "静的解析処理が完了しました。"