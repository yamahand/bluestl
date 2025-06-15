#!/bin/bash

# BlueStl ドキュメント生成スクリプト
# Doxygenを使用してAPIドキュメントを生成します

set -e

echo "BlueStl APIドキュメント生成を開始します..."

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

# Doxygenのインストール確認
if ! command -v doxygen &> /dev/null; then
    echo "エラー: Doxygenがインストールされていません。"
    echo "Ubuntu/Debian: sudo apt-get install doxygen graphviz"
    echo "macOS: brew install doxygen graphviz"
    echo "Windows: https://www.doxygen.nl/download.html からダウンロード"
    exit 1
fi

# 出力ディレクトリの準備
if [ -d "docs/html" ]; then
    echo "既存のドキュメントをクリアしています..."
    rm -rf docs/html
fi

mkdir -p docs

# Doxygenでドキュメント生成
echo "Doxygenでドキュメントを生成中..."
if ! doxygen Doxyfile 2>&1 | tee docs/doxygen.log; then
    echo "エラー: ドキュメント生成が失敗しました。"
    echo "詳細は docs/doxygen.log を確認してください。"
    exit 1
fi

# 生成結果の確認
if [ -f "docs/html/index.html" ]; then
    echo "✅ ドキュメント生成が成功しました！"
    echo "📂 出力先: $(realpath docs/html)"
    echo "🌐 ブラウザで確認: file://$(realpath docs/html/index.html)"
    
    # ファイル数とサイズの表示
    FILE_COUNT=$(find docs/html -type f | wc -l)
    DIR_SIZE=$(du -sh docs/html | cut -f1)
    echo "📊 生成されたファイル数: $FILE_COUNT"
    echo "📊 ドキュメントサイズ: $DIR_SIZE"
    
    # 主要ファイルの確認
    if [ -f "docs/html/annotated.html" ]; then
        echo "✅ クラス一覧ページが生成されました"
    fi
    if [ -f "docs/html/files.html" ]; then
        echo "✅ ファイル一覧ページが生成されました"
    fi
    if [ -f "docs/html/search/search.js" ]; then
        echo "✅ 検索機能が利用可能です"
    fi
    
    # ブラウザで自動オープン（オプション）
    if command -v xdg-open &> /dev/null; then
        read -p "ブラウザでドキュメントを開きますか？ (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            xdg-open "docs/html/index.html"
        fi
    elif command -v open &> /dev/null; then
        read -p "ブラウザでドキュメントを開きますか？ (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            open "docs/html/index.html"
        fi
    fi
    
    echo ""
    echo "📖 使用方法:"
    echo "  - ローカル閲覧: docs/html/index.html をブラウザで開く"
    echo "  - GitHub Pages: プッシュ後に自動デプロイされます"
    echo "  - 再生成: ./docs/generate_docs.sh を再実行"
    
else
    echo "❌ エラー: ドキュメント生成が完了しませんでした。"
    echo "Doxyfileの設定か、ソースコードのコメント形式を確認してください。"
    exit 1
fi

echo "ドキュメント生成処理が完了しました。"