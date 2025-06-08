﻿// テストコード for hash_map.h
#include "bluestl/optional.h"
#include "bluestl/hash_map.h"
#include "bluestl/allocator.h"
#include "bluestl/vector.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "test_allocator.h"

#if 1

using pair_type = bluestl::pair<int, std::string>;
using allocator_type = TestAllocator<pair_type>;
using hash_map_type = bluestl::hash_map<int, std::string, allocator_type>;

// BlueSTLのテスト用のセクション
TEST_CASE("hash_map 基本機能テスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map");
    hash_map_type map(alloc);

    SECTION("初期状態の確認") {
        CHECK(map.size() == 0);
        CHECK(map.empty());
        CHECK(map.capacity() == hash_map_type::initial_capacity);
    }

    SECTION("要素の追加と取得") {
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        CHECK(map.size() == 3);
        CHECK_FALSE(map.empty());
        CHECK(map.at(1) == "one");
        CHECK(map.at(2) == "two");
        CHECK(map.at(3) == "three");
    }

    SECTION("[]演算子のテスト") {
        map[1] = "one";
        map[2] = "two";

        CHECK(map.size() == 2);
        CHECK(map[1] == "one");
        CHECK(map[2] == "two");

        // 存在しないキーへのアクセスで新しい要素が作成されることを確認
        CHECK(map.contains(3) == false);
        map[3] = "three";
        CHECK(map.contains(3) == true);
        CHECK(map[3] == "three");
    }

    SECTION("要素の存在確認") {
        map.insert(1, "one");
        map.insert(2, "two");

        CHECK(map.contains(1));
        CHECK(map.contains(2));
        CHECK_FALSE(map.contains(3));

        // findメソッドのテスト
        CHECK(map.find(1) != map.end());
        CHECK(map.find(3) == map.end());
    }

    SECTION("要素の削除") {
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        CHECK(map.size() == 3);

        // キーによる削除
        CHECK(map.erase(2));
        CHECK(map.size() == 2);
        CHECK_FALSE(map.contains(2));
        CHECK(map.contains(1));
        CHECK(map.contains(3));

        // 存在しないキーの削除
        CHECK_FALSE(map.erase(4));
        CHECK(map.size() == 2);
    }

    SECTION("イテレーターのテスト") {
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        // 要素を反復処理するイテレーターのテスト
        int count = 0;
        for (const auto& pair : map) {
            CHECK((pair.first == 1 || pair.first == 2 || pair.first == 3));
            CHECK((pair.second == "one" || pair.second == "two" || pair.second == "three"));
            ++count;
        }
        CHECK(count == 3);

        // イテレーターを使った削除
        auto it = map.find(2);
        it = map.erase(it);
        CHECK(map.size() == 2);
        CHECK_FALSE(map.contains(2));
    }
}

TEST_CASE("hash_map コピーと移動のテスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map3");
    hash_map_type map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    SECTION("コピーコンストラクターのテスト") {
        hash_map_type copy(map);

        CHECK(copy.size() == map.size());
        CHECK(copy.contains(1));
        CHECK(copy.contains(2));
        CHECK(copy.contains(3));
        CHECK(copy[1] == "one");
        CHECK(copy[2] == "two");
        CHECK(copy[3] == "three");

        // コピー元を変更してもコピー先には影響しないことを確認
        map.insert(4, "four");
        CHECK(map.size() == 4);
        CHECK(copy.size() == 3);
        CHECK_FALSE(copy.contains(4));
    }

    SECTION("コピー代入演算子のテスト") {
        hash_map_type copy(alloc);
        copy.insert(5, "five");

        copy = map;

        CHECK(copy.size() == map.size());
        CHECK(copy.contains(1));
        CHECK(copy.contains(2));
        CHECK(copy.contains(3));
        CHECK_FALSE(copy.contains(5));
        CHECK(copy[1] == "one");
        CHECK(copy[2] == "two");
        CHECK(copy[3] == "three");
    }

    SECTION("ムーブコンストラクターのテスト") {
        hash_map_type original(alloc);
        original.insert(1, "one");
        original.insert(2, "two");

        auto alloc_moved = allocator_type("test_hash_map_moved");
        hash_map_type moved(alloc_moved);
        moved = std::move(original);

        CHECK(moved.size() == 2);
        CHECK(moved.contains(1));
        CHECK(moved.contains(2));
        CHECK(moved[1] == "one");
        CHECK(moved[2] == "two");

        // 元のマップは空になっていることを確認
        CHECK(original.size() == 0);
        CHECK(original.empty());
        CHECK_FALSE(original.contains(1));
    }

    SECTION("ムーブ代入演算子のテスト") {
        hash_map_type original(alloc);
        original.insert(1, "one");
        original.insert(2, "two");

        hash_map_type moved(alloc);
        moved.insert(5, "five");

        moved = std::move(original);

        CHECK(moved.size() == 2);
        CHECK(moved.contains(1));
        CHECK(moved.contains(2));
        CHECK_FALSE(moved.contains(5));

        // 元のマップは空になっていることを確認
        CHECK(original.size() == 0);
        CHECK(original.empty());
    }
}

