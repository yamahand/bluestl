/**
 * @file span.h
 * @brief 配列やバッファの一部を安全に参照するための軽量ビュー
 * @author BlueStl
 * @date 2025
 *
 * このファイルは、BlueStlライブラリのspanクラス（array_view）の実装を提供します。
 * 配列やコンテナの連続したメモリ領域への非所有参照を提供し、
 * 境界チェック付きアクセスや範囲ベース操作を安全に行えます。
 *
 * 特徴:
 * - ヒープ割り当てなし
 * - 軽量で高速
 * - 境界チェック機能
 * - イテレータサポート
 * - C++20準拠
 * - 例外なし・RTTIなし
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <utility>
#include "assert_handler.h"
#include "log_macros.h"

namespace bluestl {

/**
 * @brief 動的範囲を表す定数
 */
inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

/**
 * @class span
 * @brief 配列やバッファの一部を安全に参照するための軽量ビュー
 * @tparam T 要素型
 * @tparam Extent 静的サイズ（dynamic_extentの場合は動的サイズ）
 *
 * spanは連続したメモリ領域への非所有参照を提供します。
 * STL std::span風のインターフェースを提供し、安全な配列アクセスを実現します。
 *
 * 使用例:
 * @code
 * int arr[] = {1, 2, 3, 4, 5};
 * bluestl::span<int> s(arr, 5);
 * for (auto& elem : s) {
 *     // 安全な範囲アクセス
 * }
 * @endcode
 */
template <typename T, std::size_t Extent = dynamic_extent>
class span {
   public:
    // --- 型定義 ---
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // --- 定数 ---
    static constexpr size_type extent = Extent;

   private:
    pointer data_;
    [[no_unique_address]]
    std::conditional_t<Extent != dynamic_extent, std::integral_constant<size_type, Extent>, size_type> size_;

   public:
    // --- コンストラクタ ---

    /**
     * @brief デフォルトコンストラクタ（Extent == 0の場合のみ有効）
     */
    constexpr span() noexcept
        requires(Extent == 0 || Extent == dynamic_extent)
        : data_(nullptr) {
        if constexpr (Extent == dynamic_extent) {
            size_ = 0;
        }
    }

    /**
     * @brief ポインタと要素数からspanを構築
     * @param ptr データポインタ
     * @param count 要素数
     */
    constexpr span(pointer ptr, size_type count) noexcept : data_(ptr) {
        if constexpr (Extent == dynamic_extent) {
            size_ = count;
        } else {
            BLUESTL_ASSERT_MSG(count == Extent, "Size mismatch for fixed extent span");
        }
    }

    /**
     * @brief 開始・終了ポインタからspanを構築
     * @param first 開始ポインタ
     * @param last 終了ポインタ
     */
    constexpr span(pointer first, pointer last) noexcept : data_(first) {
        const auto computed_size = static_cast<size_type>(last - first);
        if constexpr (Extent == dynamic_extent) {
            size_ = computed_size;
        } else {
            BLUESTL_ASSERT_MSG(computed_size == Extent, "Size mismatch for fixed extent span");
        }
    }

    /**
     * @brief 配列からspanを構築
     * @tparam N 配列サイズ
     * @param arr 配列参照
     */
    template <std::size_t N>
        requires(Extent == dynamic_extent || Extent == N)
    constexpr span(element_type (&arr)[N]) noexcept : data_(arr) {
        if constexpr (Extent == dynamic_extent) {
            size_ = N;
        }
    }

    /**
     * @brief 他のコンテナからspanを構築
     * @tparam Container コンテナ型
     * @param container コンテナ参照
     */
    template <typename Container>
        requires(!std::is_array_v<Container> && !std::is_same_v<std::remove_cvref_t<Container>, span> &&
                 requires(Container& c) {
                     c.data();
                     c.size();
                     { c.data() } -> std::convertible_to<pointer>;
                     { c.size() } -> std::convertible_to<size_type>;
                 })
    constexpr span(Container& container) noexcept : data_(container.data()) {
        if constexpr (Extent == dynamic_extent) {
            size_ = container.size();
        } else {
            BLUESTL_ASSERT_MSG(container.size() == Extent, "Container size mismatch for fixed extent span");
        }
    }

