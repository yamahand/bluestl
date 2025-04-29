#pragma once

namespace bluestl {
// 2つの型を圧縮して1つの型にするための基底クラス
template <typename T1, typename T2, bool is_first_empty, bool is_second_empty>
class compressed_pair_impl;  // 前方宣言

// 1つ目の型が空で、2つ目の型が空でない場合
template <typename T1, typename T2>
class compressed_pair_impl<T1, T2, true, false> : private T1 {
   public:
    using first_type = T1;   // 最初の型
    using second_type = T2;  // 2番目の型

    // コンストラクタ
    compressed_pair_impl() : m_second() {}
    compressed_pair_impl(const T1& first, const T2& second) : m_second(second) {}

    // アクセサ
    first_type& first() {
        return *this;
    }
    const first_type& first() const {
        return *this;
    }
    second_type& second() {
        return m_second;
    }
    const second_type& second() const {
        return m_second;
    }

   private:
    T2 m_second;  // 2番目の型のインスタンス
};

// 1つ目の型が空ではなく、2つ目の型が空の場合
template <typename T1, typename T2>
class compressed_pair_impl<T1, T2, false, true> : private T2 {
   public:
    using first_type = T1;   // 最初の型
    using second_type = T2;  // 2番目の型

    // コンストラクタ
    compressed_pair_impl() : m_first() {}
    compressed_pair_impl(const T1& first, const T2& second) : m_first(first) {}

    // アクセサ
    first_type& first() {
        return m_first;
    }
    const first_type& first() const {
        return m_first;
    }
    second_type& second() {
        return *this;
    }
    const second_type& second() const {
        return *this;
    }

   private:
    T1 m_first;  // 最初の型のインスタンス
};

// 1つ目の型と2つ目の型が両方とも空ではない場合
template <typename T1, typename T2>
class compressed_pair_impl<T1, T2, false, false> {
   public:
    using first_type = T1;   // 最初の型
    using second_type = T2;  // 2番目の型

    // コンストラクタ
    compressed_pair_impl() : m_first(), m_second() {}
    compressed_pair_impl(const T1& first, const T2& second) : m_first(first), m_second(second) {}

    // アクセサ
    first_type& first() {
        return m_first;
    }
    const first_type& first() const {
        return m_first;
    }
    second_type& second() {
        return m_second;
    }
    const second_type& second() const {
        return m_second;
    }

   private:
    T1 m_first;   // 最初の型のインスタンス
    T2 m_second;  // 2番目の型のインスタンス
};

// 1つ目の型と2つ目の型が両方とも空の場合
template <typename T1, typename T2>
class compressed_pair_impl<T1, T2, true, true> : private T1, private T2 {
   public:
    using first_type = T1;   // 最初の型
    using second_type = T2;  // 2番目の型

    // コンストラクタ
    compressed_pair_impl() {}
    compressed_pair_impl(const T1& first, const T2& second) {}

    // アクセサ
    first_type& first() {
        return static_cast<T1&>(*this);
    }
    const first_type& first() const {
        return static_cast<const T1&>(*this);
    }
    second_type& second() {
        return static_cast<T2&>(*this);
    }
    const second_type& second() const {
        return static_cast<const T2&>(*this);
    }
};

// 圧縮ペア
// 2つの型を圧縮して1つの型にするためのクラス
template <typename T1, typename T2>
class compressed_pair : public compressed_pair_impl<T1, T2, __is_empty(T1), __is_empty(T2)> {
   private:
    using base_type = compressed_pair_impl<T1, T2, __is_empty(T1), __is_empty(T2)>;  // 基底クラス
   public:
    using first_type = T1;   // 最初の型
    using second_type = T2;  // 2番目の型

    // コンストラクタ
    compressed_pair() : base_type() {}
    compressed_pair(const T1& first, const T2& second) : base_type(first, second) {}

    // アクセサ
    first_type& first() {
        return base_type::first();
    }
    const first_type& first() const {
        return base_type::first();
    }
    second_type& second() {
        return base_type::second();
    }
    const second_type& second() const {
        return base_type::second();
    }
};
}  // namespace bluestl