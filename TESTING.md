# BlueStl テストスイート

BlueStlプロジェクトの包括的テストスイートの説明とガイドラインです。

## 概要

BlueStlテストスイートは以下の機能を提供します：

- **包括的なユニットテスト**: Catch2フレームワークを使用
- **パフォーマンステスト**: Google Benchmarkフレームワークを使用
- **コードカバレッジ測定**: lcov/genhtmlを使用
- **メモリ安全性チェック**: AddressSanitizer/UndefinedBehaviorSanitizerを使用
- **自動化スクリプト**: 全ての機能を統合した実行スクリプト

## テストカテゴリ

### 1. 基本機能テスト

各コンテナの基本的な動作を検証：

- **vector**: 動的配列の全機能（挿入、削除、リサイズなど）
- **string**: 文字列クラスの全機能（検索、操作、変換など）
- **hash_map**: ハッシュマップの全機能（挿入、検索、削除など）
- **optional**: オプショナル値の全機能
- **variant**: バリアント型の全機能
- **fixed_vector**: 固定サイズベクターの全機能
- **fixed_string**: 固定サイズ文字列の全機能

### 2. エッジケーステスト

境界値や特殊条件でのテスト：

- 空のコンテナでの操作
- 最大サイズでの操作
- 範囲外アクセス（アサーション確認）
- 自己代入
- nullptrでの操作

### 3. メモリ安全性テスト

メモリリークや不正アクセスの検出：

- RAII準拠（デストラクタ自動呼び出し）
- ダングリングポインタの回避
- 例外安全性（基本保証）
- メモリアライメント

### 4. パフォーマンステスト

実行時性能の測定とベンチマーク：

- 大量データでの操作性能
- std::標準ライブラリとの比較
- メモリ使用量の効率性
- 操作の時間計算量

### 5. 特殊型対応テスト

様々な型との互換性：

- ムーブオンリー型
- const型
- 参照型
- ポインタ型
- アライメント要求の厳しい型

## テスト実行方法

### 基本テスト

```bash
# 基本テストのみ実行
./run_tests.sh

# または
./run_tests.sh --help  # オプション一覧を表示
```

### コードカバレッジ付きテスト

```bash
# カバレッジレポートを生成
./run_tests.sh --coverage

# レポートはbuild/coverage_html/index.htmlに生成されます
```

### メモリ安全性チェック付きテスト

```bash
# AddressSanitizer + UndefinedBehaviorSanitizerを使用
./run_tests.sh --sanitizers
```

### ベンチマークテスト

```bash
# パフォーマンステストを実行
./run_tests.sh --benchmarks
```

### 全機能を有効化

```bash
# 全ての機能を同時に実行
./run_tests.sh --coverage --sanitizers --benchmarks
```

### リリースビルドでのテスト

```bash
# 最適化されたリリースビルドでテスト
./run_tests.sh --release
```

### クリーンビルド

```bash
# 既存のビルドファイルを削除してから実行
./run_tests.sh --clean
```

## CMakeオプション

CMakeを直接使用する場合のオプション：

```bash
# カバレッジ有効化
cmake -DENABLE_COVERAGE=ON ..

# サニタイザ有効化
cmake -DENABLE_SANITIZERS=ON ..

# ベンチマーク有効化
cmake -DENABLE_BENCHMARKS=ON ..

# リリースビルド
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## テストファイル構成

### ユニットテスト (`tests/`)

- `test_vector.cpp`: vectorクラスの包括的テスト
- `test_string.cpp`: stringクラスの包括的テスト
- `test_hash_map.cpp`: hash_mapクラスの包括的テスト
- `test_optional.cpp`: optionalクラスの包括的テスト
- `test_variant.cpp`: variantクラスの包括的テスト
- `test_fixed_vector.cpp`: fixed_vectorクラスのテスト
- `test_fixed_string.cpp`: fixed_stringクラスのテスト
- その他のコンテナテスト

### ベンチマークテスト (`benchmarks/`)

- `benchmark_vector.cpp`: vectorのパフォーマンステスト
- `benchmark_hash_map.cpp`: hash_mapのパフォーマンステスト

## テスト品質指標

### カバレッジ目標

- **行カバレッジ**: 95%以上
- **関数カバレッジ**: 98%以上
- **分岐カバレッジ**: 90%以上

### パフォーマンス指標

- **メモリ使用量**: std::ライブラリと同等以下
- **実行速度**: std::ライブラリの80%以上の性能
- **コンパイル時間**: 標準ライブラリより高速

## テスト作成ガイドライン

### 1. テストケース命名

- 日本語でテストの内容を明確に記述
- SECTIONを使用して論理的にグループ化
- エッジケースは明確に「エッジケース」と記載

```cpp
TEST_CASE("bluestl::vector 基本動作", "[vector]") {
    SECTION("デフォルトコンストラクタ") {
        // テストコード
    }
    
    SECTION("サイズ指定コンストラクタ") {
        // テストコード
    }
}
```

### 2. メモリ安全性テスト

デストラクタ呼び出しを確認するテストパターン：

```cpp
SECTION("メモリリークテスト") {
    bool destroyed = false;
    {
        container<TestType> c;
        c.emplace_back(42, &destroyed);
        REQUIRE(!destroyed);
    } // デストラクタが自動的に呼ばれる
    REQUIRE(destroyed);
}
```

### 3. パフォーマンステスト

大量データでの性能テスト：

```cpp
SECTION("大量データのパフォーマンス") {
    const int N = 100000;
    container<int> c;
    
    for (int i = 0; i < N; ++i) {
        c.push_back(i);
    }
    REQUIRE(c.size() == N);
}
```

### 4. エラー処理テスト

アサーション発生条件の説明：

```cpp
SECTION("範囲外アクセス（アサート発生のため直接テストしない）") {
    container<int> c = {1, 2, 3};
    
    // 正常なアクセス範囲の確認
    REQUIRE(c.at(0) == 1);
    REQUIRE(c.at(2) == 3);
    
    // 注意：c.at(10)はアサーション発生するためテストしない
}
```

## 継続的インテグレーション

### 推奨CI設定

```yaml
# GitHub Actions例
- name: テスト実行（基本）
  run: ./run_tests.sh

- name: テスト実行（カバレッジ）
  run: ./run_tests.sh --coverage

- name: テスト実行（サニタイザ）
  run: ./run_tests.sh --sanitizers

- name: ベンチマーク実行
  run: ./run_tests.sh --benchmarks --release
```

## トラブルシューティング

### よくある問題

1. **lcovが見つからない**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install lcov
   
   # macOS
   brew install lcov
   ```

2. **Google Benchmarkが見つからない**
   - CMakeが自動的にダウンロードします
   - インターネット接続を確認してください

3. **サニタイザエラー**
   - メモリリークやバッファオーバーフローを示します
   - エラーメッセージを詳しく確認してコードを修正してください

4. **テスト失敗**
   - 詳細な出力で原因を特定してください：
   ```bash
   ./run_tests.sh --verbose
   ```

## レポート解釈

### カバレッジレポート

- `build/coverage_html/index.html`をブラウザで開く
- 赤い行：未実行コード
- 緑の行：実行済みコード
- 黄色い行：部分的に実行されたコード

### ベンチマークレポート

- 実行時間（ns, μs, ms）
- スループット（items/second）
- メモリ使用量
- std::ライブラリとの比較

この包括的なテストスイートにより、BlueStlライブラリの品質と性能を継続的に監視・改善できます。