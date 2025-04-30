#include <catch2/catch_test_macros.hpp>
#include "bluestl/vector.h"
#include "test_allocator.h"
#include <string> // テスト用にstringを使用
#include <compare> // std::compare_three_way_result

// テスト用の簡単な構造体
struct TestStruct {
    int id;
    std::string name;
    static int construction_count;
    static int destruction_count;
    static int copy_construction_count;
    static int move_construction_count;
    static int copy_assignment_count;
    static int move_assignment_count;

    TestStruct(int i = 0, std::string n = "") : id(i), name(std::move(n)) { construction_count++; }
    ~TestStruct() { destruction_count++; }
    TestStruct(const TestStruct& other) : id(other.id), name(other.name) { copy_construction_count++; construction_count++; }
    TestStruct(TestStruct&& other) noexcept : id(other.id), name(std::move(other.name)) { move_construction_count++; construction_count++; other.id = -1;} // ムーブ元を変更
    TestStruct& operator=(const TestStruct& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            copy_assignment_count++;
        }
        return *this;
    }
     TestStruct& operator=(TestStruct&& other) noexcept {
        if (this != &other) {
            id = other.id;
            name = std::move(other.name);
            move_assignment_count++;
            other.id = -1; // ムーブ元を変更
        }
        return *this;
    }

    // 比較演算子
    bool operator==(const TestStruct& rhs) const { return id == rhs.id && name == rhs.name; }
    auto operator<=>(const TestStruct& rhs) const {
        if (auto cmp = id <=> rhs.id; cmp != 0) return cmp;
        return name <=> rhs.name;
    }


    static void reset_counts() {
        construction_count = 0;
        destruction_count = 0;
        copy_construction_count = 0;
        move_construction_count = 0;
        copy_assignment_count = 0;
        move_assignment_count = 0;
    }
};

int TestStruct::construction_count = 0;
int TestStruct::destruction_count = 0;
int TestStruct::copy_construction_count = 0;
int TestStruct::move_construction_count = 0;
int TestStruct::copy_assignment_count = 0;
int TestStruct::move_assignment_count = 0;