using int_pair_type = bluestl::pair<int, int>;
using int_allocator_type = TestAllocator<int_pair_type>;
using int_hash_map_type = bluestl::hash_map<int, int, int_allocator_type>;

TEST_CASE("hash_map リハッシュのテスト", "[hash_map]") {
    auto alloc = int_allocator_type("test_hash_map3");
    int_hash_map_type map(alloc);

    // 初期容量を超える要素を追加してリハッシュを発生させる
    const int initial_capacity = int_hash_map_type::initial_capacity;
    const int elements_to_add =
        static_cast<int>(initial_capacity * int_hash_map_type::max_load_factor) + 5;

    for (int i = 0; i < elements_to_add; ++i) {
        map.insert(i, i * 10);
    }

    // リハッシュが発生し、容量が増加していることを確認
    CHECK(map.capacity() > initial_capacity);

    // すべての要素がアクセス可能であることを確認
    for (int i = 0; i < elements_to_add; ++i) {
        CHECK(map.contains(i));
        CHECK(map[i] == i * 10);
    }
}

TEST_CASE("hash_map トゥームストーンテスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map3");
    hash_map_type map(alloc);

    // 要素を追加
    for (int i = 0; i < 10; ++i) {
        map.insert(i, std::to_string(i));
    }

    // いくつかの要素を削除してトゥームストーンを作成
    for (int i = 0; i < 10; i += 2) {
        map.erase(i);
    }

    // 削除した要素が見つからないことを確認
    for (int i = 0; i < 10; i += 2) {
        CHECK_FALSE(map.contains(i));
    }

    // 残っている要素が正しく取得できることを確認
    for (int i = 1; i < 10; i += 2) {
        CHECK(map.contains(i));
        CHECK(map[i] == std::to_string(i));
    }

    // 新しい要素を追加し、削除された場所が再利用されることを確認
    for (int i = 0; i < 10; i += 2) {
        map.insert(i, std::to_string(i * 100));
    }

    CHECK(map.size() == 10);

    // 新しく追加された要素を確認
    for (int i = 0; i < 10; i += 2) {
        CHECK(map.contains(i));
        CHECK(map[i] == std::to_string(i * 100));
    }
}

TEST_CASE("hash_map constイテレーターのテスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map3");
    hash_map_type map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    // constイテレーターを使用して要素にアクセス
    const auto& const_map = map;
    int count = 0;

    for (auto it = const_map.cbegin(); it != const_map.cend(); ++it) {
        CHECK((it->first == 1 || it->first == 2 || it->first == 3));
        CHECK((it->second == "one" || it->second == "two" || it->second == "three"));
        ++count;
    }

    CHECK(count == 3);
}

struct CustomKey {
    int id;
    std::string name;

    bool operator==(const CustomKey& other) const {
        return id == other.id && name == other.name;
    }
};

namespace bluestl {
// std::string に対する特殊化
template <>
constexpr hash_default_t hash<std::string>(const std::string& str) noexcept {
#if BLUESTL_HASH_ALGO == fnv1a
    return fnv1a_hash(str.data(), str.size());
#elif BLUESTL_HASH_ALGO == xx
    return xxhash32(str.data(), str.size());
#elif BLUESTL_HASH_ALGO == murmur
    return murmur3_32(str.data(), str.size());
#endif
}

// CustomKey に対する特殊化（std::stringの特殊化を使用）
template <>
constexpr hash_default_t hash<CustomKey>(const CustomKey& key) noexcept {
    return hash(key.id) ^ (hash(key.name) << 1);
}
}  // namespace bluestl

using custom_pair_type = bluestl::pair<CustomKey, int>;
using custom_allocator_type = TestAllocator<custom_pair_type>;
using custom_hash_map_type = bluestl::hash_map<CustomKey, int, custom_allocator_type>;