    /**
     * @brief constコンテナからspanを構築
     * @tparam Container コンテナ型
     * @param container コンテナ参照
     */
    template <typename Container>
        requires(!std::is_array_v<Container> && !std::is_same_v<std::remove_cvref_t<Container>, span> &&
                 std::is_const_v<element_type> &&
                 requires(const Container& c) {
                     c.data();
                     c.size();
                     { c.data() } -> std::convertible_to<pointer>;
                     { c.size() } -> std::convertible_to<size_type>;
                 })
    constexpr span(const Container& container) noexcept : data_(container.data()) {
        if constexpr (Extent == dynamic_extent) {
            size_ = container.size();
        } else {
            BLUESTL_ASSERT_MSG(container.size() == Extent, "Container size mismatch for fixed extent span");
        }
    }

    /**
     * @brief 別のspanからの変換コンストラクタ
     * @tparam U 元の要素型
     * @tparam OtherExtent 元のExtent
     * @param other 変換元span
     */
    template <typename U, std::size_t OtherExtent>
        requires(std::convertible_to<U (*)[], element_type (*)[]> &&
                 (Extent == dynamic_extent || OtherExtent == dynamic_extent || Extent == OtherExtent))
    constexpr span(const span<U, OtherExtent>& other) noexcept : data_(other.data()) {
        if constexpr (Extent == dynamic_extent) {
            size_ = other.size();
        } else if constexpr (OtherExtent != dynamic_extent) {
            static_assert(Extent == OtherExtent, "Fixed extent mismatch");
        } else {
            BLUESTL_ASSERT_MSG(other.size() == Extent, "Dynamic span size mismatch with fixed extent");
        }
    }

    // --- デストラクタ ---
    ~span() = default;

    // --- 代入演算子 ---
    constexpr span& operator=(const span& other) noexcept = default;

    // --- 要素アクセス ---

    /**
     * @brief 指定インデックスの要素にアクセス（境界チェックなし）
     * @param idx インデックス
     * @return 要素への参照
     */
    constexpr reference operator[](size_type idx) const noexcept {
        BLUESTL_ASSERT_MSG(idx < size(), "Index out of bounds");
        return data_[idx];
    }

    /**
     * @brief 指定インデックスの要素にアクセス（境界チェック付き）
     * @param idx インデックス
     * @return 要素への参照、範囲外の場合は未定義動作
     */
    constexpr reference at(size_type idx) const noexcept {
        BLUESTL_ASSERT_MSG(idx < size(), "Index out of bounds in at()");
        return data_[idx];
    }

    /**
     * @brief 最初の要素への参照を取得
     * @return 最初の要素への参照
     */
    constexpr reference front() const noexcept {
        BLUESTL_ASSERT_MSG(!empty(), "front() called on empty span");
        return data_[0];
    }

    /**
     * @brief 最後の要素への参照を取得
     * @return 最後の要素への参照
     */
    constexpr reference back() const noexcept {
        BLUESTL_ASSERT_MSG(!empty(), "back() called on empty span");
        return data_[size() - 1];
    }

    /**
     * @brief データポインタを取得
     * @return データポインタ
     */
    constexpr pointer data() const noexcept {
        return data_;
    }

    // --- イテレータ ---

    /**
     * @brief 開始イテレータを取得
     * @return 開始イテレータ
     */
    constexpr iterator begin() const noexcept {
        return data_;
    }

    /**
     * @brief 終了イテレータを取得
     * @return 終了イテレータ
     */
    constexpr iterator end() const noexcept {
        return data_ + size();
    }

    /**
     * @brief const開始イテレータを取得
     * @return const開始イテレータ
     */
    constexpr const_iterator cbegin() const noexcept {
        return data_;
    }

