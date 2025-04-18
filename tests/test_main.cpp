#include <iostream>

void test_fixed_vector();
void test_fixed_vector_ext();
void test_vector();

int main() {
    std::cout << "fixed_vectorのテスト開始..." << std::endl;
    test_fixed_vector();
    std::cout << "fixed_vector拡張テスト開始..." << std::endl;
    test_fixed_vector_ext();
    std::cout << "vectorのテスト開始..." << std::endl;
    test_vector();
    std::cout << "全テスト完了" << std::endl;
    return 0;
}
