// -----------------------------------------------------------------------------
// Bluestl optional.h
// C++20準拠・STL風インターフェースの値の有無を表す型（optional）
// -----------------------------------------------------------------------------
/// @file optional.h
/// @brief Bluestlプロジェクトのoptionalクラスを提供します。
///
/// Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
///
/// @details
/// optionalは、値が存在するかどうかを明示的に表現できる型です。
/// - RTTIなし、例外なし、ヒープ割り当てなし、header-only設計
/// - STL std::optionalに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
///   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
///
/// 主な特徴:
///   - C++20準拠、STL std::optional風のAPI
///   - RTTI/例外/ヒープ割り当てなし
///   - header-only、#pragma onceによるインクルードガード
///   - has_value()で値の有無を判定
///   - value()/operator*()/operator->()で値へアクセス
///   - emplace()/reset()で値の構築・破棄
///   - STL std::optionalとの違い: 例外非対応、RTTI非使用、最小限の実装
///
/// Bluestl全体の設計方針:
///   - 高速なコンパイル・実行
///   - RTTI/例外/ヒープ割り当てなし
///   - header-only
///   - STLに似たインターフェース
///   - シンプルで明確なC++コード
///   - 分離・粒度の細かい設計
#pragma once

#include <utility>
#include <type_traits>

#include "assert_handler.h"

namespace bluestl {

/**
 * @brief nullopt_t 型の定義
 */
struct nullopt_t {
    explicit constexpr nullopt_t() = default;
};

/**
 * @brief nullopt 定数
 */
inline constexpr nullopt_t nullopt{};

/**
 * @class optional
 * @brief 値の有無を表す型。STL std::optional風インターフェース。
 * @tparam T 保持する値の型
 *
 * - T型の値を「ある/なし」で保持
 * - RTTI/例外/ヒープ割り当てなし
 * - STL std::optional風インターフェース
 * - has_value()で値の有無を判定
 * - value()/operator*()/operator->()で値へアクセス
 * - emplace()/reset()で値の構築・破棄
 */
template <typename T>
class optional {
   public:
    /**
     * @brief デフォルトコンストラクタ。値を保持しない状態で初期化。
     */
    constexpr optional() noexcept : has_value_(false) {}

    /**
     * @brief nullopt_t からのコンストラクタ。値を保持しない状態で初期化。
     */
    constexpr optional(nullopt_t) noexcept : has_value_(false) {}

    /**
     * @brief コピーコンストラクタ。値をコピーしてoptionalを構築。
     * @param value コピー元の値
     */
    constexpr optional(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) : has_value_(true) {
        new (&storage_) T(value);
    }

    /**
     * @brief ムーブコンストラクタ。値をムーブしてoptionalを構築。
     * @param value ムーブ元の値
     */
    constexpr optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) : has_value_(true) {
        new (&storage_) T(std::move(value));
    }

    /**
     * @brief optionalのコピーコンストラクタ。他のoptionalの値をコピー。
     * @param other コピー元のoptional
     */
    constexpr optional(const optional& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(*other);
        }
    }

    /**
     * @brief optionalのムーブコンストラクタ。他のoptionalの値をムーブ。
     * @param other ムーブ元のoptional
     */
    constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(std::move(*other));
        }
    }

    /**
     * @brief デストラクタ。値があれば破棄。
     */
    constexpr ~optional() {
        reset();
    }

    /**
     * @brief コピー代入演算子。他のoptionalの値をコピー。
     * @param other コピー元のoptional
     * @return *this
     */
    constexpr optional& operator=(const optional& other) noexcept(std::is_nothrow_copy_assignable_v<T> &&
                                                                  std::is_nothrow_copy_constructible_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                *get() = *other;
            } else if (has_value_) {
                reset();
            } else if (other.has_value_) {
                emplace(*other);
            }
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子。他のoptionalの値をムーブ。
     * @param other ムーブ元のoptional
     * @return *this
     */
    constexpr optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable_v<T> &&
                                                             std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                *get() = std::move(*other);
            } else if (has_value_) {
                reset();
            } else if (other.has_value_) {
                emplace(std::move(*other));
            }
        }
        return *this;
    }

    /**
     * @brief nullopt_t からの代入演算子。値をリセット。
     */
    constexpr optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    /**
     * @brief emplace: 新しい値を構築し、optionalに格納。以前の値は破棄される。
     * @tparam Args コンストラクタ引数型
     * @param args コンストラクタ引数
     * @return 構築された値への参照
     */
    template <typename... Args>
    constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        reset();
        new (&storage_) T(std::forward<Args>(args)...);
        has_value_ = true;
        return **this;
    }

    /**
     * @brief reset: 値を破棄し、値なし状態にする。
     */
    constexpr void reset() noexcept {
        if (has_value_) {
            get()->~T();
            has_value_ = false;
        }
    }

    /**
     * @brief operator->: 値へのポインタアクセス。
     * @return 値へのポインタ
     */
    constexpr T* operator->() noexcept {
        return get();
    }
    constexpr const T* operator->() const noexcept {
        return get();
    }

    /**
     * @brief operator*: 値への参照アクセス。
     * @return 値への参照
     */
    constexpr T& operator*() noexcept {
        return *get();
    }
    constexpr const T& operator*() const noexcept {
        return *get();
    }

    /**
     * @brief operator bool: 値が存在する場合true。
     * @return 値が存在すればtrue
     */
    constexpr explicit operator bool() const noexcept {
        return has_value_;
    }
    constexpr bool has_value() const noexcept {
        return has_value_;
    }

    /**
     * @brief value: 値を返す。値がなければアサート。
     * @return 値への参照
     */
    constexpr T& value() {
        BLUESTL_ASSERT(has_value());
        return *get();
    }
    constexpr const T& value() const {
        BLUESTL_ASSERT(has_value());
        return *get();
    }

    /**
     * @brief value_or: 値があれば値を返し、なければデフォルト値を返す。
     * @param default_value デフォルト値
     * @return 値またはデフォルト値
     */
    template<typename U>
    constexpr T value_or(U&& default_value) const {
        return has_value() ? **this : static_cast<T>(std::forward<U>(default_value));
    }

   private:
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    storage_t storage_;  ///< 値のストレージ
    bool has_value_;     ///< 値の有無

    /**
     * @brief get: 値へのポインタを返す（内部用）。
     * @return 値へのポインタ
     */
    constexpr T* get() noexcept {
        return reinterpret_cast<T*>(&storage_);
    }
    constexpr const T* get() const noexcept {
        return reinterpret_cast<const T*>(&storage_);
    }
};

