/**
 * @file array.h
 * @brief 固定サイズ配列コンテナの実装
 * @author BlueStl
 * @date 2025
 *
 * このファイルは、BlueStlライブラリのarrayクラスの実装を提供します。
 * コンパイル時に決定される固定サイズの配列コンテナで、
 * STL std::arrayと互換性のあるインターフェースを提供します。
 *
 * 特徴:
 * - コンパイル時固定サイズ
 * - ヒープ割り当てなし（スタックのみ）
 * - 境界チェック機能
 * - イテレータサポート
 * - 集約初期化対応
 * - C++20準拠
 * - 例外なし・RTTIなし
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <utility>
#include <compare>
#include "assert_handler.h"
#include "log_macros.h"

namespace bluestl {

/**
 * @class array
 * @brief 固定サイズ配列コンテナ
 * @tparam T 要素型
 * @tparam N 要素数
 *
 * arrayは固定サイズの配列コンテナです。
 * STL std::array風のインターフェースを提供し、安全な配列操作を実現します。
 * コンパイル時にサイズが決定され、スタック上に配置されます。
 *
 * 使用例:
 * @code
 * bluestl::array<int, 5> arr = {1, 2, 3, 4, 5};
 * for (auto& elem : arr) {
 *     // 安全な範囲アクセス
 * }
 * @endcode
 */
template <typename T, std::size_t N>
class array {
   public:
    // 型エイリアス
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // データメンバ
    T data_[N > 0 ? N : 1];  // 空の配列を避けるため、0の場合は1にする

    /**
     * @brief 要素へのアクセス（境界チェックあり）
     * @param pos インデックス
     * @return 要素への参照
     */
    constexpr reference at(size_type pos) noexcept {
        BLUESTL_ASSERT(pos < N);
        return data_[pos];
    }

    /**
     * @brief 要素への読み取り専用アクセス（境界チェックあり）
     * @param pos インデックス
     * @return 要素への const 参照
     */
    constexpr const_reference at(size_type pos) const noexcept {
        BLUESTL_ASSERT(pos < N);
        return data_[pos];
    }

    /**
     * @brief 要素へのアクセス（境界チェックなし）
     * @param pos インデックス
     * @return 要素への参照
     */
    constexpr reference operator[](size_type pos) noexcept {
        return data_[pos];
    }

    /**
     * @brief 要素への読み取り専用アクセス（境界チェックなし）
     * @param pos インデックス
     * @return 要素への const 参照
     */
    constexpr const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    /**
     * @brief 先頭要素への参照を取得
     * @return 先頭要素への参照
     */
    constexpr reference front() noexcept {
        static_assert(N > 0, "array::front() called on empty array");
        return data_[0];
    }

    /**
     * @brief 先頭要素への読み取り専用参照を取得
     * @return 先頭要素への const 参照
     */
    constexpr const_reference front() const noexcept {
        static_assert(N > 0, "array::front() called on empty array");
        return data_[0];
    }

    /**
     * @brief 末尾要素への参照を取得
     * @return 末尾要素への参照
     */
    constexpr reference back() noexcept {
        static_assert(N > 0, "array::back() called on empty array");
        return data_[N - 1];
    }

    /**
     * @brief 末尾要素への読み取り専用参照を取得
     * @return 末尾要素への const 参照
     */
    constexpr const_reference back() const noexcept {
        static_assert(N > 0, "array::back() called on empty array");
        return data_[N - 1];
    }

    /**
     * @brief 内部データ配列へのポインタを取得
     * @return データ配列へのポインタ
     */
    constexpr T* data() noexcept {
        return data_;
    }

    /**
     * @brief 内部データ配列への読み取り専用ポインタを取得
     * @return データ配列への const ポインタ
     */
    constexpr const T* data() const noexcept {
        return data_;
    }

    /**
     * @brief 先頭要素へのイテレータを取得
     * @return 先頭要素へのイテレータ
     */
    constexpr iterator begin() noexcept {
        return data_;
    }

    /**
     * @brief 先頭要素への読み取り専用イテレータを取得
     * @return 先頭要素への const イテレータ
     */
    constexpr const_iterator begin() const noexcept {
        return data_;
    }

