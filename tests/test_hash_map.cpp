// テストコード for hash_map.h
#include "../include/stl/hash_map.h"
#include "../include/stl/allocator.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "test_allocator.h"

// BlueSTLのテスト用のセクション
TEST_CASE("hash_map 基本機能テスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map");
    bluestl::hash_map<int, std::string, bluestl::allocator> map(alloc);

    SECTION("初期状態の確認")
    {
        CHECK(map.size() == 0);
        CHECK(map.empty());
        CHECK(map.capacity() == bluestl::hash_map<int, std::string, bluestl::allocator>::initial_capacity);
    }

    SECTION("要素の追加と取得")
    {
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        CHECK(map.size() == 3);
        CHECK_FALSE(map.empty());
        CHECK(map.at(1) == "one");
        CHECK(map.at(2) == "two");
        CHECK(map.at(3) == "three");
    }

    SECTION("[]演算子のテスト")
    {
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

    SECTION("要素の存在確認")
    {
        map.insert(1, "one");
        map.insert(2, "two");

        CHECK(map.contains(1));
        CHECK(map.contains(2));
        CHECK_FALSE(map.contains(3));

        // findメソッドのテスト
        CHECK(map.find(1) != map.end());
        CHECK(map.find(3) == map.end());
    }

    SECTION("要素の削除")
    {
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

    SECTION("イテレーターのテスト")
    {
        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        // 要素を反復処理するイテレーターのテスト
        int count = 0;
        for (const auto &pair : map)
        {
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

TEST_CASE("hash_map コピーと移動のテスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<int, std::string, bluestl::allocator> map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    SECTION("コピーコンストラクターのテスト")
    {
        bluestl::hash_map<int, std::string, bluestl::allocator> copy(map);

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

    SECTION("コピー代入演算子のテスト")
    {
        bluestl::hash_map<int, std::string, bluestl::allocator> copy(alloc);
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

    SECTION("ムーブコンストラクターのテスト")
    {
        bluestl::hash_map<int, std::string, bluestl::allocator> original(alloc);
        original.insert(1, "one");
        original.insert(2, "two");

        bluestl::hash_map<int, std::string, bluestl::allocator> moved(std::move(original));

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

    SECTION("ムーブ代入演算子のテスト")
    {
        bluestl::hash_map<int, std::string, bluestl::allocator> original(alloc);
        original.insert(1, "one");
        original.insert(2, "two");

        bluestl::hash_map<int, std::string, bluestl::allocator> moved(alloc);
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

TEST_CASE("hash_map リハッシュのテスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<int, int, bluestl::allocator> map(alloc);

    // 初期容量を超える要素を追加してリハッシュを発生させる
    const int initial_capacity = bluestl::hash_map<int, int, bluestl::allocator>::initial_capacity;
    const int elements_to_add = static_cast<int>(initial_capacity * bluestl::hash_map<int, int, bluestl::allocator>::max_load_factor) + 5;

    for (int i = 0; i < elements_to_add; ++i)
    {
        map.insert(i, i * 10);
    }

    // リハッシュが発生し、容量が増加していることを確認
    CHECK(map.capacity() > initial_capacity);

    // すべての要素がアクセス可能であることを確認
    for (int i = 0; i < elements_to_add; ++i)
    {
        CHECK(map.contains(i));
        CHECK(map[i] == i * 10);
    }
}

TEST_CASE("hash_map トゥームストーンテスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<int, std::string, bluestl::allocator> map(alloc);

    // 要素を追加
    for (int i = 0; i < 10; ++i)
    {
        map.insert(i, std::to_string(i));
    }

    // いくつかの要素を削除してトゥームストーンを作成
    for (int i = 0; i < 10; i += 2)
    {
        map.erase(i);
    }

    // 削除した要素が見つからないことを確認
    for (int i = 0; i < 10; i += 2)
    {
        CHECK_FALSE(map.contains(i));
    }

    // 残っている要素が正しく取得できることを確認
    for (int i = 1; i < 10; i += 2)
    {
        CHECK(map.contains(i));
        CHECK(map[i] == std::to_string(i));
    }

    // 新しい要素を追加し、削除された場所が再利用されることを確認
    for (int i = 0; i < 10; i += 2)
    {
        map.insert(i, std::to_string(i * 100));
    }

    CHECK(map.size() == 10);

    // 新しく追加された要素を確認
    for (int i = 0; i < 10; i += 2)
    {
        CHECK(map.contains(i));
        CHECK(map[i] == std::to_string(i * 100));
    }
}

TEST_CASE("hash_map constイテレーターのテスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<int, std::string, bluestl::allocator> map(alloc);

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    // constイテレーターを使用して要素にアクセス
    const auto &const_map = map;
    int count = 0;

    for (auto it = const_map.cbegin(); it != const_map.cend(); ++it)
    {
        CHECK((it->first == 1 || it->first == 2 || it->first == 3));
        CHECK((it->second == "one" || it->second == "two" || it->second == "three"));
        ++count;
    }

    CHECK(count == 3);
}

struct CustomKey
{
    int id;
    std::string name;

    bool operator==(const CustomKey &other) const
    {
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
}


TEST_CASE("hash_map カスタムキータイプのテスト", "[hash_map]")
{

    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<CustomKey, int, bluestl::allocator> map(alloc);

    // カスタムキーを使用してマップにデータを追加
    map.insert({1, "Alice"}, 100);
    map.insert({2, "Bob"}, 200);
    map.insert({3, "Charlie"}, 300);

    CHECK(map.size() == 3);
    CHECK(map.contains({1, "Alice"}));
    CHECK(map.contains({2, "Bob"}));
    CHECK(map.contains({3, "Charlie"}));

    CHECK(map[{1, "Alice"}] == 100);
    CHECK(map[{2, "Bob"}] == 200);
    CHECK(map[{3, "Charlie"}] == 300);

    // キーの一部のみが一致する場合はfalse
    CHECK_FALSE(map.contains({1, "Bob"}));
}

TEST_CASE("hash_map クリアと再利用のテスト", "[hash_map]")
{
    auto alloc = TestAllocator("test_hash_map3");
    bluestl::hash_map<int, std::string, bluestl::allocator> map(alloc);

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
