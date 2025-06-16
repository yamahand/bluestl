# BlueStl 静的解析ガイド

このドキュメントでは、BlueStlプロジェクトにおける静的解析ツールの使用方法と設定について説明します。

## 📋 目次

1. [概要](#概要)
2. [ツール構成](#ツール構成)
3. [使用方法](#使用方法)
4. [設定ファイル](#設定ファイル)
5. [CI/CD統合](#cicd統合)
6. [Pre-commitフック](#pre-commitフック)
7. [トラブルシューティング](#トラブルシューティング)

## 概要

BlueStlでは、コード品質の維持と向上のために複数の静的解析ツールを統合して使用しています。

### 主な目的

- **バグの早期発見**: 潜在的な問題を開発初期段階で検出
- **コード品質の向上**: 可読性、保守性、パフォーマンスの改善
- **C++20準拠**: モダンC++のベストプラクティスの遵守
- **一貫性の保持**: コーディング規約の自動チェック

## ツール構成

### 主要ツール

| ツール | 目的 | 対象 |
|--------|------|------|
| **clang-tidy** | C++静的解析、モダン化 | ヘッダファイル |
| **cppcheck** | バグ検出、未定義動作 | 全ソースファイル |
| **clang-format** | コードフォーマット | C++ファイル |
| **clang-analyzer** | 深い静的解析 | オプション |

### 補助ツール

- **include-what-you-use**: インクルード最適化
- **xmlstarlet**: XML解析（レポート処理）
- **pre-commit**: Git フック管理

## 使用方法

### 1. 基本的な静的解析実行

```bash
# 全ツールでの包括的解析
./scripts/static_analysis.sh

# 詳細出力付き
./scripts/static_analysis.sh --verbose

# 問題の自動修正（注意して使用）
./scripts/static_analysis.sh --fix
```

### 2. Python版統合ツール

```bash
# 高度な解析とレポート生成
python3 scripts/run_static_analysis.py

# Clang Static Analyzerも含めて実行
python3 scripts/run_static_analysis.py --enable-clang-analyzer

# 詳細ログ出力
python3 scripts/run_static_analysis.py --verbose
```

### 3. 個別ツールの実行

```bash
# clang-tidyのみ
./scripts/static_analysis.sh --no-cppcheck

# cppcheckのみ
./scripts/static_analysis.sh --no-clang-tidy

# include-what-you-useも含める
./scripts/static_analysis.sh --enable-iwyu
```

### 4. 特定ファイルの解析

```bash
# 特定のヘッダファイル
clang-tidy include/bluestl/vector.h \
    --config-file=.clang-tidy \
    --header-filter=".*include/bluestl.*" \
    -- -std=c++20 -Iinclude

# フォーマットチェック
clang-format --dry-run --Werror include/bluestl/vector.h
```

## 設定ファイル

### `.clang-tidy`

C++静的解析の詳細設定

```yaml
# 主要設定
Checks: 'bugprone-*,cert-*,clang-analyzer-*,modernize-*,performance-*,readability-*'
HeaderFilterRegex: '^.*/include/bluestl/.*'
WarningsAsErrors: ''

# BlueStl固有の設定
CheckOptions:
  - key: readability-identifier-naming.NamespaceCase
    value: lower_case
  - key: readability-identifier-naming.ClassCase
    value: lower_case
```

### `.cppcheck`

cppcheck固有の抑制設定

```ini
# 基本設定
enable=warning,style,performance,portability,information

# 抑制ルール
suppress=missingIncludeSystem
suppress=unusedFunction:tests/*
suppress=noExplicitConstructor
```

### `.clang-format`

コードフォーマットルール（既存の設定を使用）

## CI/CD統合

### GitHub Actions

静的解析は自動的にCI/CDパイプラインで実行されます：

```yaml
# .github/workflows/ci.yml
- name: Run comprehensive static analysis
  run: |
    ./scripts/static_analysis.sh --verbose
    python3 scripts/run_static_analysis.py --enable-clang-analyzer
```

### 実行タイミング

- **プッシュ時**: `master`, `develop` ブランチ
- **プルリクエスト時**: 全ブランチ
- **定期実行**: 毎日（オプション）

### レポート出力

- **Artifacts**: 詳細なレポートファイル
- **PR コメント**: 解析結果のサマリー
- **ステータスチェック**: ビルドステータスに反映

## Pre-commitフック

開発者のローカル環境でコミット前に自動チェック：

### セットアップ

```bash
# pre-commitのインストール
pip install pre-commit

# フックの有効化
pre-commit install

# 手動実行
pre-commit run --all-files
```

### チェック項目

1. **コードフォーマット**: clang-format
2. **静的解析**: clang-tidy（軽量版）
3. **BlueStl固有チェック**: カスタムスクリプト
4. **インクルードガード**: ヘッダファイル
5. **C++20準拠**: モダンC++チェック

### 設定ファイル

`.pre-commit-config.yaml` で詳細設定

## レポートの読み方

### 統合レポート

`static_analysis_reports/summary_YYYYMMDD_HHMMSS.md`

- **問題数の総計**
- **ツール別の結果**
- **主な問題カテゴリ**
- **推奨アクション**

### 個別レポート

- `clang_tidy_YYYYMMDD_HHMMSS.txt`: clang-tidy結果
- `cppcheck_YYYYMMDD_HHMMSS.xml`: cppcheck結果（XML）
- `iwyu_YYYYMMDD_HHMMSS.txt`: include-what-you-use結果

### 重要度の判断

| レベル | 対応 | 例 |
|--------|------|-----|
| **error** | 即座に修正 | メモリリーク、未定義動作 |
| **warning** | 優先的に修正 | 潜在的バグ、非推奨機能 |
| **style** | 時間があるときに修正 | 命名規則、フォーマット |
| **info** | 参考情報 | 最適化提案、改善案 |

## トラブルシューティング

### よくある問題

#### 1. clang-tidyが見つからない

```bash
# Ubuntu/Debian
sudo apt-get install clang-tidy

# macOS
brew install llvm
```

#### 2. 大量の警告が出る

```bash
# 段階的な修正を推奨
./scripts/static_analysis.sh --no-cppcheck  # clang-tidyのみ
```

#### 3. 解析が遅い

```bash
# ファイルを限定
find include/bluestl -name "vector.h" -exec clang-tidy {} ...
```

#### 4. 誤検知の抑制

`.clang-tidy` で個別ルールを無効化：

```yaml
Checks: '*,-bugprone-easily-swappable-parameters'
```

### パフォーマンス最適化

#### 1. 並列実行

```bash
# 複数ファイルを並列処理
find include/bluestl -name "*.h" | xargs -P4 -I{} clang-tidy {} ...
```

#### 2. キャッシュ活用

```bash
# compilation databaseの生成
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
```

#### 3. 対象ファイル限定

```bash
# 変更されたファイルのみ
git diff --name-only HEAD^ | grep "\\.h$" | xargs clang-tidy ...
```

## 高度な使用方法

### カスタムチェックの追加

1. `.clang-tidy` にルール追加
2. `scripts/pre_commit_static_analysis.sh` にロジック追加
3. CI設定の更新

### 新しいツールの統合

1. `scripts/static_analysis.sh` にツール追加
2. 設定ファイルの作成
3. CI/CDパイプラインの更新
4. ドキュメントの更新

### レポート形式のカスタマイズ

`scripts/run_static_analysis.py` の `generate_summary_report` メソッドを編集

## 参考リンク

- [clang-tidy documentation](https://clang.llvm.org/extra/clang-tidy/)
- [cppcheck manual](http://cppcheck.sourceforge.net/manual.pdf)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Modern C++ Guidelines](https://github.com/Microsoft/GSL)

## サポート

問題や質問がある場合：

1. **GitHub Issues**: バグ報告、機能要求
2. **プロジェクトドキュメント**: 追加情報
3. **CI/CDログ**: 詳細なエラー情報

---

*このドキュメントはBlueStlプロジェクトの静的解析統合の一部です。*