    /**
     * @brief 先頭要素への読み取り専用イテレータを取得
     * @return 先頭要素への const イテレータ
     */
    constexpr const_iterator cbegin() const noexcept {
        return data_;
    }

    /**
     * @brief 末尾の次の要素へのイテレータを取得
     * @return 末尾の次の要素へのイテレータ
     */
    constexpr iterator end() noexcept {
        return data_ + N;
    }

    /**
     * @brief 末尾の次の要素への読み取り専用イテレータを取得
     * @return 末尾の次の要素への const イテレータ
     */
    constexpr const_iterator end() const noexcept {
        return data_ + N;
    }

    /**
     * @brief 末尾の次の要素への読み取り専用イテレータを取得
     * @return 末尾の次の要素への const イテレータ
     */
    constexpr const_iterator cend() const noexcept {
        return data_ + N;
    }

    /**
     * @brief 末尾要素への逆方向イテレータを取得
     * @return 末尾要素への逆方向イテレータ
     */
    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    /**
     * @brief 末尾要素への読み取り専用逆方向イテレータを取得
     * @return 末尾要素への const 逆方向イテレータ
     */
    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 末尾要素への読み取り専用逆方向イテレータを取得
     * @return 末尾要素への const 逆方向イテレータ
     */
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 先頭の前の要素への逆方向イテレータを取得
     * @return 先頭の前の要素への逆方向イテレータ
     */
    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    /**
     * @brief 先頭の前の要素への読み取り専用逆方向イテレータを取得
     * @return 先頭の前の要素への const 逆方向イテレータ
     */
    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief 先頭の前の要素への読み取り専用逆方向イテレータを取得
     * @return 先頭の前の要素への const 逆方向イテレータ
     */
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief 配列が空かどうかを判定
     * @return 空の場合 true、それ以外は false
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return N == 0;
    }

    /**
     * @brief 配列のサイズを取得
     * @return 要素数
     */
    [[nodiscard]] constexpr size_type size() const noexcept {
        return N;
    }

    /**
     * @brief 配列の最大サイズを取得
     * @return 最大要素数（sizeと同じ）
     */
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return N;
    }

    /**
     * @brief 配列の全要素を指定した値で埋める
     * @param value 埋める値
     */
    constexpr void fill(const T& value) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        for (size_type i = 0; i < N; ++i) {
            data_[i] = value;
        }
    }

    /**
     * @brief 他の配列と要素を交換
     * @param other 交換する配列
     */
    constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<T>) {
        for (size_type i = 0; i < N; ++i) {
            using std::swap;
            swap(data_[i], other.data_[i]);
        }
    }
};

// 0サイズ配列の特殊化
template <typename T>
class array<T, 0> {
   public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // アクセサ関数（使用されるべきではない）
    constexpr reference at(size_type) noexcept {
        // 0サイズ配列ではアクセスできない
        return *static_cast<T*>(nullptr);  // 到達不可能
    }

    constexpr const_reference at(size_type) const noexcept {
        // 0サイズ配列ではアクセスできない
        return *static_cast<const T*>(nullptr);  // 到達不可能
    }

    constexpr reference operator[](size_type) noexcept {
        return *static_cast<T*>(nullptr);  // 未定義動作だが、0サイズなので呼ばれるべきではない
    }

    constexpr const_reference operator[](size_type) const noexcept {
        return *static_cast<const T*>(nullptr);  // 未定義動作だが、0サイズなので呼ばれるべきではない
    }

    constexpr reference front() noexcept {
        static_assert(false, "array<T, 0>::front() called on empty array");
        return *static_cast<T*>(nullptr);  // 到達不可能
    }

    constexpr const_reference front() const noexcept {
        static_assert(false, "array<T, 0>::front() called on empty array");
        return *static_cast<const T*>(nullptr);  // 到達不可能
    }

    constexpr reference back() noexcept {
        static_assert(false, "array<T, 0>::back() called on empty array");
        return *static_cast<T*>(nullptr);  // 到達不可能
    }