TEST_CASE("vector の基本操作", "[vector]") {
    TestAllocator<int> allocator_("test_vector_basic");
    // アロケータ型を明示的に指定
    bluestl::vector<int, TestAllocator<int>> vec(allocator_);
    const auto& allocator = vec.get_allocator_ref();

    SECTION("初期状態") {
        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.capacity() == 0);
        REQUIRE(vec.data() == nullptr);
        REQUIRE(allocator.get_allocated_bytes() == 0);
    }

    SECTION("要素の追加 (push_back)") {
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);

        REQUIRE_FALSE(vec.empty());
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
        REQUIRE(vec.front() == 1);
        REQUIRE(vec.back() == 3);
        REQUIRE(vec.at(1) == 2);

        // 容量の確認 (実装依存だが、0より大きいことは確実)
        REQUIRE(vec.capacity() >= vec.size());
        REQUIRE(vec.get_allocator_ref().get_allocated_bytes() > 0);
        REQUIRE(vec.get_allocator_ref().get_allocation_count() > 0); // 再確保が発生する可能性がある
    }

     SECTION("要素の追加 (emplace_back)") {
        TestStruct::reset_counts();
        TestAllocator<TestStruct> tmp_allocator("emplace_back_alloc");
        bluestl::vector<TestStruct, TestAllocator<TestStruct>> svec(tmp_allocator);
        const auto& struct_allocator = svec.get_allocator_ref();

        svec.emplace_back(1, "one");
        svec.emplace_back(2, "two");

        REQUIRE(svec.size() == 2);
        REQUIRE(svec[0].id == 1);
        REQUIRE(svec[0].name == "one");
        REQUIRE(svec[1].id == 2);
        REQUIRE(svec[1].name == "two");
        // emplace_back は直接構築するのでコピー/ムーブ構築は発生しないはず (再確保時を除く)
        // REQUIRE(TestStruct::copy_construction_count == 0); // 再確保でムーブが発生する可能性あり
        // REQUIRE(TestStruct::move_construction_count <= svec.size()); // 再確保でムーブが発生する可能性あり
        REQUIRE(TestStruct::construction_count >= 2); // 直接構築 + 再確保時のムーブ構築
        REQUIRE(svec.get_allocator_ref().get_allocated_bytes() > 0);
    }


    SECTION("要素の削除 (pop_back)") {
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        size_t initial_capacity = vec.capacity();

        vec.pop_back();
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec.back() == 2);
        // pop_backは容量を変更しない
        REQUIRE(vec.capacity() == initial_capacity);
    }

    SECTION("クリア操作 (clear)") {
        vec.push_back(1);
        vec.push_back(2);
        size_t capacity_before_clear = vec.capacity();
        size_t allocated_before_clear = vec.get_allocator_ref().get_allocated_bytes();
        size_t deallocation_count_before_clear = allocator.get_deallocation_count(); // クリア前の解放回数を記録

        vec.clear();
        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0);
        // clearは容量を変更しない
        REQUIRE(vec.capacity() == capacity_before_clear);
        // clearはメモリを解放しない
        REQUIRE(allocator.get_allocated_bytes() == allocated_before_clear);
        REQUIRE(allocator.get_deallocation_count() == deallocation_count_before_clear); // clearでは解放されない
    }

    SECTION("容量確保 (reserve)") {
        REQUIRE(vec.capacity() == 0);
        vec.reserve(10);
        REQUIRE(vec.capacity() >= 10);
        REQUIRE(allocator.get_allocated_bytes() >= 10 * sizeof(int));
        REQUIRE(allocator.get_allocation_count() == 1);

        // 要素追加が正常に行われるか
        vec.push_back(4);
        vec.push_back(5);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 4);
        REQUIRE(vec[1] == 5);
        // reserve後なので再確保は発生しないはず
        REQUIRE(allocator.get_allocation_count() == 1);
    }

     SECTION("容量縮小 (shrink_to_fit)") {
        vec.reserve(20);
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        REQUIRE(vec.capacity() >= 20);
        size_t alloc_count_before = allocator.get_allocation_count();

        vec.shrink_to_fit();
        REQUIRE(vec.capacity() == 3);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 3);
        REQUIRE(allocator.get_allocated_bytes() == 3 * sizeof(int));
        // 再確保が発生する
        REQUIRE(allocator.get_allocation_count() > alloc_count_before);
        REQUIRE(allocator.get_deallocation_count() > 0);

        // サイズ0の場合
        vec.clear();
        vec.reserve(10);
        REQUIRE(vec.capacity() >= 10);
        vec.shrink_to_fit();
        REQUIRE(vec.capacity() == 0);
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.data() == nullptr);
        // メモリが解放されているはず
        // (ただし、他のテストの影響で0にならない可能性もあるため、差分で確認するのがより正確)
    }


    SECTION("イテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);

        int sum = 0; // ループの外で宣言
        for (auto it = vec.begin(); it != vec.end(); ++it) { // iterator -> auto
            sum += *it;
        }
        REQUIRE(sum == 60);

        // 範囲for
        sum = 0; // 再利用
        for (int val : vec) {
            sum += val;
        }
        REQUIRE(sum == 60);
    }

    SECTION("constイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);

        const auto& cvec = vec;
        int csum = 0; // ループの外で宣言
        for (auto it = cvec.cbegin(); it != cvec.cend(); ++it) { // const_iterator -> auto
            csum += *it;
        }
        REQUIRE(csum == 60);

        // 範囲for (const)
        csum = 0; // 再利用
        for (const int& val : cvec) {
            csum += val;
        }
         REQUIRE(csum == 60);
    }

    SECTION("リバースイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);

        auto rit = vec.rbegin();
        REQUIRE(*rit == 30);
        ++rit;
        REQUIRE(*rit == 20);
        ++rit;
        REQUIRE(*rit == 10);
        ++rit;
        REQUIRE(rit == vec.rend());
    }

    SECTION("constリバースイテレータ") {
        vec.clear();
        vec.push_back(10);
        vec.push_back(20);
        vec.push_back(30);

        const auto& cvec = vec;
        auto crit = cvec.crbegin();
        REQUIRE(*crit == 30);
        ++crit;
        REQUIRE(*crit == 20);
        ++crit;
        REQUIRE(*crit == 10);
        ++crit;
        REQUIRE(crit == cvec.crend());
    }

    SECTION("要素アクセス (front/back/at/operator[])") {
        vec.clear();
        REQUIRE(vec.empty());

        vec.push_back(100);
        vec.push_back(200);
        vec.push_back(300);

        REQUIRE_FALSE(vec.empty());
        REQUIRE(vec.front() == 100);
        REQUIRE(vec.back() == 300);
        REQUIRE(vec.at(1) == 200);
        REQUIRE(vec[1] == 200);

        vec.at(1) = 250;
        REQUIRE(vec.at(1) == 250);
        REQUIRE(vec[1] == 250);

        const auto& cvec = vec;
        REQUIRE(cvec.front() == 100);
        REQUIRE(cvec.back() == 300);
        REQUIRE(cvec.at(1) == 250);
        REQUIRE(cvec[1] == 250);
    }

    SECTION("リサイズ (resize)") {
        vec.clear();
        vec.push_back(100);
        vec.push_back(250);
        size_t capacity_before = vec.capacity();

        // 拡大 (デフォルト値)
        vec.resize(5); // デフォルトコンストラクタが呼ばれる
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[0] == 100);
        REQUIRE(vec[1] == 250);
        REQUIRE(vec[2] == 0); // intのデフォルト値
        REQUIRE(vec[3] == 0);
        REQUIRE(vec[4] == 0);
        REQUIRE(vec.capacity() >= 5);

        // 拡大 (指定値)
        vec.resize(7, 42);
        REQUIRE(vec.size() == 7);
        REQUIRE(vec[5] == 42);
        REQUIRE(vec[6] == 42);

        // 縮小
        vec.resize(2);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec.back() == 250);
        // resizeは容量を変更しない場合がある
        REQUIRE(vec.capacity() >= 7);

        // 0にリサイズ
        vec.resize(0);
        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0);
    }

    SECTION("スワップ (swap)") {
        vec.clear();
        vec.push_back(100);
        vec.push_back(250);
        size_t vec_cap = vec.capacity();
        size_t vec_alloc_count = allocator.get_allocation_count();

        TestAllocator<int> tmp_allocator2("test_vector_swap");
        bluestl::vector<int, TestAllocator<int>> vec2(tmp_allocator2);
        const auto& allocator2 = vec2.get_allocator_ref();
        vec2.push_back(999);
        size_t vec2_cap = vec2.capacity();
        size_t vec2_alloc_count = allocator2.get_allocation_count();


        // アロケータが異なり、propagate_on_container_swap = true なのでアロケータも交換される
        vec.swap(vec2);

        REQUIRE(vec.size() == 1);
        REQUIRE(vec.front() == 999);
        REQUIRE(vec.capacity() == vec2_cap);
        REQUIRE(vec.get_allocator().get_name() == std::string("test_vector_swap"));
        // swapはアロケータも交換されるので参照を持っていても変わる
        REQUIRE(allocator.get_allocation_count() == vec2_alloc_count);
        REQUIRE(allocator2.get_allocation_count() == vec_alloc_count);


        REQUIRE(vec2.size() == 2);
        REQUIRE(vec2.front() == 100);
        REQUIRE(vec2.back() == 250);
        REQUIRE(vec2.capacity() == vec_cap);
        REQUIRE(vec2.get_allocator().get_name() == std::string("test_vector_basic"));
    }

    SECTION("代入 (assign)") {
        // assign(count, value)
        vec.assign(3, 7);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 7);
        REQUIRE(vec[1] == 7);
        REQUIRE(vec[2] == 7);
        REQUIRE(vec.capacity() >= 3);

        // assign(iterator, iterator)
        int arr[] = {1, 2, 3, 4, 5};
        vec.assign(arr, arr + 5);
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[4] == 5);

        // assign(initializer_list)
        vec.assign({10, 20});
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 10);
        REQUIRE(vec[1] == 20);
    }

    SECTION("挿入 (insert)") {
        vec.assign({10, 20, 30});

        // insert(pos, value)
        auto it = vec.insert(vec.begin() + 1, 15); // {10, 15, 20, 30} // auto を使用
        REQUIRE(*it == 15);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == 10);
        REQUIRE(vec[1] == 15);
        REQUIRE(vec[2] == 20);
        REQUIRE(vec[3] == 30);

        // insert(pos, count, value)
        it = vec.insert(vec.end(), 2, 99); // {10, 15, 20, 30, 99, 99} // auto を使用
        REQUIRE(*it == 99);
        REQUIRE(vec.size() == 6);
        REQUIRE(vec[4] == 99);
        REQUIRE(vec[5] == 99);

        // insert(pos, first, last)
        int arr[] = {1, 2};
        it = vec.insert(vec.begin(), arr, arr + 2); // {1, 2, 10, 15, 20, 30, 99, 99} // auto を使用
        REQUIRE(*it == 1);
        REQUIRE(vec.size() == 8);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
        REQUIRE(vec[2] == 10);
    }

     SECTION("直接構築挿入 (emplace)") {
        TestStruct::reset_counts();
        TestAllocator<TestStruct> struct_allocator("emplace_alloc");
        bluestl::vector<TestStruct, TestAllocator<TestStruct>> svec(struct_allocator);
        svec.assign({ {1, "one"}, {3, "three"} });
        REQUIRE(TestStruct::construction_count > 0); // assignでの構築
        TestStruct::reset_counts();

        auto it = svec.emplace(svec.begin() + 1, 2, "two"); // {one, two, three} // auto を使用

        REQUIRE(it->id == 2);
        REQUIRE(it->name == "two");
        REQUIRE(svec.size() == 3);
        REQUIRE(svec[0].id == 1);
        REQUIRE(svec[1].id == 2);
        REQUIRE(svec[2].id == 3);
        // emplace は直接構築 + 後ろの要素のムーブが発生するはず
        REQUIRE(TestStruct::construction_count >= 1); // emplaceでの直接構築 + ムーブ構築
        REQUIRE(TestStruct::move_construction_count + TestStruct::move_assignment_count > 0); // 要素移動
        REQUIRE(TestStruct::copy_construction_count == 0);
        REQUIRE(TestStruct::copy_assignment_count == 0);
    }


    SECTION("削除 (erase)") {
        vec.assign({1, 2, 3, 4, 5});

        // erase(pos)
        auto it = vec.erase(vec.begin() + 1); // {1, 3, 4, 5} // auto を使用
        REQUIRE(*it == 3);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 3);
        REQUIRE(vec[2] == 4);
        REQUIRE(vec[3] == 5);

        // erase(first, last)
        it = vec.erase(vec.begin() + 1, vec.begin() + 3); // {1, 5} // auto を使用
        REQUIRE(*it == 5);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 5);

        // 末尾を削除
        it = vec.erase(vec.end() - 1); // {1} // auto を使用
        REQUIRE(it == vec.end());
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == 1);

         // 全て削除
        it = vec.erase(vec.begin(), vec.end()); // auto を使用
        REQUIRE(it == vec.end());
        REQUIRE(vec.empty());
    }

    SECTION("比較演算子") {
        bluestl::vector<int, TestAllocator<int>> vec1(allocator);
        bluestl::vector<int, TestAllocator<int>> vec2(allocator);
        bluestl::vector<int, TestAllocator<int>> vec3(allocator);

        vec1.assign({1, 2, 3});
        vec2.assign({1, 2, 3});
        vec3.assign({1, 2, 4});

        REQUIRE(vec1 == vec2);
        REQUIRE_FALSE(vec1 == vec3);
        REQUIRE(vec1 != vec3);

        REQUIRE((vec1 <=> vec2) == std::strong_ordering::equal);
        REQUIRE((vec1 <=> vec3) == std::strong_ordering::less);
        REQUIRE((vec3 <=> vec1) == std::strong_ordering::greater);

        vec2.push_back(4); // {1, 2, 3, 4}
        REQUIRE(vec1 != vec2);
        REQUIRE((vec1 <=> vec2) == std::strong_ordering::less); // 長さが違う
    }

    SECTION("コンストラクタ") {
        // デフォルトコンストラクタ
        bluestl::vector<int, TestAllocator<int>> v_def(allocator);
        REQUIRE(v_def.empty());

        // アロケータのみ
        bluestl::vector<int, TestAllocator<int>> v_alloc(allocator);
        REQUIRE(v_alloc.empty());

        // count
        bluestl::vector<int, TestAllocator<int>> v_count(5, allocator);
        REQUIRE(v_count.size() == 5);
        REQUIRE(v_count[0] == 0);

        // count + value
        bluestl::vector<int, TestAllocator<int>> v_count_val(3, 99, allocator);
        REQUIRE(v_count_val.size() == 3);
        REQUIRE(v_count_val[0] == 99);
        REQUIRE(v_count_val[1] == 99);
        REQUIRE(v_count_val[2] == 99);

        // iterator range
        int arr[] = {5, 6, 7};
        bluestl::vector<int, TestAllocator<int>> v_range(arr, arr + 3, allocator);
        REQUIRE(v_range.size() == 3);
        REQUIRE(v_range[0] == 5);
        REQUIRE(v_range[2] == 7);

        // copy constructor (アロケータはselectされる)
        TestAllocator<int> alloc_copy("copy_alloc");
        bluestl::vector<int, TestAllocator<int>> v_copy_src(3, 1, alloc_copy);
        bluestl::vector<int, TestAllocator<int>> v_copy_dst(v_copy_src);
        REQUIRE(v_copy_dst.size() == 3);
        REQUIRE(v_copy_dst[0] == 1);
        // select_on_container_copy_construction のデフォルトはコピー元を返す
        REQUIRE(v_copy_dst.get_allocator() == v_copy_src.get_allocator());

        // move constructor (アロケータもムーブされる)
        bluestl::vector<int, TestAllocator<int>> v_move_src(2, 2, allocator);
        size_t src_cap = v_move_src.capacity();
        auto src_data = v_move_src.data(); // pointer -> auto に修正
        bluestl::vector<int, TestAllocator<int>> v_move_dst(std::move(v_move_src));
        REQUIRE(v_move_dst.size() == 2);
        REQUIRE(v_move_dst[0] == 2);
        REQUIRE(v_move_dst.capacity() == src_cap);
        REQUIRE(v_move_dst.data() == src_data);
        REQUIRE(v_move_dst.get_allocator() == allocator); // ムーブされたアロケータ
        // ムーブ元は空になる
        REQUIRE(v_move_src.empty());
        REQUIRE(v_move_src.capacity() == 0);
        REQUIRE(v_move_src.data() == nullptr);

        // initializer_list
        bluestl::vector<int, TestAllocator<int>> v_init({8, 9}, allocator);
        REQUIRE(v_init.size() == 2);
        REQUIRE(v_init[0] == 8);
        REQUIRE(v_init[1] == 9);
    }

     SECTION("代入演算子") {
        TestAllocator<int> alloc1("assign_alloc1");
        TestAllocator<int> alloc2("assign_alloc2");
        bluestl::vector<int, TestAllocator<int>> v1({1, 2}, alloc1);
        bluestl::vector<int, TestAllocator<int>> v2({3, 4, 5}, alloc2);

        // コピー代入 (propagate = true なのでアロケータもコピーされる)
        v1 = v2;
        REQUIRE(v1.size() == 3);
        REQUIRE(v1[0] == 3);
        REQUIRE(v1.get_allocator() == alloc2); // アロケータがコピーされた

        // ムーブ代入 (propagate = true なのでアロケータもムーブされる)
        bluestl::vector<int, TestAllocator<int>> v3({6}, alloc1);
        auto v2_data = v2.data(); // pointer -> auto
        size_t v2_cap = v2.capacity();
        v3 = std::move(v2);
        REQUIRE(v3.size() == 3);
        REQUIRE(v3[0] == 3);
        REQUIRE(v3.get_allocator() == alloc2); // アロケータがムーブされた
        REQUIRE(v3.data() == v2_data);
        REQUIRE(v3.capacity() == v2_cap);
        REQUIRE(v2.empty()); // ムーブ元は空
        REQUIRE(v2.data() == nullptr);

        // initializer_list 代入
        v1 = {7, 8, 9, 10};
        REQUIRE(v1.size() == 4);
        REQUIRE(v1[0] == 7);
        REQUIRE(v1[3] == 10);
    }

}

