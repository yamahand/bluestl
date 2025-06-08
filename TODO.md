## 優先度1: 基本コンテナとビュー（高頻度使用、他の機能の基盤）

- [ ] span / array_view
  配列やバッファの一部を安全に参照するための軽量ビュー。

- [ ] array
  固定サイズ配列の安全なラッパー。

- [ ] string_view
  文字列の非所有ビュー。文字列操作の高速化とメモリ使用量削減。

- [ ] string
  可変長文字列

- [x] small_vector / small_buffer_vector
  小規模な配列をヒープを使わずに扱うための可変長配列。
  固定長＋オーバーフロー時のみヒープ確保。

## 優先度2: エラーハンドリングと基本アルゴリズム

- [ ] expected / result
  エラーハンドリングのための型。例外を使わない安全なエラー処理。

- [ ] algorithm functions
  sort, find, binary_search等の基本アルゴリズム関数群。

- [ ] iterator utilities
  イテレータヘルパーとアダプター。範囲ベース操作の支援。

- [x] optional
  値の有無を明示的に扱うための型。

- [x] variant
  複数型のいずれかを格納できる型。状態管理やイベントデータなどで利用。

## 優先度3: コールバックとメモリ管理

- [ ] function / delegate
  コールバックやイベントリスナー、スクリプトバインディングなど。

- [ ] unique_ptr
  所有権管理のためのスマートポインタ。RAII原則の実現。

- [ ] memory utilities
  align, launder等のメモリ操作ユーティリティ。

- [x] fixed_string / small_string
  固定長または小容量の文字列型。

## 優先度4: 高性能コンテナ

- [ ] flat_map / flat_set
  小規模な辞書や集合を連続メモリで管理し、高速なイテレーションを実現。

- [ ] ring_buffer / circular_buffer
  サウンドバッファやイベントキュー、ネットワークパケット管理などで利用。

- [ ] stack / queue
  基本的なスタックとキューのアダプター。

- [ ] deque / fixed_deque
  両端キュー。効率的な前後への要素追加・削除。

- [ ] bitset / dynamic_bitset
  ビットフラグ管理やエンティティ管理などで頻繁に利用されます。

## 優先度5: 連想コンテナと特殊コンテナ

- [ ] unordered_map / unordered_set
  ハッシュテーブルベースの連想コンテナ。

- [ ] priority_queue / fixed_priority_queue
  優先度付きキュー。ヒープベースの効率的な優先度管理。

- [ ] list / forward_list
  連結リスト。頻繁な挿入・削除に適した構造。

- [ ] intrusive_list / intrusive_set
  メモリ確保を最小限に抑えたい場合や、オブジェクト自身がリストノードを持つ場合に利用。

## 優先度6: メタプログラミングとコンパイル時機能

- [ ] type_traits
  型特性の検査とメタプログラミング支援。

- [ ] concepts
  C++20コンセプトによる型制約。コンパイル時型安全性の向上。

- [ ] constexpr_vector / constexpr_string
  コンパイル時計算可能なコンテナ。メタプログラミングの強化。

## 優先度7: ユーティリティと数値計算

- [ ] chrono utilities
  時間計測とタイマー機能。高精度時間操作。

- [ ] random number generators
  擬似乱数生成器。ゲーム開発や数値計算で必要。

- [ ] numeric algorithms
  accumulate, reduce, transform_reduce等の数値計算アルゴリズム。

- [ ] format / print
  文字列フォーマット機能。printf代替の型安全な文字列生成。

## 優先度8: 高度なメモリ管理

- [ ] pool_allocator / object_pool
  頻繁な生成・破棄が発生するオブジェクトのための高速メモリ管理。

- [ ] shared_ptr / weak_ptr
  参照カウント型スマートポインタ。複雑な所有権管理。

## 優先度9: 並行処理（最後に実装、複雑性が高い）

- [ ] atomic
  アトミック操作のサポート。マルチスレッド環境での安全なデータアクセス。

- [ ] mutex / lock_guard / unique_lock
  ミューテックスと関連ロック機構。スレッド同期のための基本要素。

- [ ] thread / this_thread
  スレッド管理とスレッドローカル操作。

- [ ] condition_variable
  スレッド間の条件待ち同期。

## 優先度10: 高度な機能

- [ ] ranges
  C++20範囲ライブラリの代替実装。関数型プログラミングスタイルの支援。