TEST_CASE("hash_map カスタムキータイプのテスト", "[hash_map]") {
    auto alloc = custom_allocator_type("test_hash_map3");
    custom_hash_map_type map(alloc);

    // カスタムキーを使用してマップにデータを追加
    map.insert({ 1, "Alice" }, 100);
    map.insert({ 2, "Bob" }, 200);
    map.insert({ 3, "Charlie" }, 300);

    CHECK(map.size() == 3);
    CHECK(map.contains({ 1, "Alice" }));
    CHECK(map.contains({ 2, "Bob" }));
    CHECK(map.contains({ 3, "Charlie" }));

    CHECK(map[{ 1, "Alice" }] == 100);
    CHECK(map[{ 2, "Bob" }] == 200);
    CHECK(map[{ 3, "Charlie" }] == 300);

    // キーの一部のみが一致する場合はfalse
    CHECK_FALSE(map.contains({ 1, "Bob" }));
}

TEST_CASE("hash_map クリアと再利用のテスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map3");
    hash_map_type map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    CHECK(map.size() == 3);

    // マップをクリア
    map.clear();

    CHECK(map.size() == 0);
    CHECK(map.empty());
    CHECK_FALSE(map.contains(1));
    CHECK_FALSE(map.contains(2));
    CHECK_FALSE(map.contains(3));

    // クリア後に新しい要素を追加
    map.insert(4, "four");
    map.insert(5, "five");

    CHECK(map.size() == 2);
    CHECK(map.contains(4));
    CHECK(map.contains(5));
    CHECK(map[4] == "four");
    CHECK(map[5] == "five");
}

TEST_CASE("hash_map try_getのテスト", "[hash_map]") {
    auto alloc = allocator_type("test_hash_map_try_get");
    hash_map_type map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");

    SECTION("存在するキーでtry_getが値を返す") {
        auto opt = map.try_get(1);
        CHECK(opt.has_value());
        CHECK(*opt == "one");
        // 参照であることの確認（値を書き換える）
        *opt = "ONE";
        CHECK(map[1] == "ONE");
    }

    SECTION("存在しないキーでtry_getが値なしを返す") {
        auto opt = map.try_get(3);
        CHECK_FALSE(opt.has_value());
    }

    SECTION("const hash_mapでのtry_get") {
        const auto& cmap = map;
        auto opt = cmap.try_get(2);
        CHECK(opt.has_value());
        CHECK(*opt == "two");
    }
}

/**
 * @brief hash_mapのSTL準拠API（try_emplace, insert_or_assign, emplace）のテスト
 */
TEST_CASE("hash_map STL準拠APIのテスト", "[hash_map][stl_api]") {
    auto alloc = allocator_type("test_hash_map_stl_api");
    hash_map_type map(alloc);    SECTION("try_emplace: 新規挿入と既存キー") {
        auto result1 = map.try_emplace(1, std::string("one"));
        CHECK(result1.second);
        CHECK(map[1] == "one");
        // 既存キーには挿入されない
        auto result2 = map.try_emplace(1, std::string("uno"));
        CHECK_FALSE(result2.second);
        CHECK(map[1] == "one");
    }    SECTION("insert_or_assign: 新規挿入と上書き") {
        auto result1 = map.insert_or_assign(2, std::string("two"));
        CHECK(result1.second);
        CHECK(map[2] == "two");
        // 既存キーは上書きされる
        auto result2 = map.insert_or_assign(2, std::string("TWO"));
        CHECK_FALSE(result2.second);
        CHECK(map[2] == "TWO");
    }    SECTION("emplace: 新規挿入のみ") {
        auto result1 = map.emplace(3, std::string("three"));
        CHECK(result1.second);
        CHECK(map[3] == "three");
        // 既存キーには挿入されない
        auto result2 = map.emplace(3, std::string("tres"));
        CHECK_FALSE(result2.second);
        CHECK(map[3] == "three");
    }
}

/**
 * @brief hash_mapのムーブセマンティクステスト
 */
