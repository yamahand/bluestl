// -----------------------------------------------------------------------------
// Bluestl string_view.h
// C++20準拠・STL風インターフェースの文字列ビュークラス
// -----------------------------------------------------------------------------
/**
 * @file string_view.h
 * @brief Bluestlプロジェクトのstring_viewクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * string_viewは、文字列データへの読み取り専用ビューを提供するクラスです。
 * - RTTIなし、例外なし、header-only設計
 * - STL std::string_viewに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
 *   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
 *
 * 主な特徴:
 *   - C++20準拠、STL std::string_view風のAPI
 *   - RTTI/例外なし
 *   - header-only、#pragma onceによるインクルードガード
 *   - substr/find/starts_with/ends_with などの操作
 *   - イテレータ(begin/end/rbegin/rend)
 *   - 比較演算子（==, !=, <, >, <=, >=）
 *   - メモリ確保なし（既存文字列への参照のみ）
 *   - const char*および std::stringとの相互運用性
 *
 * Bluestl全体の設計方針:
 *   - 高速なコンパイル・実行
 *   - RTTI/例外/最小限のヒープ割り当て
 *   - header-only
 *   - STLに似たインターフェース
 *   - シンプルで明確なC++コード
 *   - 分離・粒度の細かい設計
 *   - STL std::string_viewとの違い: 例外非対応、RTTI非使用、最小限の実装
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <utility>
#include <type_traits>
#include <iterator>
#include <concepts>
#include <algorithm>
#include "assert_handler.h"

namespace bluestl {

/**
 * @class string_view
 * @brief 文字列データへの読み取り専用ビューを提供するクラス
 *
 * string_viewは文字列データのコピーを作成せず、既存の文字列データへの
 * 軽量な参照を提供します。主に関数の引数や戻り値として使用されます。
 *
 * @details
 * - const char*とsize_tを保持するだけの軽量なクラス
 * - ヒープ確保を行わない
 * - null終端を前提としない（明示的なサイズを保持）
 * - STL std::string_viewと類似のインターフェース
 */
class string_view {
   public:
    // ========================================
    // Type Definitions
    // ========================================
    using value_type = char;
    using pointer = const char*;
    using const_pointer = const char*;
    using reference = const char&;
    using const_reference = const char&;
    using iterator = const char*;
    using const_iterator = const char*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // ========================================
    // Constants
    // ========================================
    static constexpr size_type npos = static_cast<size_type>(-1);

   private:
    const char* data_;  ///< 文字列データへのポインタ
    size_type size_;    ///< 文字列の長さ

   public:
    // ========================================
    // Constructors and Destructor
    // ========================================

    /**
     * @brief デフォルトコンストラクタ
     * @details 空のstring_viewを作成します
     */
    constexpr string_view() noexcept : data_(nullptr), size_(0) {}

    /**
     * @brief null終端文字列からのコンストラクタ
     * @param str null終端文字列
     * @details strがnullptrの場合は空のstring_viewを作成します
     */
    constexpr string_view(const char* str) noexcept
        : data_(str), size_(str ? std::char_traits<char>::length(str) : 0) {}

    /**
     * @brief 文字列データとサイズからのコンストラクタ
     * @param str 文字列データへのポインタ
     * @param len 文字列の長さ
     */
    constexpr string_view(const char* str, size_type len) noexcept : data_(str), size_(len) {}

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元のstring_view
     */
    constexpr string_view(const string_view& other) noexcept = default;

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元のstring_view
     */
    constexpr string_view(string_view&& other) noexcept = default;

    /**
     * @brief デストラクタ
     */
    ~string_view() = default;

    // ========================================
    // Assignment Operators
    // ========================================

    /**
     * @brief コピー代入演算子
     * @param other 代入元のstring_view
     * @return *this
     */
    constexpr string_view& operator=(const string_view& other) noexcept = default;

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元のstring_view
     * @return *this
     */
    constexpr string_view& operator=(string_view&& other) noexcept = default;