    /**
     * @brief const終了イテレータを取得
     * @return const終了イテレータ
     */
    constexpr const_iterator cend() const noexcept {
        return data_ + size();
    }

    /**
     * @brief 逆方向開始イテレータを取得
     * @return 逆方向開始イテレータ
     */
    constexpr reverse_iterator rbegin() const noexcept {
        return reverse_iterator(end());
    }

    /**
     * @brief 逆方向終了イテレータを取得
     * @return 逆方向終了イテレータ
     */
    constexpr reverse_iterator rend() const noexcept {
        return reverse_iterator(begin());
    }

    /**
     * @brief const逆方向開始イテレータを取得
     * @return const逆方向開始イテレータ
     */
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    /**
     * @brief const逆方向終了イテレータを取得
     * @return const逆方向終了イテレータ
     */
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    // --- サイズ情報 ---

    /**
     * @brief 要素数を取得
     * @return 要素数
     */
    constexpr size_type size() const noexcept {
        if constexpr (Extent != dynamic_extent) {
            return Extent;
        } else {
            return size_;
        }
    }

    /**
     * @brief バイト単位のサイズを取得
     * @return バイト単位のサイズ
     */
    constexpr size_type size_bytes() const noexcept {
        return size() * sizeof(element_type);
    }

    /**
     * @brief 空かどうかを判定
     * @return 空の場合true
     */
    constexpr bool empty() const noexcept {
        return size() == 0;
    }

    // --- サブスパン操作 ---

    /**
     * @brief 先頭から指定数の要素のサブスパンを取得
     * @tparam Count 要素数
     * @return サブスパン
     */
    template <std::size_t Count>
    constexpr span<element_type, Count> first() const noexcept {
        static_assert(Count <= Extent || Extent == dynamic_extent, "Count exceeds span extent");
        BLUESTL_ASSERT_MSG(Count <= size(), "Count exceeds span size");
        return span<element_type, Count>(data_, Count);
    }

    /**
     * @brief 先頭から指定数の要素のサブスパンを取得（動的版）
     * @param count 要素数
     * @return サブスパン
     */
    constexpr span<element_type> first(size_type count) const noexcept {
        BLUESTL_ASSERT_MSG(count <= size(), "Count exceeds span size");
        return span<element_type>(data_, count);
    }

    /**
     * @brief 末尾から指定数の要素のサブスパンを取得
     * @tparam Count 要素数
     * @return サブスパン
     */
    template <std::size_t Count>
    constexpr span<element_type, Count> last() const noexcept {
        static_assert(Count <= Extent || Extent == dynamic_extent, "Count exceeds span extent");
        BLUESTL_ASSERT_MSG(Count <= size(), "Count exceeds span size");
        return span<element_type, Count>(data_ + (size() - Count), Count);
    }

    /**
     * @brief 末尾から指定数の要素のサブスパンを取得（動的版）
     * @param count 要素数
     * @return サブスパン
     */
    constexpr span<element_type> last(size_type count) const noexcept {
        BLUESTL_ASSERT_MSG(count <= size(), "Count exceeds span size");
        return span<element_type>(data_ + (size() - count), count);
    }

    /**
     * @brief 指定オフセットからのサブスパンを取得
     * @tparam Offset 開始オフセット
     * @tparam Count 要素数（dynamic_extentで残り全部）
     * @return サブスパン
     */
    template <std::size_t Offset, std::size_t Count = dynamic_extent>
    constexpr auto subspan() const noexcept {
        static_assert(Offset <= Extent || Extent == dynamic_extent, "Offset exceeds span extent");
        BLUESTL_ASSERT_MSG(Offset <= size(), "Offset exceeds span size");

        if constexpr (Count == dynamic_extent) {
            constexpr auto new_extent = (Extent == dynamic_extent) ? dynamic_extent : Extent - Offset;
            return span<element_type, new_extent>(data_ + Offset, size() - Offset);
        } else {
            static_assert(Offset + Count <= Extent || Extent == dynamic_extent, "Offset + Count exceeds span extent");
            BLUESTL_ASSERT_MSG(Offset + Count <= size(), "Offset + Count exceeds span size");
            return span<element_type, Count>(data_ + Offset, Count);
        }
    }