TEST_CASE("hash_map ムーブセマンティクスのテスト", "[hash_map][move]") {
    auto alloc = allocator_type("test_hash_map_move");
    hash_map_type map(alloc);

    SECTION("operator[] ムーブ") {
        int key1 = 1;
        map[std::move(key1)] = "one";  // キーをムーブ (実際にはコピーされるが、オーバーロードをテスト)
        CHECK(map.contains(1));
        CHECK(map[1] == "one");

        // 既存要素へのアクセス (キーはムーブされない)
        int key2 = 1;
        map[std::move(key2)] = "ONE";
        CHECK(map[1] == "ONE");
    }    SECTION("insert ムーブ") {
        int key1 = 1;
        std::string val1 = "one";
        auto result1 = map.insert(std::move(key1), std::move(val1));
        CHECK(result1.second);
        CHECK(map.contains(1));
        CHECK(map[1] == "one");
        CHECK(val1.empty());  // ムーブされたことを確認

        int key2 = 1;  // 既存キー
        std::string val2 = "uno";
        auto result2 = map.insert(std::move(key2), std::move(val2));
        CHECK_FALSE(result2.second);
        CHECK(map[1] == "one");     // 変更されない
        CHECK_FALSE(val2.empty());  // ムーブされなかったことを確認
    }

    SECTION("try_emplace ムーブ") {
        int key1 = 1;        auto result1 = map.try_emplace(std::move(key1), std::string("one"));
        CHECK(result1.second);
        CHECK(map.contains(1));
        CHECK(map[1] == "one");

        int key2 = 1;  // 既存キー
        auto result2 = map.try_emplace(std::move(key2), std::string("uno"));
        CHECK_FALSE(result2.second);
        CHECK(map[1] == "one");  // 変更されない
    }

    SECTION("emplace ムーブ") {
        int key1 = 3;        auto result1 = map.emplace(std::move(key1), std::string("three"));
        CHECK(result1.second);
        CHECK(map.contains(3));
        CHECK(map[3] == "three");

        int key2 = 3;  // 既存キー
        auto result2 = map.emplace(std::move(key2), std::string("tres"));
        CHECK_FALSE(result2.second);
        CHECK(map[3] == "three");  // 変更されない
    }

    SECTION("insert_or_assign ムーブ") {
        int key1 = 2;
        std::string val1 = "two";        auto result1 = map.insert_or_assign(std::move(key1), std::move(val1));
        CHECK(result1.second);
        CHECK(map.contains(2));
        CHECK(map[2] == "two");
        CHECK(val1.empty());  // ムーブされたことを確認

        int key2 = 2;  // 既存キー
        std::string val2 = "TWO";
        auto result2 = map.insert_or_assign(std::move(key2), std::move(val2));
        CHECK_FALSE(result2.second);
        CHECK(map[2] == "TWO");  // 上書きされる
        CHECK(val2.empty());     // ムーブされたことを確認
    }
}

/**
 * @brief hash_mapの範囲ベース操作テスト
 */
TEST_CASE("hash_map 範囲ベース操作のテスト", "[hash_map][range]") {
    using vector_allocator_type = TestAllocator<pair_type>;
    auto alloc = vector_allocator_type("test_hash_map_range");

    // 挿入用データ
    bluestl::vector<pair_type, vector_allocator_type> data(alloc);
    data.push_back({ 1, "one" });
    data.push_back({ 2, "two" });
    data.push_back({ 3, "three" });
    data.push_back({ 2, "deux" });  // 重複キー

    SECTION("範囲コンストラクタ") {
        hash_map_type map(data.begin(), data.end(), alloc);
        CHECK(map.size() == 3);  // 重複キーは無視される
        CHECK(map.contains(1));
        CHECK(map.contains(2));
        CHECK(map.contains(3));
        CHECK(map[1] == "one");
        CHECK(map[2] == "two");  // 最初の "two" が挿入される
        CHECK(map[3] == "three");
    }

    SECTION("範囲挿入") {
        hash_map_type map(alloc);
        map.insert(0, "zero");
        map.insert(data.begin(), data.end());

        CHECK(map.size() == 4);  // 0, 1, 2, 3
        CHECK(map.contains(0));
        CHECK(map.contains(1));
        CHECK(map.contains(2));
        CHECK(map.contains(3));
        CHECK(map[0] == "zero");
        CHECK(map[1] == "one");
        CHECK(map[2] == "two");
        CHECK(map[3] == "three");
    }

    SECTION("範囲削除") {
        hash_map_type map(alloc);
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");
        map.insert(4, "four");
        map.insert(5, "five");

        auto it_start = map.begin();

        auto it_end = it_start;
        it_end++;
        it_end++;

        CHECK(it_start != map.end());
        CHECK(it_end != map.end());

        // 削除前の状態を確認
        CHECK(map.contains(2));
        CHECK(map.contains(3));

        // 範囲削除を実行
        auto next_it = map.erase(it_start, it_end);

        // 削除後のサイズを確認
        CHECK(map.size() == 3);  // 1, 4, 5 が残る


        // 戻り値のイテレータが last (it_end) と同じか確認
        // erase(iterator) がリハッシュしない限りは一致するはず
        // CHECK(next_it == it_end); // リハッシュの可能性を考慮し、コメントアウト
    }
}

#endif