    // ========================================
    // Iterators
    // ========================================

    /**
     * @brief 先頭要素へのイテレータを取得
     * @return 先頭要素へのconst_iterator
     */
    constexpr const_iterator begin() const noexcept {
        return data_;
    }

    /**
     * @brief 先頭要素へのイテレータを取得
     * @return 先頭要素へのconst_iterator
     */
    constexpr const_iterator cbegin() const noexcept {
        return data_;
    }

    /**
     * @brief 末尾の次の要素へのイテレータを取得
     * @return 末尾の次の要素へのconst_iterator
     */
    constexpr const_iterator end() const noexcept {
        return data_ + size_;
    }

    /**
     * @brief 末尾の次の要素へのイテレータを取得
     * @return 末尾の次の要素へのconst_iterator
     */
    constexpr const_iterator cend() const noexcept {
        return data_ + size_;
    }

    /**
     * @brief 逆順先頭要素への逆イテレータを取得
     * @return 逆順先頭要素へのconst_reverse_iterator
     */
    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 逆順先頭要素への逆イテレータを取得
     * @return 逆順先頭要素へのconst_reverse_iterator
     */
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 逆順末尾の次の要素への逆イテレータを取得
     * @return 逆順末尾の次の要素へのconst_reverse_iterator
     */
    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief 逆順末尾の次の要素への逆イテレータを取得
     * @return 逆順末尾の次の要素へのconst_reverse_iterator
     */
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    // ========================================
    // Element Access
    // ========================================

    /**
     * @brief 指定位置の文字への参照を取得
     * @param pos 位置（0から始まる）
     * @return 指定位置の文字への参照
     * @details 範囲外アクセスの場合、アサーションでエラーとなります
     */
    constexpr const_reference operator[](size_type pos) const noexcept {
        BLUESTL_ASSERT_MSG(pos < size_, "string_view::operator[]: index out of range");
        return data_[pos];
    }

    /**
     * @brief 指定位置の文字への参照を取得（範囲チェック付き）
     * @param pos 位置（0から始まる）
     * @return 指定位置の文字への参照
     * @details 範囲外アクセスの場合、アサーションでエラーとなります
     */
    constexpr const_reference at(size_type pos) const noexcept {
        BLUESTL_ASSERT_MSG(pos < size_, "string_view::at: index out of range");
        return data_[pos];
    }

    /**
     * @brief 先頭文字への参照を取得
     * @return 先頭文字への参照
     * @details 空のstring_viewの場合、未定義動作となります
     */
    constexpr const_reference front() const noexcept {
        BLUESTL_ASSERT_MSG(!empty(), "string_view::front: empty string_view");
        return data_[0];
    }

    /**
     * @brief 末尾文字への参照を取得
     * @return 末尾文字への参照
     * @details 空のstring_viewの場合、未定義動作となります
     */
    constexpr const_reference back() const noexcept {
        BLUESTL_ASSERT_MSG(!empty(), "string_view::back: empty string_view");
        return data_[size_ - 1];
    }

    /**
     * @brief 文字列データへのポインタを取得
     * @return 文字列データへのポインタ（null終端は保証されません）
     */
    constexpr const_pointer data() const noexcept {
        return data_;
    }

    // ========================================
    // Capacity
    // ========================================

    /**
     * @brief 文字列の長さを取得
     * @return 文字列の長さ
     */
    constexpr size_type size() const noexcept {
        return size_;
    }

    /**
     * @brief 文字列の長さを取得
     * @return 文字列の長さ
     */
    constexpr size_type length() const noexcept {
        return size_;
    }

    /**
     * @brief 文字列が空かどうかを判定
     * @return 空の場合true、そうでなければfalse
     */
    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief 最大可能な文字列長を取得
     * @return 最大可能な文字列長
     */
    constexpr size_type max_size() const noexcept {
        return static_cast<size_type>(-1) / sizeof(char);
    }