/**
 * @class optional<T&>
 * @brief 参照型T&用のoptional特殊化。値の有無をポインタで管理。
 * @tparam T 参照先の型
 */
template <typename T>
class optional<T&> {
   public:
    /**
     * @brief デフォルトコンストラクタ。値なし状態で初期化。
     */
    constexpr optional() noexcept : ptr_(nullptr) {}
    /**
     * @brief 参照からoptionalを構築。
     * @param ref 参照
     */
    constexpr optional(T& ref) noexcept : ptr_(&ref) {}
    /**
     * @brief コピーコンストラクタ。
     * @param other コピー元
     */
    constexpr optional(const optional& other) noexcept : ptr_(other.ptr_) {}
    /**
     * @brief has_value: 値が存在するか判定。
     * @return 値があればtrue
     */
    constexpr bool has_value() const noexcept {
        return ptr_ != nullptr;
    }
    /**
     * @brief operator bool: 値が存在すればtrue。
     */
    constexpr explicit operator bool() const noexcept {
        return has_value();
    }
    /**
     * @brief value: 値への参照を返す。
     * @return 値への参照
     */
    constexpr T& value() const noexcept {
        BLUESTL_ASSERT(ptr_);
        return *ptr_;
    }
    /**
     * @brief operator*: 値への参照。
     */
    constexpr T& operator*() const noexcept {
        return value();
    }
    /**
     * @brief operator->: 値へのポインタ。
     */
    constexpr T* operator->() const noexcept {
        BLUESTL_ASSERT(ptr_);
        return ptr_;
    }
    /**
     * @brief reset: 値なし状態にする。
     */
    void reset() noexcept {
        ptr_ = nullptr;
    }

   private:
    T* ptr_;
};

/**
 * @brief optionalの等価比較演算子。
 */
template <typename T>
constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    if (lhs.has_value() && rhs.has_value()) {
        return *lhs == *rhs;
    }
    return lhs.has_value() == rhs.has_value();
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const T& rhs) noexcept {
    return lhs.has_value() && *lhs == rhs;
}

template <typename T>
constexpr bool operator==(const T& lhs, const optional<T>& rhs) noexcept {
    return rhs.has_value() && lhs == *rhs;
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const T& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename T>
constexpr bool operator!=(const T& lhs, const optional<T>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename T>
constexpr bool operator<(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    if (!lhs.has_value() && !rhs.has_value()) return false;
    if (!lhs.has_value()) return true;
    if (!rhs.has_value()) return false;
    return *lhs < *rhs;
}

template <typename T>
constexpr bool operator<=(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    return !(rhs < lhs);
}

template <typename T>
constexpr bool operator>(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    return rhs < lhs;
}

template <typename T>
constexpr bool operator>=(const optional<T>& lhs, const optional<T>& rhs) noexcept {
    return !(lhs < rhs);
}

template <typename T>
constexpr bool operator<(const optional<T>& lhs, const T& rhs) noexcept {
    return !lhs.has_value() || *lhs < rhs;
}

template <typename T>
constexpr bool operator<(const T& lhs, const optional<T>& rhs) noexcept {
    return rhs.has_value() && lhs < *rhs;
}

template <typename T>
constexpr bool operator<=(const optional<T>& lhs, const T& rhs) noexcept {
    return !lhs.has_value() || *lhs <= rhs;
}

template <typename T>
constexpr bool operator<=(const T& lhs, const optional<T>& rhs) noexcept {
    return rhs.has_value() && lhs <= *rhs;
}

template <typename T>
constexpr bool operator>(const optional<T>& lhs, const T& rhs) noexcept {
    return lhs.has_value() && *lhs > rhs;
}

template <typename T>
constexpr bool operator>(const T& lhs, const optional<T>& rhs) noexcept {
    return !rhs.has_value() || lhs > *rhs;
}

template <typename T>
constexpr bool operator>=(const optional<T>& lhs, const T& rhs) noexcept {
    return lhs.has_value() && *lhs >= rhs;
}

template <typename T>
constexpr bool operator>=(const T& lhs, const optional<T>& rhs) noexcept {
    return !rhs.has_value() || lhs >= *rhs;
}

/**
 * @brief make_optional 関数
 */
template <typename T>
constexpr optional<std::decay_t<T>> make_optional(T&& value) {
    return optional<std::decay_t<T>>{std::forward<T>(value)};
}

template <typename T, typename... Args>
constexpr optional<T> make_optional(Args&&... args) {
    return optional<T>{T(std::forward<Args>(args)...)};
}

}  // namespace bluestl
