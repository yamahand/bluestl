// -----------------------------------------------------------------------------
// Bluestl fixed_string.h
// C++20準拠・STL風インターフェースの固定サイズ文字列
// -----------------------------------------------------------------------------
/**
 * @file fixed_string.h
 * @brief Bluestlプロジェクトのfixed_stringクラスを提供します。
 *
 * Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
 *
 * @details
 * fixed_stringは、固定サイズの文字列コンテナです。
 * - RTTIなし、例外なし、ヒープ割り当てなし、header-only設計
 * - STL std::stringに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
 *   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
 *
 * 主な特徴:
 *   - C++20準拠、STL std::string風のAPI
 *   - RTTI/例外なし
 *   - header-only、#pragma onceによるインクルードガード
 *   - append/substr/find/c_str/clear/resize などの操作
 *   - イテレータ(begin/end/rbegin/rend)
 *   - 比較演算子（==, !=, <, >, <=, >=）
 *   - 文字列リテラル対応
 *
 * Bluestl全体の設計方針:
 *   - 高速なコンパイル・実行
 *   - RTTI/例外/ヒープ割り当てなし
 *   - header-only
 *   - STLに似たインターフェース
 *   - シンプルで明確なC++コード
 *   - 分離・粒度の細かい設計
 *   - STL std::stringとの違い: 固定サイズ、例外非対応、RTTI非使用、最小限の実装
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>
#include <type_traits>
#include <iterator>
#include "assert_handler.h"

namespace bluestl {

/**
 * @class fixed_string
 * @brief 固定サイズ文字列コンテナ。STL std::string風インターフェース。
 * @tparam Capacity 最大文字数（null終端文字含まず）
 *
 * - 固定サイズでヒープ割り当てなし
 * - RTTI/例外なし
 * - STL std::string風インターフェース
 * - constexpr対応
 */
template <std::size_t Capacity>
class fixed_string {
public:
    using value_type = char;
    using reference = char&;
    using const_reference = const char&;
    using size_type = std::size_t;
    using iterator = char*;
    using const_iterator = const char*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos = static_cast<size_type>(-1);    /**
     * @brief デフォルトコンストラクタ
     */
    constexpr fixed_string() noexcept : size_(0), storage_{'\0'} {
    }

    /**
     * @brief デストラクタ
     */
    ~fixed_string() noexcept = default;    /**
     * @brief C文字列からの構築
     * @param str C文字列
     */
    constexpr fixed_string(const char* str) noexcept : size_(0), storage_{'\0'} {
        if (str) {
            assign(str);
        }
    }    /**
     * @brief string_viewからの構築
     * @param sv string_view
     */
    constexpr fixed_string(std::string_view sv) noexcept : size_(0), storage_{'\0'} {
        assign(sv);
    }    /**
     * @brief C文字列から指定長さで構築
     * @param str C文字列
     * @param count 使用する文字数
     */
    constexpr fixed_string(const char* str, size_type count) noexcept : size_(0), storage_{'\0'} {
        assign(str, count);
    }