    /**
     * @brief 指定オフセットからのサブスパンを取得（動的版）
     * @param offset 開始オフセット
     * @param count 要素数（dynamic_extentで残り全部）
     * @return サブスパン
     */
    constexpr span<element_type> subspan(size_type offset, size_type count = dynamic_extent) const noexcept {
        BLUESTL_ASSERT_MSG(offset <= size(), "Offset exceeds span size");
        if (count == dynamic_extent) {
            return span<element_type>(data_ + offset, size() - offset);
        } else {
            BLUESTL_ASSERT_MSG(offset + count <= size(), "Offset + Count exceeds span size");
            return span<element_type>(data_ + offset, count);
        }
    }
};

// --- 推論ガイド ---

template <typename It, typename EndOrSize>
span(It, EndOrSize) -> span<std::remove_reference_t<std::iter_reference_t<It>>>;

template <typename T, std::size_t N>
span(T (&)[N]) -> span<T, N>;

template <typename Container>
span(Container&) -> span<typename Container::value_type>;

template <typename Container>
span(const Container&) -> span<const typename Container::value_type>;

// --- 比較演算子 ---

/**
 * @brief span同士の等価比較
 * @tparam T1, T2 要素型
 * @tparam E1, E2 Extent値
 * @param lhs 左辺span
 * @param rhs 右辺span
 * @return 等価の場合true
 */
template <typename T1, std::size_t E1, typename T2, std::size_t E2>
constexpr bool operator==(const span<T1, E1>& lhs, const span<T2, E2>& rhs) noexcept {
    if (lhs.size() != rhs.size()) return false;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) return false;
    }
    return true;
}

/**
 * @brief span同士の三方比較
 * @tparam T1, T2 要素型
 * @tparam E1, E2 Extent値
 * @param lhs 左辺span
 * @param rhs 右辺span
 * @return 比較結果
 */
template <typename T1, std::size_t E1, typename T2, std::size_t E2>
constexpr auto operator<=>(const span<T1, E1>& lhs, const span<T2, E2>& rhs) noexcept {
    const auto min_size = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();
    for (std::size_t i = 0; i < min_size; ++i) {
        if (auto cmp = lhs[i] <=> rhs[i]; cmp != 0) {
            return cmp;
        }
    }
    return lhs.size() <=> rhs.size();
}

// --- ヘルパー関数 ---

/**
 * @brief バイト表現としてのspanを取得
 * @tparam T 要素型
 * @tparam Extent サイズ
 * @param s 元のspan
 * @return バイトspanとしてのビュー
 */
template <typename T, std::size_t Extent>
constexpr auto as_bytes(span<T, Extent> s) noexcept {
    constexpr auto new_extent = (Extent == dynamic_extent) ? dynamic_extent : Extent * sizeof(T);
    return span<const std::byte, new_extent>(reinterpret_cast<const std::byte*>(s.data()), s.size_bytes());
}

/**
 * @brief 書き込み可能バイト表現としてのspanを取得
 * @tparam T 要素型
 * @tparam Extent サイズ
 * @param s 元のspan
 * @return 書き込み可能バイトspanとしてのビュー
 */
template <typename T, std::size_t Extent>
    requires(!std::is_const_v<T>)
constexpr auto as_writable_bytes(span<T, Extent> s) noexcept {
    constexpr auto new_extent = (Extent == dynamic_extent) ? dynamic_extent : Extent * sizeof(T);
    return span<std::byte, new_extent>(reinterpret_cast<std::byte*>(s.data()), s.size_bytes());
}

/**
 * @brief array_view型エイリアス（std::spanとの互換性）
 */
template <typename T, std::size_t Extent = dynamic_extent>
using array_view = span<T, Extent>;

}  // namespace bluestl