    // ========================================
    // Modifiers
    // ========================================

    /**
     * @brief 先頭から指定文字数を削除
     * @param n 削除する文字数
     * @details nがsize()より大きい場合、全ての文字が削除されます
     */
    constexpr void remove_prefix(size_type n) noexcept {
        if (n > size_) {
            n = size_;
        }
        data_ += n;
        size_ -= n;
    }

    /**
     * @brief 末尾から指定文字数を削除
     * @param n 削除する文字数
     * @details nがsize()より大きい場合、全ての文字が削除されます
     */
    constexpr void remove_suffix(size_type n) noexcept {
        if (n > size_) {
            n = size_;
        }
        size_ -= n;
    }

    /**
     * @brief 別のstring_viewと内容を交換
     * @param other 交換先のstring_view
     */
    constexpr void swap(string_view& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
    }

    // ========================================
    // String Operations
    // ========================================

    /**
     * @brief 部分文字列のビューを取得
     * @param pos 開始位置（デフォルト: 0）
     * @param len 長さ（デフォルト: npos）
     * @return 部分文字列のstring_view
     * @details posがsize()以上の場合、空のstring_viewを返します
     */
    constexpr string_view substr(size_type pos = 0, size_type len = npos) const noexcept {
        if (pos >= size_) {
            return string_view();
        }
        size_type actual_len = std::min(len, size_ - pos);
        return string_view(data_ + pos, actual_len);
    }

    /**
     * @brief 文字列比較
     * @param sv 比較対象のstring_view
     * @return 比較結果（負の値: *this < sv、0: *this == sv、正の値: *this > sv）
     */
    constexpr int compare(string_view sv) const noexcept {
        size_type min_size = std::min(size_, sv.size_);
        if (min_size > 0) {
            int result = std::char_traits<char>::compare(data_, sv.data_, min_size);
            if (result != 0) {
                return result;
            }
        }
        if (size_ < sv.size_) return -1;
        if (size_ > sv.size_) return 1;
        return 0;
    }

    /**
     * @brief 部分文字列比較
     * @param pos1 自身の開始位置
     * @param n1 自身の比較する長さ
     * @param sv 比較対象のstring_view
     * @return 比較結果
     */
    constexpr int compare(size_type pos1, size_type n1, string_view sv) const noexcept {
        return substr(pos1, n1).compare(sv);
    }

    /**
     * @brief 指定文字の検索
     * @param ch 検索する文字
     * @param pos 検索開始位置（デフォルト: 0）
     * @return 見つかった位置、見つからない場合はnpos
     */
    constexpr size_type find(char ch, size_type pos = 0) const noexcept {
        if (pos >= size_) {
            return npos;
        }
        for (size_type i = pos; i < size_; ++i) {
            if (data_[i] == ch) {
                return i;
            }
        }
        return npos;
    }

    /**
     * @brief 部分文字列の検索
     * @param sv 検索するstring_view
     * @param pos 検索開始位置（デフォルト: 0）
     * @return 見つかった位置、見つからない場合はnpos
     */
    constexpr size_type find(string_view sv, size_type pos = 0) const noexcept {
        if (sv.empty()) {
            return pos <= size_ ? pos : npos;
        }
        if (pos + sv.size_ > size_) {
            return npos;
        }
        for (size_type i = pos; i <= size_ - sv.size_; ++i) {
            if (substr(i, sv.size_).compare(sv) == 0) {
                return i;
            }
        }
        return npos;
    }

    /**
     * @brief 指定文字の後方検索
     * @param ch 検索する文字
     * @param pos 検索開始位置（デフォルト: npos）
     * @return 見つかった位置、見つからない場合はnpos
     */
    constexpr size_type rfind(char ch, size_type pos = npos) const noexcept {
        if (empty()) {
            return npos;
        }
        if (pos >= size_) {
            pos = size_ - 1;
        }
        for (size_type i = pos + 1; i > 0; --i) {
            if (data_[i - 1] == ch) {
                return i - 1;
            }
        }
        return npos;
    }