    /**
     * @brief 文字を指定回数繰り返して構築
     * @param count 文字数
     * @param ch 文字
     */
    constexpr fixed_string(size_type count, char ch) noexcept : size_(0), storage_{'\0'} {
        assign(count, ch);
    }    /**
     * @brief イテレータから構築
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     */
    template<typename InputIt>
    constexpr fixed_string(InputIt first, InputIt last) noexcept : size_(0), storage_{'\0'} {
        assign(first, last);
    }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元
     */
    constexpr fixed_string(const fixed_string& other) noexcept : size_(0), storage_{'\0'} {
        assign(other);
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元
     */
    constexpr fixed_string(fixed_string&& other) noexcept : size_(0), storage_{'\0'} {
        assign(other);
        other.clear();
    }

    /**
     * @brief コピー代入演算子
     * @param other コピー元
     * @return *this
     */
    constexpr fixed_string& operator=(const fixed_string& other) noexcept {
        if (this != &other) {
            assign(other);
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元
     * @return *this
     */
    constexpr fixed_string& operator=(fixed_string&& other) noexcept {
        if (this != &other) {
            assign(other);
            other.clear();
        }
        return *this;
    }

    /**
     * @brief C文字列代入演算子
     * @param str C文字列
     * @return *this
     */
    constexpr fixed_string& operator=(const char* str) noexcept {
        assign(str);
        return *this;
    }

    /**
     * @brief string_view代入演算子
     * @param sv string_view
     * @return *this
     */
    constexpr fixed_string& operator=(std::string_view sv) noexcept {
        assign(sv);
        return *this;
    }

    /**
     * @brief 文字列の長さを取得
     * @return 文字列の長さ
     */
    [[nodiscard]] constexpr size_type size() const noexcept {
        return size_;
    }

    /**
     * @brief 文字列の長さを取得（sizeのエイリアス）
     * @return 文字列の長さ
     */
    [[nodiscard]] constexpr size_type length() const noexcept {
        return size_;
    }

    /**
     * @brief 最大容量を取得
     * @return 最大容量
     */
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return Capacity;
    }

    /**
     * @brief 最大容量を取得（capacityのエイリアス）
     * @return 最大容量
     */
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return Capacity;
    }

    /**
     * @brief 空かどうか判定
     * @return 空ならtrue
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief 内部データへのポインタ取得
     * @return データポインタ
     */
    [[nodiscard]] constexpr char* data() noexcept {
        return storage_;
    }

    /**
     * @brief 内部データへのポインタ取得（const版）
     * @return データポインタ
     */
    [[nodiscard]] constexpr const char* data() const noexcept {
        return storage_;
    }

    /**
     * @brief C文字列として取得
     * @return null終端文字列
     */
    [[nodiscard]] constexpr const char* c_str() const noexcept {
        return storage_;
    }

    /**
     * @brief 指定インデックスの文字参照
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept {
        return storage_[pos];
    }

    /**
     * @brief 指定インデックスの文字参照（const版）
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept {
        return storage_[pos];
    }

    /**
     * @brief 範囲チェック付きアクセス
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] constexpr reference at(size_type pos) noexcept {
        BLUESTL_ASSERT(pos < size_);
        return storage_[pos];
    }

    /**
     * @brief 範囲チェック付きアクセス（const版）
     * @param pos インデックス
     * @return 文字参照
     */
    [[nodiscard]] constexpr const_reference at(size_type pos) const noexcept {
        BLUESTL_ASSERT(pos < size_);
        return storage_[pos];
    }

    /**
     * @brief 先頭文字参照
     * @return 先頭文字参照
     */
    [[nodiscard]] constexpr reference front() noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return storage_[0];
    }