    constexpr const_reference back() const noexcept {
        static_assert(false, "array<T, 0>::back() called on empty array");
        return *static_cast<const T*>(nullptr);  // 到達不可能
    }

    // データアクセス
    constexpr T* data() noexcept {
        return nullptr;
    }
    constexpr const T* data() const noexcept {
        return nullptr;
    }

    // イテレータ
    constexpr iterator begin() noexcept {
        return nullptr;
    }
    constexpr const_iterator begin() const noexcept {
        return nullptr;
    }
    constexpr const_iterator cbegin() const noexcept {
        return nullptr;
    }
    constexpr iterator end() noexcept {
        return nullptr;
    }
    constexpr const_iterator end() const noexcept {
        return nullptr;
    }
    constexpr const_iterator cend() const noexcept {
        return nullptr;
    }
    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    // サイズ関連
    [[nodiscard]] constexpr bool empty() const noexcept {
        return true;
    }
    [[nodiscard]] constexpr size_type size() const noexcept {
        return 0;
    }
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return 0;
    }

    // 操作
    constexpr void fill(const T&) noexcept {}
    constexpr void swap(array&) noexcept {}
};

/**
 * @brief 配列同士の等価比較
 * @param lhs 左辺の配列
 * @param rhs 右辺の配列
 * @return 全要素が等しい場合 true
 */
template <typename T, std::size_t N>
constexpr bool operator==(const array<T, N>& lhs, const array<T, N>& rhs) noexcept {
    if constexpr (N == 0) {
        return true;
    } else {
        for (std::size_t i = 0; i < N; ++i) {
            if (!(lhs[i] == rhs[i])) {
                return false;
            }
        }
        return true;
    }
}

/**
 * @brief 配列同士の三方比較
 * @param lhs 左辺の配列
 * @param rhs 右辺の配列
 * @return 比較結果
 */
template <typename T, std::size_t N>
    requires std::three_way_comparable<T>
constexpr auto operator<=>(const array<T, N>& lhs, const array<T, N>& rhs) noexcept {
    if constexpr (N == 0) {
        return std::strong_ordering::equal;
    } else {
        for (std::size_t i = 0; i < N; ++i) {
            if (auto cmp = lhs[i] <=> rhs[i]; cmp != 0) {
                return cmp;
            }
        }
        return std::strong_ordering::equal;
    }
}

/**
 * @brief 配列の要素を取得（タプル風インターフェース）
 * @tparam I インデックス
 * @param arr 配列
 * @return I番目の要素への参照
 */
template <std::size_t I, typename T, std::size_t N>
constexpr T& get(array<T, N>& arr) noexcept {
    static_assert(I < N, "Index out of bounds");
    return arr[I];
}

/**
 * @brief 配列の要素を取得（タプル風インターフェース、const版）
 * @tparam I インデックス
 * @param arr 配列
 * @return I番目の要素への const 参照
 */
template <std::size_t I, typename T, std::size_t N>
constexpr const T& get(const array<T, N>& arr) noexcept {
    static_assert(I < N, "Index out of bounds");
    return arr[I];
}

/**
 * @brief 配列の要素を取得（タプル風インターフェース、move版）
 * @tparam I インデックス
 * @param arr 配列
 * @return I番目の要素のmove参照
 */
template <std::size_t I, typename T, std::size_t N>
constexpr T&& get(array<T, N>&& arr) noexcept {
    static_assert(I < N, "Index out of bounds");
    return std::move(arr[I]);
}

/**
 * @brief 配列同士の要素交換
 * @param lhs 左辺の配列
 * @param rhs 右辺の配列
 */
template <typename T, std::size_t N>
constexpr void swap(array<T, N>& lhs, array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

/**
 * @brief 配列からの推論ガイド
 */
template <typename T, typename... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;

}  // namespace bluestl

// std::tuple_size と std::tuple_element のサポート
namespace std {

template <typename T, std::size_t N>
struct tuple_size<bluestl::array<T, N>> : integral_constant<std::size_t, N> {};

template <std::size_t I, typename T, std::size_t N>
struct tuple_element<I, bluestl::array<T, N>> {
    static_assert(I < N, "Index out of bounds");
    using type = T;
};

}  // namespace std
