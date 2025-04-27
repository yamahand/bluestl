// -----------------------------------------------------------------------------
// Bluestl variant.h
// C++20準拠・STL風インターフェースの型安全なユニオン型
// -----------------------------------------------------------------------------
/// @file variant.h
/// @brief Bluestlプロジェクトのvariantクラスを提供します。
///
/// Bluestlは、高速なコンパイル・実行、固定サイズコンテナ、STLの代替/補完を目指すC++20用ライブラリです。
///
/// @details
/// variantは、複数の型のうち1つだけを安全に保持できる型安全なユニオン型です。
/// - RTTIなし、例外なし、ヒープ割り当てなし、header-only設計
/// - STL std::variantに似たインターフェースを持ちますが、Bluestlの設計方針に従い、
///   例外やRTTIを一切使用せず、最小限の依存で高速なデバッグ・ビルドを実現します。
///
/// 主な特徴:
///   - C++20準拠、STL std::variant風のAPI
///   - RTTI/例外/ヒープ割り当てなし
///   - header-only、#pragma onceによるインクルードガード
///   - index()で現在の型インデックス取得
///   - get_if<T>()で型Tの値へのポインタ取得
///   - emplace<T>(...)で型Tの値を再構築
///   - visit(Visitor)で値にアクセス
///   - valueless_by_exception()はSTLと同じく例外時のみtrue（Bluestlでは例外非対応のため通常はfalse）
///   - STL std::variantとの違い: 例外非対応、RTTI非使用、最小限の実装
///
/// Bluestl全体の設計方針:
///   - 高速なコンパイル・実行
///   - RTTI/例外/ヒープ割り当てなし
///   - header-only
///   - STLに似たインターフェース
///   - シンプルで明確なC++コード
///   - 分離・粒度の細かい設計
#pragma once

#include <type_traits>
#include <utility>
#include <cstddef>

#include "assert_handler.h"

namespace bluestl {
/**
 * @class variant
 * @brief 型安全なユニオン型。STL std::variant風インターフェース。
 * @tparam Types 保持する型のリスト
 *
 * - Types...のいずれか1つの値を保持
 * - RTTI/例外/ヒープ割り当てなし
 * - STL std::variant風インターフェース
 * - index()で現在の型インデックス取得
 * - get_if<T>()で型Tの値へのポインタ取得
 * - emplace<T>(...)で型Tの値を再構築
 * - visit(Visitor)で値にアクセス
 */
template <typename... Types>
class variant {
    static_assert(sizeof...(Types) > 0, "variant must have at least one alternative");

    
    // 最大サイズと最大アラインメントを計算
    static constexpr size_t max_size = std::max({ sizeof(Types)... });
    static constexpr size_t max_align = std::max({ alignof(Types)... });
    static constexpr size_t type_count = sizeof...(Types);

    template <typename T, typename... Rest>
    static constexpr size_t type_index_impl(size_t idx = 0) {
        if constexpr (sizeof...(Rest) == 0) {
            return idx;
        } else {
            return type_index_impl<Rest...>(idx + 1);
        }
    }

    template <typename T, size_t I = 0>
    static constexpr size_t type_index() {
        return type_index_impl<Types...>();
    }

    template <typename T, typename... Rest>
    static constexpr size_t find_type_index(size_t idx = 0) {
        if constexpr (sizeof...(Rest) == 0) {
            return idx;
        } else if constexpr (std::is_same_v<T, T>) {
            return idx;
        } else {
            return find_type_index<T, Rest...>(idx + 1);
        }
    }


public:
    /**
     * @brief デフォルトコンストラクタ。値を保持しない（valueless_by_exception()がtrue）。
     */
    variant() noexcept : index_(npos) {}

    /**
     * @brief 型Tの値でvariantを構築。
     * @tparam T 構築する型
     * @param value 値
     */
    template <typename T, typename = std::enable_if_t<(std::is_same_v<T, Types> || ...)>>
    variant(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        emplace<T>(value);
    }

