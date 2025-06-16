#!/bin/bash
# Include Guard チェックスクリプト
# BlueStlヘッダファイルのinclude guardをチェック

set -e

echo "🛡️  Include Guard チェックを実行中..."

CHANGED_FILES="$@"
ISSUES_FOUND=0

for file in $CHANGED_FILES; do
    if [[ ! -f "$file" ]] || [[ ! "$file" =~ \.h$ ]]; then
        continue
    fi
    
    echo "  チェック中: $file"
    
    # ファイル名からガード名を生成（期待値）
    # include/bluestl/vector.h -> BLUESTL_VECTOR_H_
    EXPECTED_GUARD=$(echo "$file" | sed 's|include/bluestl/||' | tr '[:lower:]' '[:upper:]' | sed 's/\./_/g' | sed 's/^/BLUESTL_/' | sed 's/$/_/')
    
    # #pragma once の使用をチェック
    if grep -q "^#pragma once" "$file"; then
        echo "    ✅ #pragma once を使用"
        continue
    fi
    
    # include guard のチェック
    FIRST_LINE=$(head -1 "$file")
    SECOND_LINE=$(head -2 "$file" | tail -1)
    LAST_LINE=$(tail -1 "$file")
    
    # #ifndef チェック
    if [[ "$FIRST_LINE" =~ ^#ifndef[[:space:]]+([A-Z_]+)$ ]]; then
        GUARD_NAME="${BASH_REMATCH[1]}"
        
        # #define チェック
        if [[ "$SECOND_LINE" =~ ^#define[[:space:]]+$GUARD_NAME$ ]]; then
            
            # #endif チェック
            if [[ "$LAST_LINE" =~ ^#endif.*$GUARD_NAME ]]; then
                
                # ガード名が期待値と一致するかチェック
                if [ "$GUARD_NAME" = "$EXPECTED_GUARD" ]; then
                    echo "    ✅ Include guard OK: $GUARD_NAME"
                else
                    echo "    ⚠️  Include guard名が推奨形式と異なります"
                    echo "        実際: $GUARD_NAME"
                    echo "        推奨: $EXPECTED_GUARD"
                fi
                
            else
                echo "    ❌ #endif が見つからないか、ガード名が一致しません"
                ISSUES_FOUND=$((ISSUES_FOUND + 1))
            fi
            
        else
            echo "    ❌ #define $GUARD_NAME が見つかりません"
            ISSUES_FOUND=$((ISSUES_FOUND + 1))
        fi
        
    else
        echo "    ❌ #ifndef が見つからないか、形式が正しくありません"
        echo "        推奨: #ifndef $EXPECTED_GUARD"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
    
    # 重複ガードのチェック
    GUARD_COUNT=$(grep -c "#ifndef\|#pragma once" "$file" || echo "0")
    if [ "$GUARD_COUNT" -gt 1 ]; then
        echo "    ⚠️  複数のinclude guardが検出されました"
    fi
done

echo ""
echo "📊 Include Guard チェック結果:"

if [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ 問題なし"
else
    echo "❌ $ISSUES_FOUND 件の問題が見つかりました"
    echo ""
    echo "💡 修正方法:"
    echo "  1. #pragma once の使用（推奨）"
    echo "  2. 従来のinclude guard: #ifndef BLUESTL_FILENAME_H_"
    echo "     例: #ifndef BLUESTL_VECTOR_H_"
    echo "         #define BLUESTL_VECTOR_H_"
    echo "         ..."
    echo "         #endif // BLUESTL_VECTOR_H_"
fi

exit $ISSUES_FOUND