// アロケータ伝播フラグがfalseの場合のテスト
struct NoPropagateAllocatorTag {}; // タグ

template <typename T>
class TestAllocatorNoPropagate : public TestAllocator<T> {
public:
     using propagate_on_container_copy_assignment = std::false_type;
     using propagate_on_container_move_assignment = std::false_type;
     using propagate_on_container_swap = std::false_type;
     // is_always_equal は false のまま

     template <typename U>
     struct rebind {
         using other = TestAllocatorNoPropagate<U>;
     };

     explicit TestAllocatorNoPropagate(const char* name = "TestAllocatorNoPropagate") noexcept
        : TestAllocator<T>(name) {}

     // 他のコンストラクタ、代入演算子も基底クラスを呼び出すように実装が必要だが、テスト目的なので省略
};


TEST_CASE("vector アロケータ伝播なし", "[vector][allocator]") {
    TestAllocatorNoPropagate<int> tmp_alloc1("no_prop_alloc1");
    TestAllocatorNoPropagate<int> tmp_alloc2("no_prop_alloc2tmp_");

    bluestl::vector<int, TestAllocatorNoPropagate<int>> vec1({1, 2}, tmp_alloc1);
    bluestl::vector<int, TestAllocatorNoPropagate<int>> vec2({3, 4, 5}, tmp_alloc2);

    const auto& alloc1 = vec1.get_allocator_ref();
    const auto& alloc2 = vec1.get_allocator_ref();

    SECTION("コピー代入 (propagate = false)") {
        vec1 = vec2; // 要素はコピーされるが、アロケータはそのまま
        REQUIRE(vec1.size() == 3);
        REQUIRE(vec1[0] == 3);
        REQUIRE(vec1.get_allocator() == alloc1); // アロケータは変わらない
        // alloc1 で再確保が発生するはず
        REQUIRE(alloc1.get_allocation_count() > 1); // 初期確保 + 再確保
    }

    SECTION("ムーブ代入 (propagate = false)") {
        bluestl::vector<int, TestAllocatorNoPropagate<int>> vec3({6}, alloc1);

        // アロケータが異なる場合、要素ごとのムーブが発生
        vec3 = std::move(vec2);
        REQUIRE(vec3.size() == 3);
        REQUIRE(vec3[0] == 3);
        REQUIRE(vec3.get_allocator_ref() == alloc1); // アロケータは変わらない
        REQUIRE(vec2.empty()); // ムーブ元は常に空になる
        // alloc1 で再確保が発生するはず
        REQUIRE(alloc1.get_allocation_count() > 1);

        // アロケータが同じ場合、リソースをスチール
        bluestl::vector<int, TestAllocatorNoPropagate<int>> vec4({7, 8}, alloc1);
        auto vec4_data = vec4.data(); // pointer -> auto
        size_t vec4_cap = vec4.capacity();
        vec3 = std::move(vec4);
        REQUIRE(vec3.size() == 2);
        REQUIRE(vec3[0] == 7);
        REQUIRE(vec3.get_allocator() == alloc1);
        REQUIRE(vec3.data() == vec4_data); // リソースが移動
        REQUIRE(vec3.capacity() == vec4_cap);
        REQUIRE(vec4.empty());
        REQUIRE(vec4.data() == nullptr);
    }

     SECTION("swap (propagate = false)") {
         // アロケータが異なる場合は未定義動作 (Bluestlではアサート)
         // REQUIRE_THROWS(vec1.swap(vec2)); // アサートなのでテスト不可

         // アロケータが同じ場合
         bluestl::vector<int, TestAllocatorNoPropagate<int>> vec3({7, 8}, alloc1);
         auto vec1_data = vec1.data(); size_t vec1_cap = vec1.capacity(); size_t vec1_size = vec1.size(); // pointer -> auto
         auto vec3_data = vec3.data(); size_t vec3_cap = vec3.capacity(); size_t vec3_size = vec3.size(); // pointer -> auto

         vec1.swap(vec3);

         REQUIRE(vec1.get_allocator() == alloc1);
         REQUIRE(vec3.get_allocator() == alloc1);
         REQUIRE(vec1.data() == vec3_data); REQUIRE(vec1.capacity() == vec3_cap); REQUIRE(vec1.size() == vec3_size);
         REQUIRE(vec3.data() == vec1_data); REQUIRE(vec3.capacity() == vec1_cap); REQUIRE(vec3.size() == vec1_size);
     }
}