    /**
     * @brief 型Tの右辺値でvariantを構築。
     * @tparam T 構築する型
     * @param value 値
     */
    template <typename T, typename = std::enable_if_t<(std::is_same_v<T, Types> || ...)>>
    variant(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        emplace<T>(std::move(value));
    }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元variant
     */
    variant(const variant &other) {
        // otherがvaluelessなら自分もvaluelessに
        if (other.valueless_by_exception())
        {
            reset();
            return;
        }
        // 通常のコピー処理
        emplace_by_index(other.index(), other);
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元variant
     */
	variant(variant&& other) noexcept((std::is_nothrow_move_constructible_v<Types> && ...)) : index_(npos) {
		if (!other.valueless_by_exception()) {
            other.visit([this](auto&& v) { this->emplace<std::decay_t<decltype(v)>>(std::move(v)); });
            other.reset(); // ムーブ後の状態をリセット
        }
        else {
            index_ = npos;
        }
	}

    /**
     * @brief デストラクタ
     */
    ~variant() { destroy(); }

    /**
     * @brief コピー代入演算子
     * @param other コピー元variant
     * @return *this
     */
    variant& operator=(const variant& other) {
        if (this == &other) return *this;
        if (other.valueless_by_exception()) {
            reset();
            return *this;
        }
        emplace_by_index(other.index(), other);
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元variant
     * @return *this
     */
    variant& operator=(variant&& other) noexcept((std::is_nothrow_move_assignable_v<Types> && ...)) {
        if (this != &other) {
            if (other.valueless_by_exception()) {
                reset();
            } else {
                other.visit([this](auto&& v) { this->emplace<std::decay_t<decltype(v)>>(std::move(v)); });
            }
        }
        return *this;
    }

    /**
     * @brief 型Tの値をvariantに代入。
     * @tparam T 代入する型
     * @param value 値
     * @return *this
     */
    template <typename T>
    variant& operator=(T&& value) noexcept(std::is_nothrow_assignable_v<T&, T&&>&& std::is_nothrow_constructible_v<T, T&&>) {
        static_assert((std::is_same_v<T, Types> || ...), "T must be one of the variant's types");
        emplace<std::decay_t<T>>(std::forward<T>(value)); // 新しい値を構築
        return *this;
    }

    /**
     * @brief 型Tの値を新たに構築し、variantに格納。
     * @tparam T 構築する型
     * @tparam Args コンストラクタ引数型
     * @param args コンストラクタ引数
     * @return 構築された値への参照
     */
    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        static_assert((std::is_same_v<T, Types> || ...), "T must be one of Types...");
        destroy();
        new (&storage_) T(std::forward<Args>(args)...);
        index_ = index_of<T>();
        return *reinterpret_cast<T*>(&storage_);
    }

    /**
     * @brief variantをvalueless状態にする。
     */
    void reset() noexcept {
        destroy();
        index_ = npos;
    }

    /**
     * @brief 値を保持していない場合true。
     * @return 値がなければtrue
     */
    bool valueless_by_exception() const noexcept { return index_ == npos; }
    /**
     * @brief 現在保持している型のインデックス（0始まり）を返す。
     * @return 型インデックス
     */
    size_t index() const noexcept { return index_; }

    /**
     * @brief 現在の値が型Tであればtrue。
     * @tparam T 判定する型
     * @return 型が一致すればtrue
     */
    template <typename T>
    bool holds_alternative() const noexcept {
        return index_ == index_of<T>();
    }

    /**
     * @brief 型Tの値へのポインタを返す。型が一致しない場合はnullptr。
     * @tparam T 取得する型
     * @return 値へのポインタ
     */
    template <typename T>
    T* get_if() noexcept {
        return holds_alternative<T>() ? reinterpret_cast<T*>(&storage_) : nullptr;
    }
    template <typename T>
    const T* get_if() const noexcept {
        return holds_alternative<T>() ? reinterpret_cast<const T*>(&storage_) : nullptr;
    }

    /**
     * @brief 保持している値にVisitorを適用。
     * @tparam Visitor ビジター型
     * @param vis ビジター
     * @return ビジターの戻り値
     */
    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) {
        if (valueless_by_exception()) {
            // 未定義動作: 何もしない
            return;
        }
        return visit_impl(*this, std::forward<Visitor>(vis));
    }
    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) const {
        if (valueless_by_exception()) {
            // 未定義動作: 何もしない
            return;
        }
        return visit_impl(*this, std::forward<Visitor>(vis));
    }

