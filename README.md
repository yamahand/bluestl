# BlueStl

高速なコンパイル時間と実行時間、そしてコンテナと機能の固定サイズ代替に重点を置いた、C++20対応のSTL代替/補完ライブラリです。

## 特徴

- **C++20準拠**: 最新のC++20機能を活用した設計
- **RTTI/例外なし**: RTTIや例外を使用しない高速実行環境
- **高速コンパイル**: 外部ヘッダーへの依存を最小限に抑制
- **固定サイズコンテナ**: ヒープ割り当てを行わない`fixed_`プレフィックスコンテナ
- **Header-Only**: インクルードするだけで使用可能
- **STL風インターフェース**: 既存のSTLコードとの互換性を重視

## サポートされているコンテナ

### 基本コンテナ
- ✅ `bluestl::vector` - 動的配列
- ✅ `bluestl::string` - 可変長文字列
- ✅ `bluestl::array` - 固定サイズ配列
- ✅ `bluestl::span` - 配列ビュー
- ⚠️ `bluestl::string_view` - 文字列ビュー（実装済み、未追跡）

### 固定サイズコンテナ
- ✅ `bluestl::fixed_string` - 固定サイズ文字列
- ✅ `bluestl::fixed_vector` - 固定サイズ動的配列
- ✅ `bluestl::fixed_hash_map` - 固定サイズハッシュマップ
- ✅ `bluestl::small_buffer_vector` - 小容量バッファ付き動的配列

### 連想コンテナ
- ✅ `bluestl::hash_map` - ハッシュマップ

### ユーティリティ
- ✅ `bluestl::optional` - 値の有無を表現
- ✅ `bluestl::variant` - 複数型のいずれかを格納
- ✅ `bluestl::pair` - 2つの値のペア
- ✅ `bluestl::tuple` - 複数の値のタプル

## クイックスタート

### 必要な環境
- C++20対応コンパイラ（GCC 11+、Clang 13+、MSVC 2019+）
- CMake 3.15以上

### インクルードと使用例

```cpp
#include <bluestl/vector.h>
#include <bluestl/string.h>
#include <bluestl/fixed_vector.h>

int main() {
    // 動的配列
    bluestl::vector<int> vec;
    vec.push_back(42);
    
    // 可変長文字列
    bluestl::string str("Hello, BlueStl!");
    
    // 固定サイズ配列（ヒープ割り当てなし）
    bluestl::fixed_vector<int, 10> fixed_vec;
    fixed_vec.push_back(100);
    
    return 0;
}
```

### ビルドとテスト

```bash
# リポジトリのクローン
git clone <repository-url>
cd bluestl

# ビルドとテスト実行
./run_tests.sh

# または手動でビルド
mkdir build && cd build
cmake ..
cmake --build .
./test_all
```

## プロジェクト構造

```
bluestl/
├── include/bluestl/     # ヘッダファイル
│   ├── vector.h
│   ├── string.h
│   ├── fixed_vector.h
│   └── ...
├── tests/               # テストコード
├── .github/workflows/   # CI/CD設定
├── CMakeLists.txt       # ビルド設定
└── TODO.md             # 実装予定機能
```

## 設計方針

1. **高速なコンパイル**: 深い関数ネストを避け、インクルード依存を最小化
2. **明確なC++コード**: 難読化されていないシンプルな実装
3. **分離と粒度**: 単一インクルードでの影響を最小限に
4. **カスタムアロケータ**: 外部からアロケータを渡すことが可能
5. **デバッグ効率**: 最適化されていないコードでも高速実行

## 貢献方法

1. Issueを作成して議論
2. フォークしてfeatureブランチを作成
3. 変更を実装してテストを追加
4. Pull Requestを作成

## ライセンス

[ライセンス情報を追加してください]

## ロードマップ

詳細な実装予定については[TODO.md](TODO.md)をご確認ください。

### 次の主要機能
- `expected/result` - エラーハンドリング
- `algorithm` functions - 基本アルゴリズム
- `flat_map/flat_set` - 高性能連想コンテナ

---

**注意**: このライブラリは開発中です。本番環境での使用前に十分なテストを行ってください。