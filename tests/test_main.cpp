#include <catch2/catch_test_macros.hpp>
#include <iostream>

// ハッシュ関数テスト
void test_hash_functions();
// hash_map, fixed_hash_mapテスト
void test_fixed_hash_map();
void test_hash_map();
// fixed_vector, vector, fixed_vector_ext テスト
void test_fixed_vector();
void test_vector();
void test_fixed_vector_ext();

int main() {
    std::cout << "test_hash_functions..." << std::endl;
    test_hash_functions();
    std::cout << "test_fixed_hash_map..." << std::endl;
    test_fixed_hash_map();
    std::cout << "test_hash_map..." << std::endl;
    test_hash_map();
    std::cout << "test_fixed_vector..." << std::endl;
    test_fixed_vector();
    std::cout << "test_vector..." << std::endl;
    test_vector();
    std::cout << "test_fixed_vector_ext..." << std::endl;
    test_fixed_vector_ext();
    std::cout << "すべてのテストが正常に完了しました。" << std::endl;
    return 0;
}
