- [ ] bitset / dynamic_bitset  
  ビットフラグ管理やエンティティ管理などで頻繁に利用されます。

- [ ] small_vector / small_buffer_vector  
  小規模な配列をヒープを使わずに扱うための可変長配列。
  固定長＋オーバーフロー時のみヒープ確保。

- [ ] ring_buffer / circular_buffer  
  サウンドバッファやイベントキュー、ネットワークパケット管理などで利用。

- [ ] span / array_view  
  配列やバッファの一部を安全に参照するための軽量ビュー。

- [x] optional  
  値の有無を明示的に扱うための型。

- [ ] variant  
  複数型のいずれかを格納できる型。状態管理やイベントデータなどで利用。

- [ ] function / delegate  
  コールバックやイベントリスナー、スクリプトバインディングなど。

- [ ] flat_map / flat_set  
  小規模な辞書や集合を連続メモリで管理し、高速なイテレーションを実現。

- [ ] intrusive_list / intrusive_set  
  メモリ確保を最小限に抑えたい場合や、オブジェクト自身がリストノードを持つ場合に利用。

- [ ] pool_allocator / object_pool  
  頻繁な生成・破棄が発生するオブジェクトのための高速メモリ管理。

- [ ] fixed_string / small_string  
  固定長または小容量の文字列型。

- [ ] string
  可変長文字列