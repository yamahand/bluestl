#pragma once

#include "log_macros.h"

namespace bluestl {
// 型を持たないアロケータ
class allocator {
   public:
    using value_type = void;  // 型パラメータなし

    allocator(const char* name = "bluestl default") : m_name(name) {}

    // メモリ確保
    virtual void* allocate(size_t n) = 0;

    // メモリ解放
    virtual void deallocate(void* p, size_t) = 0;

    // アロケータ名を取得
    // デバッグ目的で名前を保持することができる
    const char* get_name() const {
        return m_name;
    }

   private:
    const char* m_name;  // 任意で名前を保持（デバッグ目的）
};
}  // namespace bluestl