    /**
     * @brief 先頭文字参照（const版）
     * @return 先頭文字参照
     */
    [[nodiscard]] constexpr const_reference front() const noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return storage_[0];
    }

    /**
     * @brief 末尾文字参照
     * @return 末尾文字参照
     */
    [[nodiscard]] constexpr reference back() noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return storage_[size_ - 1];
    }

    /**
     * @brief 末尾文字参照（const版）
     * @return 末尾文字参照
     */
    [[nodiscard]] constexpr const_reference back() const noexcept {
        BLUESTL_ASSERT(size_ > 0);
        return storage_[size_ - 1];
    }

    /**
     * @brief 先頭イテレータ
     * @return 先頭イテレータ
     */
    [[nodiscard]] constexpr iterator begin() noexcept {
        return storage_;
    }

    /**
     * @brief 先頭イテレータ（const版）
     * @return 先頭イテレータ
     */
    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return storage_;
    }

    /**
     * @brief 末尾イテレータ
     * @return 末尾イテレータ
     */
    [[nodiscard]] constexpr iterator end() noexcept {
        return storage_ + size_;
    }

    /**
     * @brief 末尾イテレータ（const版）
     * @return 末尾イテレータ
     */
    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return storage_ + size_;
    }

    /**
     * @brief constイテレータの先頭
     * @return constイテレータの先頭
     */
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    /**
     * @brief constイテレータの末尾
     * @return constイテレータの末尾
     */
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    /**
     * @brief 逆イテレータの先頭
     * @return 逆イテレータの先頭
     */
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    /**
     * @brief 逆イテレータの先頭（const版）
     * @return 逆イテレータの先頭
     */
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief 逆イテレータの末尾
     * @return 逆イテレータの末尾
     */
    [[nodiscard]] constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    /**
     * @brief 逆イテレータの末尾（const版）
     * @return 逆イテレータの末尾
     */
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief const逆イテレータの先頭
     * @return const逆イテレータの先頭
     */
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    /**
     * @brief const逆イテレータの末尾
     * @return const逆イテレータの末尾
     */
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return rend();
    }

    /**
     * @brief 文字列をクリア
     */
    constexpr void clear() noexcept {
        size_ = 0;
        storage_[0] = '\0';
    }

    /**
     * @brief 末尾に文字を追加
     * @param ch 追加する文字
     * @return 追加に成功したらtrue
     */
    constexpr bool push_back(char ch) noexcept {
        if (size_ >= Capacity) return false;
        storage_[size_] = ch;
        ++size_;
        storage_[size_] = '\0';
        return true;
    }

    /**
     * @brief 末尾の文字を削除
     */
    constexpr void pop_back() noexcept {
        if (size_ > 0) {
            --size_;
            storage_[size_] = '\0';
        }
    }

    /**
     * @brief 文字列を代入
     * @param str C文字列
     */
    constexpr void assign(const char* str) noexcept {
        clear();
        if (str) {
            size_type len = 0;
            while (str[len] && len < Capacity) {
                storage_[len] = str[len];
                ++len;
            }
            size_ = len;
            storage_[size_] = '\0';
        }
    }

    /**
     * @brief 文字列を代入
     * @param sv string_view
     */
    constexpr void assign(std::string_view sv) noexcept {
        clear();
        size_type len = (sv.size() > Capacity) ? Capacity : sv.size();
        for (size_type i = 0; i < len; ++i) {
            storage_[i] = sv[i];
        }
        size_ = len;
        storage_[size_] = '\0';
    }    /**
     * @brief 文字列を代入
     * @param other 他のfixed_string
     */
    constexpr void assign(const fixed_string& other) noexcept {
        clear();
        size_type len = (other.size_ > Capacity) ? Capacity : other.size_;
        for (size_type i = 0; i < len; ++i) {
            storage_[i] = other.storage_[i];
        }
        size_ = len;
        storage_[size_] = '\0';
    }

    /**
     * @brief 文字列を指定長さで代入
     * @param str C文字列
     * @param count 使用する文字数
     */
    constexpr void assign(const char* str, size_type count) noexcept {
        clear();
        if (str) {
            size_type len = (count > Capacity) ? Capacity : count;
            for (size_type i = 0; i < len && str[i]; ++i) {
                storage_[i] = str[i];
                size_ = i + 1;
            }
            storage_[size_] = '\0';
        }
    }

    /**
     * @brief 文字を指定回数で代入
     * @param count 文字数
     * @param ch 文字
     */
    constexpr void assign(size_type count, char ch) noexcept {
        clear();
        size_type len = (count > Capacity) ? Capacity : count;
        for (size_type i = 0; i < len; ++i) {
            storage_[i] = ch;
        }
        size_ = len;
        storage_[size_] = '\0';
    }

    /**
     * @brief イテレータから代入
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     */
    template<typename InputIt>
    constexpr void assign(InputIt first, InputIt last) noexcept {
        clear();
        for (auto it = first; it != last && size_ < Capacity; ++it) {
            storage_[size_] = *it;
            ++size_;
        }
        storage_[size_] = '\0';
    }

    /**
     * @brief 文字列を末尾に追加
     * @param str C文字列
     * @return 追加に成功したらtrue
     */
    constexpr bool append(const char* str) noexcept {
        if (!str) return true;

        size_type str_len = 0;
        while (str[str_len]) ++str_len;

        if (size_ + str_len > Capacity) return false;

        for (size_type i = 0; i < str_len; ++i) {
            storage_[size_ + i] = str[i];
        }
        size_ += str_len;
        storage_[size_] = '\0';
        return true;
    }    /**
     * @brief 文字列を末尾に追加
     * @param sv string_view
     * @return 追加に成功したらtrue
     */
    constexpr bool append(std::string_view sv) noexcept {
        if (size_ + sv.size() > Capacity) return false;

        for (size_type i = 0; i < sv.size(); ++i) {
            storage_[size_ + i] = sv[i];
        }
        size_ += sv.size();
        storage_[size_] = '\0';
        return true;
    }

    /**
     * @brief 文字列を指定長さで末尾に追加
     * @param str C文字列
     * @param count 使用する文字数
     * @return 追加に成功したらtrue
     */
    constexpr bool append(const char* str, size_type count) noexcept {
        if (!str) return true;

        size_type actual_count = 0;
        for (size_type i = 0; i < count && str[i]; ++i) {
            ++actual_count;
        }

        if (size_ + actual_count > Capacity) return false;

        for (size_type i = 0; i < actual_count; ++i) {
            storage_[size_ + i] = str[i];
        }
        size_ += actual_count;
        storage_[size_] = '\0';
        return true;
    }

    /**
     * @brief 文字を指定回数末尾に追加
     * @param count 文字数
     * @param ch 文字
     * @return 追加に成功したらtrue
     */
    constexpr bool append(size_type count, char ch) noexcept {
        if (size_ + count > Capacity) return false;

        for (size_type i = 0; i < count; ++i) {
            storage_[size_ + i] = ch;
        }
        size_ += count;
        storage_[size_] = '\0';
        return true;
    }

    /**
     * @brief イテレータから末尾に追加
     * @tparam InputIt 入力イテレータ型
     * @param first 開始イテレータ
     * @param last 終了イテレータ
     * @return 追加に成功したらtrue
     */
    template<typename InputIt>
    constexpr bool append(InputIt first, InputIt last) noexcept {
        size_type old_size = size_;
        for (auto it = first; it != last && size_ < Capacity; ++it) {
            storage_[size_] = *it;
            ++size_;
        }
        storage_[size_] = '\0';
        return (size_ < Capacity || first == last);
    }

    /**
     * @brief 文字列を末尾に追加
     * @param other 他のfixed_string
     * @return 追加に成功したらtrue
     */
    constexpr bool append(const fixed_string& other) noexcept {
        if (size_ + other.size_ > Capacity) return false;

        for (size_type i = 0; i < other.size_; ++i) {
            storage_[size_ + i] = other.storage_[i];
        }
        size_ += other.size_;
        storage_[size_] = '\0';
        return true;
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param str C文字列
     * @return *this
     */
    constexpr fixed_string& operator+=(const char* str) noexcept {
        append(str);
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param sv string_view
     * @return *this
     */
    constexpr fixed_string& operator+=(std::string_view sv) noexcept {
        append(sv);
        return *this;
    }

    /**
     * @brief 文字列を末尾に追加（+=演算子）
     * @param other 他のfixed_string
     * @return *this
     */
    constexpr fixed_string& operator+=(const fixed_string& other) noexcept {
        append(other);
        return *this;
    }

    /**
     * @brief 文字を末尾に追加（+=演算子）
     * @param ch 文字
     * @return *this
     */
    constexpr fixed_string& operator+=(char ch) noexcept {
        push_back(ch);
        return *this;
    }

    /**
     * @brief 部分文字列を取得
     * @param pos 開始位置
     * @param len 長さ（デフォルトは末尾まで）
     * @return 部分文字列
     */
    [[nodiscard]] constexpr fixed_string substr(size_type pos = 0, size_type len = npos) const noexcept {
        fixed_string result;
        if (pos >= size_) return result;

        size_type actual_len = (len == npos || pos + len > size_) ? size_ - pos : len;
        for (size_type i = 0; i < actual_len && i < Capacity; ++i) {
            result.storage_[i] = storage_[pos + i];
        }
        result.size_ = (actual_len > Capacity) ? Capacity : actual_len;
        result.storage_[result.size_] = '\0';
        return result;
    }

    /**
     * @brief 文字列を検索
     * @param str 検索する文字列
     * @param pos 検索開始位置
     * @return 見つかった位置（見つからない場合はnpos）
     */
    [[nodiscard]] constexpr size_type find(const char* str, size_type pos = 0) const noexcept {
        if (!str || pos >= size_) return npos;

        size_type str_len = 0;
        while (str[str_len]) ++str_len;

        if (str_len == 0) return pos;
        if (str_len > size_ - pos) return npos;

        for (size_type i = pos; i <= size_ - str_len; ++i) {
            bool match = true;
            for (size_type j = 0; j < str_len; ++j) {
                if (storage_[i + j] != str[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return npos;
    }

    /**
     * @brief 文字列を検索
     * @param sv 検索するstring_view
     * @param pos 検索開始位置
     * @return 見つかった位置（見つからない場合はnpos）
     */
    [[nodiscard]] constexpr size_type find(std::string_view sv, size_type pos = 0) const noexcept {
        if (pos >= size_) return npos;
        if (sv.empty()) return pos;
        if (sv.size() > size_ - pos) return npos;

        for (size_type i = pos; i <= size_ - sv.size(); ++i) {
            bool match = true;
            for (size_type j = 0; j < sv.size(); ++j) {
                if (storage_[i + j] != sv[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return npos;
    }    /**
     * @brief 文字を検索
     * @param ch 検索する文字
     * @param pos 検索開始位置
     * @return 見つかった位置（見つからない場合はnpos）
     */
    [[nodiscard]] constexpr size_type find(char ch, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < size_; ++i) {
            if (storage_[i] == ch) return i;
        }
        return npos;
    }

    /**
     * @brief 指定した文字列で始まるかチェック
     * @param str 検索する文字列
     * @return 始まる場合はtrue
     */
    [[nodiscard]] constexpr bool starts_with(const char* str) const noexcept {
        if (!str) return true;
        return find(str) == 0;
    }

    /**
     * @brief 指定した文字で始まるかチェック
     * @param ch 検索する文字
     * @return 始まる場合はtrue
     */
    [[nodiscard]] constexpr bool starts_with(char ch) const noexcept {
        return !empty() && front() == ch;
    }

    /**
     * @brief 指定したstring_viewで始まるかチェック
     * @param sv 検索するstring_view
     * @return 始まる場合はtrue
     */
    [[nodiscard]] constexpr bool starts_with(std::string_view sv) const noexcept {
        return find(sv) == 0;
    }

    /**
     * @brief 指定した文字列で終わるかチェック
     * @param str 検索する文字列
     * @return 終わる場合はtrue
     */
    [[nodiscard]] constexpr bool ends_with(const char* str) const noexcept {
        if (!str) return true;
        size_type str_len = 0;
        while (str[str_len]) ++str_len;
        if (str_len > size_) return false;
        if (str_len == 0) return true;

        for (size_type i = 0; i < str_len; ++i) {
            if (storage_[size_ - str_len + i] != str[i]) return false;
        }
        return true;
    }

    /**
     * @brief 指定した文字で終わるかチェック
     * @param ch 検索する文字
     * @return 終わる場合はtrue
     */
    [[nodiscard]] constexpr bool ends_with(char ch) const noexcept {
        return !empty() && back() == ch;
    }

    /**
     * @brief 指定したstring_viewで終わるかチェック
     * @param sv 検索するstring_view
     * @return 終わる場合はtrue
     */
    [[nodiscard]] constexpr bool ends_with(std::string_view sv) const noexcept {
        if (sv.size() > size_) return false;
        if (sv.empty()) return true;

        for (size_type i = 0; i < sv.size(); ++i) {
            if (storage_[size_ - sv.size() + i] != sv[i]) return false;
        }
        return true;
    }

    /**
     * @brief 指定した文字列が含まれるかチェック
     * @param str 検索する文字列
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] constexpr bool contains(const char* str) const noexcept {
        return find(str) != npos;
    }

    /**
     * @brief 指定した文字が含まれるかチェック
     * @param ch 検索する文字
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] constexpr bool contains(char ch) const noexcept {
        return find(ch) != npos;
    }

    /**
     * @brief 指定したstring_viewが含まれるかチェック
     * @param sv 検索するstring_view
     * @return 含まれる場合はtrue
     */
    [[nodiscard]] constexpr bool contains(std::string_view sv) const noexcept {
        return find(sv) != npos;
    }

    /**
     * @brief サイズを変更
     * @param count 新しいサイズ
     * @param ch 埋める文字（デフォルトは'\0'）
     */
    constexpr void resize(size_type count, char ch = '\0') noexcept {
        if (count > Capacity) count = Capacity;

        if (count > size_) {
            for (size_type i = size_; i < count; ++i) {
                storage_[i] = ch;
            }
        }
        size_ = count;
        storage_[size_] = '\0';
    }

    /**
     * @brief string_viewに変換
     * @return string_view
     */
    [[nodiscard]] constexpr operator std::string_view() const noexcept {
        return std::string_view(storage_, size_);
    }

    // 比較演算子（friend関数として定義）
    friend constexpr bool operator==(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        if (lhs.size_ != rhs.size_) return false;
        for (size_type i = 0; i < lhs.size_; ++i) {
            if (lhs.storage_[i] != rhs.storage_[i]) return false;
        }
        return true;
    }

    friend constexpr bool operator!=(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        size_type min_size = (lhs.size_ < rhs.size_) ? lhs.size_ : rhs.size_;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.storage_[i] < rhs.storage_[i]) return true;
            if (lhs.storage_[i] > rhs.storage_[i]) return false;
        }
        return lhs.size_ < rhs.size_;
    }

    friend constexpr bool operator>(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator<=(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator>=(const fixed_string& lhs, const fixed_string& rhs) noexcept {
        return !(lhs < rhs);
    }

    // C文字列との比較
    friend constexpr bool operator==(const fixed_string& lhs, const char* rhs) noexcept {
        if (!rhs) return lhs.empty();
        size_type rhs_len = 0;
        while (rhs[rhs_len]) ++rhs_len;
        if (lhs.size_ != rhs_len) return false;
        for (size_type i = 0; i < lhs.size_; ++i) {
            if (lhs.storage_[i] != rhs[i]) return false;
        }
        return true;
    }

    friend constexpr bool operator==(const char* lhs, const fixed_string& rhs) noexcept {
        return rhs == lhs;
    }

    friend constexpr bool operator!=(const fixed_string& lhs, const char* rhs) noexcept {
        return !(lhs == rhs);
    }    friend constexpr bool operator!=(const char* lhs, const fixed_string& rhs) noexcept {
        return !(rhs == lhs);
    }

    // C文字列との比較（順序）
    friend constexpr bool operator<(const fixed_string& lhs, const char* rhs) noexcept {
        if (!rhs) return false;
        size_type rhs_len = 0;
        while (rhs[rhs_len]) ++rhs_len;
        size_type min_size = (lhs.size_ < rhs_len) ? lhs.size_ : rhs_len;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.storage_[i] < rhs[i]) return true;
            if (lhs.storage_[i] > rhs[i]) return false;
        }
        return lhs.size_ < rhs_len;
    }

    friend constexpr bool operator<(const char* lhs, const fixed_string& rhs) noexcept {
        if (!lhs) return !rhs.empty();
        size_type lhs_len = 0;
        while (lhs[lhs_len]) ++lhs_len;
        size_type min_size = (lhs_len < rhs.size_) ? lhs_len : rhs.size_;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs[i] < rhs.storage_[i]) return true;
            if (lhs[i] > rhs.storage_[i]) return false;
        }
        return lhs_len < rhs.size_;
    }

    friend constexpr bool operator>(const fixed_string& lhs, const char* rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator>(const char* lhs, const fixed_string& rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator<=(const fixed_string& lhs, const char* rhs) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator<=(const char* lhs, const fixed_string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator>=(const fixed_string& lhs, const char* rhs) noexcept {
        return !(lhs < rhs);
    }

    friend constexpr bool operator>=(const char* lhs, const fixed_string& rhs) noexcept {
        return !(lhs < rhs);
    }

    // string_viewとの比較
    friend constexpr bool operator==(const fixed_string& lhs, std::string_view rhs) noexcept {
        if (lhs.size_ != rhs.size()) return false;
        for (size_type i = 0; i < lhs.size_; ++i) {
            if (lhs.storage_[i] != rhs[i]) return false;
        }
        return true;
    }

    friend constexpr bool operator==(std::string_view lhs, const fixed_string& rhs) noexcept {
        return rhs == lhs;
    }

    friend constexpr bool operator!=(const fixed_string& lhs, std::string_view rhs) noexcept {
        return !(lhs == rhs);
    }    friend constexpr bool operator!=(std::string_view lhs, const fixed_string& rhs) noexcept {
        return !(rhs == lhs);
    }

    // string_viewとの比較（順序）
    friend constexpr bool operator<(const fixed_string& lhs, std::string_view rhs) noexcept {
        size_type min_size = (lhs.size_ < rhs.size()) ? lhs.size_ : rhs.size();
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs.storage_[i] < rhs[i]) return true;
            if (lhs.storage_[i] > rhs[i]) return false;
        }
        return lhs.size_ < rhs.size();
    }

    friend constexpr bool operator<(std::string_view lhs, const fixed_string& rhs) noexcept {
        size_type min_size = (lhs.size() < rhs.size_) ? lhs.size() : rhs.size_;
        for (size_type i = 0; i < min_size; ++i) {
            if (lhs[i] < rhs.storage_[i]) return true;
            if (lhs[i] > rhs.storage_[i]) return false;
        }
        return lhs.size() < rhs.size_;
    }

    friend constexpr bool operator>(const fixed_string& lhs, std::string_view rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator>(std::string_view lhs, const fixed_string& rhs) noexcept {
        return rhs < lhs;
    }

    friend constexpr bool operator<=(const fixed_string& lhs, std::string_view rhs) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator<=(std::string_view lhs, const fixed_string& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend constexpr bool operator>=(const fixed_string& lhs, std::string_view rhs) noexcept {
        return !(lhs < rhs);
    }

    friend constexpr bool operator>=(std::string_view lhs, const fixed_string& rhs) noexcept {
        return !(lhs < rhs);
    }

private:
    size_type size_;
    char storage_[Capacity + 1];  // +1 for null terminator
};

}  // namespace bluestl