    /**
     * @brief 型TがTypes...の何番目か（0始まり）を返す。
     * @tparam T 判定する型
     * @return インデックス
     */
    template <typename T>
    static constexpr size_t index_of() {
        size_t idx = 0;
        size_t index = 0;
        bool found = false;
        ((std::is_same_v<T, Types> ? (found = true, index = idx) : ++idx), ...);
        return found ? index : static_cast<size_t>(-1); // 見つからない場合はエラー
    }

private:
    static constexpr size_t npos = static_cast<size_t>(-1);
    alignas(max_align) std::byte storage_[max_size];
    size_t index_ = npos;

    void destroy() noexcept {
        if (valueless_by_exception() || index_ == npos) return;
        destroy_impl(index_);
    }

    void destroy_impl(size_t idx) noexcept {
        using destroy_fn_t = void(*)(void*);
        static constexpr destroy_fn_t destroyers[] = { &destroy_one<Types>... };
        destroyers[idx](&storage_);
    }
    template <typename T>
    static void destroy_one(void* ptr) noexcept {
        reinterpret_cast<T*>(ptr)->~T();
    }

    template <typename Self, typename Visitor>
    static decltype(auto) visit_impl(Self& self, Visitor&& vis) {
        if constexpr (sizeof...(Types) == 0) {
            // 到達不能
        }
        size_t idx = self.index_;
        return visit_switch(self, std::forward<Visitor>(vis), idx);
    }
    template <typename Self, typename Visitor>
    static decltype(auto) visit_switch(Self& self, Visitor&& vis, size_t idx) {
        // 型ごとにif文で分岐
        size_t i = 0;
        return visit_switch_impl<Self, Visitor, Types...>(self, std::forward<Visitor>(vis), idx, i);
    }
    template <typename Self, typename Visitor>
    static decltype(auto) visit_switch_impl(Self&, Visitor&&, size_t, size_t) {
        // 到達不能
        throw;
    }
    template <typename Self, typename Visitor, typename T, typename... Rest>
    static decltype(auto) visit_switch_impl(Self& self, Visitor&& vis, size_t idx, size_t i) {
        if (idx == i) {
            if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
                return std::forward<Visitor>(vis)(*reinterpret_cast<const T*>(&self.storage_));
            } else {
                return std::forward<Visitor>(vis)(*reinterpret_cast<T*>(&self.storage_));
            }
        } else {
            return visit_switch_impl<Self, Visitor, Rest...>(self, std::forward<Visitor>(vis), idx, i + 1);
        }
    }

    // emplace_by_indexの実装（テンプレート再帰）
    template <size_t I = 0>
    void emplace_by_index(size_t idx, const variant& other) {
        if constexpr (I < sizeof...(Types)) {
            if (idx == I) {
                using T = std::tuple_element_t<I, std::tuple<Types...>>;
                const T* ptr = other.template get_if<T>();
                BLUESTL_ASSERT(ptr != nullptr);
                emplace<T>(*ptr);
            } else {
                emplace_by_index<I + 1>(idx, other);
            }
        } else {
            BLUESTL_ASSERT(false); // 不正なインデックス
        }
    }
};
} // namespace bluestl