    /**
     * @brief 部分文字列の後方検索
     * @param sv 検索するstring_view
     * @param pos 検索開始位置（デフォルト: npos）
     * @return 見つかった位置、見つからない場合はnpos
     */
    constexpr size_type rfind(string_view sv, size_type pos = npos) const noexcept {
        if (sv.empty()) {
            return pos <= size_ ? std::min(pos, size_) : size_;
        }
        if (sv.size_ > size_) {
            return npos;
        }
        if (pos > size_ - sv.size_) {
            pos = size_ - sv.size_;
        }
        for (size_type i = pos + 1; i > 0; --i) {
            if (substr(i - 1, sv.size_).compare(sv) == 0) {
                return i - 1;
            }
        }
        return npos;
    }

    /**
     * @brief 指定した文字列で始まるかを判定
     * @param sv 判定するstring_view
     * @return 指定した文字列で始まる場合true、そうでなければfalse
     */
    constexpr bool starts_with(string_view sv) const noexcept {
        return size_ >= sv.size_ && substr(0, sv.size_).compare(sv) == 0;
    }

    /**
     * @brief 指定した文字で始まるかを判定
     * @param ch 判定する文字
     * @return 指定した文字で始まる場合true、そうでなければfalse
     */
    constexpr bool starts_with(char ch) const noexcept {
        return !empty() && front() == ch;
    }

    /**
     * @brief 指定した文字列で終わるかを判定
     * @param sv 判定するstring_view
     * @return 指定した文字列で終わる場合true、そうでなければfalse
     */
    constexpr bool ends_with(string_view sv) const noexcept {
        return size_ >= sv.size_ && substr(size_ - sv.size_).compare(sv) == 0;
    }

    /**
     * @brief 指定した文字で終わるかを判定
     * @param ch 判定する文字
     * @return 指定した文字で終わる場合true、そうでなければfalse
     */
    constexpr bool ends_with(char ch) const noexcept {
        return !empty() && back() == ch;
    }

    /**
     * @brief 指定した文字が含まれるかを判定
     * @param ch 判定する文字
     * @return 指定した文字が含まれる場合true、そうでなければfalse
     */
    constexpr bool contains(char ch) const noexcept {
        return find(ch) != npos;
    }

    /**
     * @brief 指定した文字列が含まれるかを判定
     * @param sv 判定するstring_view
     * @return 指定した文字列が含まれる場合true、そうでなければfalse
     */
    constexpr bool contains(string_view sv) const noexcept {
        return find(sv) != npos;
    }
};

// ========================================
// Non-member Functions
// ========================================

/**
 * @brief 等価比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return 等価の場合true、そうでなければfalse
 */
constexpr bool operator==(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) == 0;
}

/**
 * @brief 非等価比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return 非等価の場合true、そうでなければfalse
 */
constexpr bool operator!=(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) != 0;
}

/**
 * @brief 小なり比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return lhs < rhsの場合true、そうでなければfalse
 */
constexpr bool operator<(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) < 0;
}

/**
 * @brief 小なりイコール比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return lhs <= rhsの場合true、そうでなければfalse
 */
constexpr bool operator<=(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) <= 0;
}

/**
 * @brief 大なり比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return lhs > rhsの場合true、そうでなければfalse
 */
constexpr bool operator>(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) > 0;
}

/**
 * @brief 大なりイコール比較演算子
 * @param lhs 左辺のstring_view
 * @param rhs 右辺のstring_view
 * @return lhs >= rhsの場合true、そうでなければfalse
 */
constexpr bool operator>=(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) >= 0;
}

/**
 * @brief swap関数
 * @param lhs 1つ目のstring_view
 * @param rhs 2つ目のstring_view
 */
constexpr void swap(string_view& lhs, string_view& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace bluestl
