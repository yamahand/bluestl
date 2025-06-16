#!/bin/bash
# C++20 Compliance チェックスクリプト
# C++20機能の適切な使用と互換性をチェック

set -e

echo "🆕 C++20 準拠チェックを実行中..."

CHANGED_FILES="$@"
ISSUES_FOUND=0
SUGGESTIONS=0

for file in $CHANGED_FILES; do
    if [[ ! -f "$file" ]] || [[ ! "$file" =~ \.(h|cpp)$ ]]; then
        continue
    fi
    
    echo "  チェック中: $file"
    
    # 1. C++20 concepts の使用推奨チェック
    if grep -q "std::enable_if\|SFINAE" "$file"; then
        echo "    💡 C++20 concepts の使用を検討してください (std::enable_if の代替)"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 2. constexpr の適切な使用チェック
    CONSTEXPR_COUNT=$(grep -c "constexpr" "$file" || echo "0")
    if [ "$CONSTEXPR_COUNT" -eq 0 ] && grep -q "inline.*function\|static.*function" "$file"; then
        echo "    💡 constexpr の使用を検討してください（コンパイル時計算）"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 3. nullptr の使用チェック
    if grep -q "\bNULL\b\|0.*pointer" "$file"; then
        echo "    ⚠️  NULL の代わりに nullptr を使用してください"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
    
    # 4. auto の適切な使用チェック
    AUTO_COUNT=$(grep -c "\bauto\b" "$file" || echo "0")
    EXPLICIT_TYPE_COUNT=$(grep -c "std::\|bluestl::" "$file" || echo "0")
    
    if [ "$AUTO_COUNT" -eq 0 ] && [ "$EXPLICIT_TYPE_COUNT" -gt 5 ]; then
        echo "    💡 型推論（auto）の使用を検討してください"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 5. range-based for loop の使用推奨
    if grep -q "for.*int.*=.*; .*<.*size(); .*++" "$file"; then
        echo "    💡 range-based for loop の使用を検討してください"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 6. std::move の適切な使用チェック
    if grep -q "std::move" "$file"; then
        # move後の変数使用チェック（簡易版）
        MOVE_LINES=$(grep -n "std::move" "$file")
        while IFS= read -r line; do
            VAR_NAME=$(echo "$line" | sed 's/.*std::move(\([^)]*\)).*/\1/' | tr -d ' ')
            LINE_NUM=$(echo "$line" | cut -d: -f1)
            
            # move後の同じ変数の使用をチェック（次の数行）
            AFTER_LINES=$(sed -n "$((LINE_NUM+1)),$((LINE_NUM+5))p" "$file")
            if echo "$AFTER_LINES" | grep -q "\b$VAR_NAME\b"; then
                echo "    ⚠️  std::move後の変数 '$VAR_NAME' が使用されている可能性があります (行 $LINE_NUM)"
                ISSUES_FOUND=$((ISSUES_FOUND + 1))
            fi
        done <<< "$MOVE_LINES"
    fi
    
    # 7. [[nodiscard]] の使用推奨
    FUNCTION_COUNT=$(grep -c "^\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(" "$file" || echo "0")
    NODISCARD_COUNT=$(grep -c "\[\[nodiscard\]\]" "$file" || echo "0")
    
    if [ "$FUNCTION_COUNT" -gt 3 ] && [ "$NODISCARD_COUNT" -eq 0 ]; then
        echo "    💡 [[nodiscard]] 属性の使用を検討してください（戻り値を無視すべきでない関数）"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 8. consteval の使用推奨（コンパイル時関数）
    if grep -q "constexpr.*return.*[0-9]" "$file" && ! grep -q "consteval" "$file"; then
        echo "    💡 コンパイル時定数には consteval の使用を検討してください"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 9. requires clause の使用推奨
    if grep -q "template.*class\|template.*typename" "$file" && ! grep -q "requires\|concept" "$file"; then
        echo "    💡 テンプレート制約に requires clause の使用を検討してください"
        SUGGESTIONS=$((SUGGESTIONS + 1))
    fi
    
    # 10. deprecated 機能の使用チェック
    DEPRECATED_FEATURES=(
        "std::auto_ptr"
        "std::binary_function"
        "std::unary_function"
        "std::random_shuffle"
    )
    
    for deprecated in "${DEPRECATED_FEATURES[@]}"; do
        if grep -q "$deprecated" "$file"; then
            echo "    ❌ 非推奨機能の使用: $deprecated"
            ISSUES_FOUND=$((ISSUES_FOUND + 1))
        fi
    done
    
    # 11. C++20 標準ライブラリ機能の活用チェック
    C20_FEATURES=(
        "std::span"
        "std::format"
        "std::ranges"
        "std::concepts"
        "std::bit_cast"
        "std::source_location"
    )
    
    for feature in "${C20_FEATURES[@]}"; do
        if grep -q "$feature" "$file"; then
            echo "    ✅ C++20機能を使用: $feature"
        fi
    done
    
    # 12. using namespace std の使用チェック（禁止）
    if grep -q "using namespace std" "$file"; then
        echo "    ❌ 'using namespace std' の使用は避けてください"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
    
    # 13. C スタイルキャストのチェック
    C_STYLE_CASTS=$(grep -o "([a-zA-Z_][a-zA-Z0-9_]*\s*\*\?\s*)" "$file" | wc -l)
    if [ "$C_STYLE_CASTS" -gt 0 ]; then
        echo "    ⚠️  C スタイルキャストの可能性があります。static_cast/dynamic_cast/reinterpret_cast を使用してください"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
done

echo ""
echo "📊 C++20 準拠チェック結果:"
echo "❌ 問題: $ISSUES_FOUND 件"
echo "💡 改善提案: $SUGGESTIONS 件"

if [ "$ISSUES_FOUND" -eq 0 ] && [ "$SUGGESTIONS" -eq 0 ]; then
    echo "✅ C++20 準拠: 問題なし"
elif [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ 問題なし（改善提案あり）"
fi

echo ""
echo "🔗 参考リソース:"
echo "  - C++20 機能一覧: https://en.cppreference.com/w/cpp/20"
echo "  - Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/"
echo "  - Modern C++ 規約: https://github.com/isocpp/CppCoreGuidelines"

# エラーレベルの問題がある場合は1で終了
if [ "$ISSUES_FOUND" -gt 0 ]; then
    exit 1
fi

exit 0