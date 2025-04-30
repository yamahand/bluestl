// -----------------------------------------------------------------------------
// Bluestl allocator_traits.h
// C++20準拠・アロケータトレイト
// -----------------------------------------------------------------------------
/**
 * @file allocator_traits.h
 * @brief アロケータのインターフェースを統一するためのトレイトクラスを提供します。
 *
 * @details
 * `bluestl::allocator_traits` は、`std::allocator_traits` に似た機能を提供し、
 * コンテナ（例: `bluestl::vector`）が任意のアロケータ型 `Allocator` を
 * 統一的なインターフェースで扱えるようにします。
 *
 * 主な機能:
 * - アロケータに必要な型定義（`pointer`, `size_type` など）を推論または提供します。
 * - アロケータの操作（`allocate`, `deallocate`, `construct`, `destroy` など）を
 *   統一的な静的メンバ関数として提供します。アロケータ自身がこれらの関数を
 *   持たない場合は、適切なデフォルト実装を提供します。
 * - コンテナのコピー/ムーブ/スワップ時にアロケータインスタンスをどのように
 *   伝播させるかを示すフラグ（`propagate_on_container_...`）を提供します。
 * - `rebind` 機能により、異なる型に対応するアロケータ型を取得できます。
 *
 * このトレイトクラスを使用することで、コンテナの実装は特定のアロケータの詳細に
 * 依存せず、より汎用的になります。また、アロケータ開発者は、基本的なメモリ管理
 * 機能に集中でき、`allocator_traits` が提供するデフォルト実装を利用できます。
 */

 #pragma once

 #include <cstddef>      // size_t, ptrdiff_t
 #include <limits>       // numeric_limits
 #include <memory>       // pointer_traits, addressof
 #include <type_traits>  // various type traits, void_t, conditional_t, etc.
 #include <utility>      // forward, declval

 namespace bluestl {

 // --- ヘルパーメタ関数 (SFINAE / 型特性検出用) ---
 namespace detail {

 // 型 T にメンバ型 U が存在するかチェック
 template <typename T, typename = void> struct has_pointer_type : std::false_type {};
 template <typename T> struct has_pointer_type<T, std::void_t<typename T::pointer>> : std::true_type {};

 template <typename T, typename = void> struct has_const_pointer_type : std::false_type {};
 template <typename T> struct has_const_pointer_type<T, std::void_t<typename T::const_pointer>> : std::true_type {};

 template <typename T, typename = void> struct has_void_pointer_type : std::false_type {};
 template <typename T> struct has_void_pointer_type<T, std::void_t<typename T::void_pointer>> : std::true_type {};

 template <typename T, typename = void> struct has_const_void_pointer_type : std::false_type {};
 template <typename T> struct has_const_void_pointer_type<T, std::void_t<typename T::const_void_pointer>> : std::true_type {};

 template <typename T, typename = void> struct has_difference_type : std::false_type {};
 template <typename T> struct has_difference_type<T, std::void_t<typename T::difference_type>> : std::true_type {};

 template <typename T, typename = void> struct has_size_type : std::false_type {};
 template <typename T> struct has_size_type<T, std::void_t<typename T::size_type>> : std::true_type {};

 template <typename T, typename = void> struct has_propagate_on_container_copy_assignment : std::false_type {};
 template <typename T> struct has_propagate_on_container_copy_assignment<T, std::void_t<typename T::propagate_on_container_copy_assignment>> : std::true_type {};

 template <typename T, typename = void> struct has_propagate_on_container_move_assignment : std::false_type {};
 template <typename T> struct has_propagate_on_container_move_assignment<T, std::void_t<typename T::propagate_on_container_move_assignment>> : std::true_type {};

 template <typename T, typename = void> struct has_propagate_on_container_swap : std::false_type {};
 template <typename T> struct has_propagate_on_container_swap<T, std::void_t<typename T::propagate_on_container_swap>> : std::true_type {};

 template <typename T, typename = void> struct has_is_always_equal : std::false_type {};
 template <typename T> struct has_is_always_equal<T, std::void_t<typename T::is_always_equal>> : std::true_type {};

 // 型 Alloc に rebind<U>::other が存在するかチェック
 template <typename Alloc, typename U, typename = void> struct has_rebind : std::false_type {};
 template <typename Alloc, typename U> struct has_rebind<Alloc, U, std::void_t<typename Alloc::template rebind<U>::other>> : std::true_type {};

 // 型 Alloc に allocate(size_type, const_void_pointer) が存在するかチェック
 template <typename Alloc, typename SizeType, typename ConstVoidPtr, typename = void> struct has_allocate_hint : std::false_type {};
 template <typename Alloc, typename SizeType, typename ConstVoidPtr>
 struct has_allocate_hint<Alloc, SizeType, ConstVoidPtr, std::void_t<decltype(std::declval<Alloc&>().allocate(std::declval<SizeType>(), std::declval<ConstVoidPtr>()))>> : std::true_type {};

 // 型 Alloc に construct(T*, Args...) が存在するかチェック
 template <typename Alloc, typename T, typename... Args> using construct_call_t = decltype(std::declval<Alloc&>().construct(std::declval<T*>(), std::declval<Args>()...));
 template <typename Alloc, typename T, typename... Args> struct has_construct_impl {
     template <typename A, typename = construct_call_t<A, T, Args...>>
     static std::true_type test(int);
     template <typename A>
     static std::false_type test(...);
     using type = decltype(test<Alloc>(0));
 };
 template <typename Alloc, typename T, typename... Args> using has_construct = typename has_construct_impl<Alloc, T, Args...>::type;


 // 型 Alloc に destroy(T*) が存在するかチェック
 template <typename Alloc, typename T> using destroy_call_t = decltype(std::declval<Alloc&>().destroy(std::declval<T*>()));
 template <typename Alloc, typename T> struct has_destroy_impl {
     template <typename A, typename = destroy_call_t<A, T>>
     static std::true_type test(int);
     template <typename A>
     static std::false_type test(...);
     using type = decltype(test<Alloc>(0));
 };
 template <typename Alloc, typename T> using has_destroy = typename has_destroy_impl<Alloc, T>::type;


 // 型 Alloc に max_size() が存在するかチェック
 template <typename Alloc, typename = void> struct has_max_size : std::false_type {};
 template <typename Alloc> struct has_max_size<Alloc, std::void_t<decltype(std::declval<const Alloc&>().max_size())>> : std::true_type {};

 // 型 Alloc に select_on_container_copy_construction() が存在するかチェック
 template <typename Alloc, typename = void> struct has_select_on_container_copy_construction : std::false_type {};
 template <typename Alloc> struct has_select_on_container_copy_construction<Alloc, std::void_t<decltype(std::declval<const Alloc&>().select_on_container_copy_construction())>> : std::true_type {};

 // --- 型推論ヘルパー ---
 template <typename Alloc>
 using pointer_t = std::conditional_t<has_pointer_type<Alloc>::value, typename Alloc::pointer, typename Alloc::value_type*>;

 template <typename Alloc, typename Ptr = pointer_t<Alloc>>
 using const_pointer_t = std::conditional_t<has_const_pointer_type<Alloc>::value, typename Alloc::const_pointer, typename std::pointer_traits<Ptr>::template rebind<const typename Alloc::value_type>>;

 // void_pointer の取得: Alloc::void_pointer があればそれ、なければ void*
 template <typename Alloc, typename Ptr = pointer_t<Alloc>>
 using void_pointer_t = std::conditional_t<has_void_pointer_type<Alloc>::value, typename Alloc::void_pointer, void*>; // デフォルトを void* に変更

 // const_void_pointer の取得: Alloc::const_void_pointer があればそれ、なければ const void*
 template <typename Alloc, typename Ptr = pointer_t<Alloc>>
 using const_void_pointer_t = std::conditional_t<has_const_void_pointer_type<Alloc>::value, typename Alloc::const_void_pointer, const void*>; // デフォルトを const void* に変更

 template <typename Alloc, typename Ptr = pointer_t<Alloc>>
 using difference_type_t = std::conditional_t<has_difference_type<Alloc>::value, typename Alloc::difference_type, typename std::pointer_traits<Ptr>::difference_type>;

 template <typename Alloc, typename DiffType = difference_type_t<Alloc>>
 using size_type_t = std::conditional_t<has_size_type<Alloc>::value, typename Alloc::size_type, std::make_unsigned_t<DiffType>>;

 template <typename Alloc>
 using pocca_t = std::conditional_t<has_propagate_on_container_copy_assignment<Alloc>::value, typename Alloc::propagate_on_container_copy_assignment, std::false_type>;

 template <typename Alloc>
 using pocma_t = std::conditional_t<has_propagate_on_container_move_assignment<Alloc>::value, typename Alloc::propagate_on_container_move_assignment, std::false_type>;

 template <typename Alloc>
 using pocs_t = std::conditional_t<has_propagate_on_container_swap<Alloc>::value, typename Alloc::propagate_on_container_swap, std::false_type>;

 template <typename Alloc>
 using iae_t = std::conditional_t<has_is_always_equal<Alloc>::value, typename Alloc::is_always_equal, std::is_empty<Alloc>>;

 } // namespace detail

 /**
  * @class allocator_traits
  * @brief アロケータのインターフェースを統一し、カスタマイズを容易にするためのトレイトクラス。
  * @tparam Allocator アロケータ型
  *
  * `std::allocator_traits` に似た機能を提供します。
  */
 template <typename Allocator>
 struct allocator_traits {
     /** @brief アロケータ自身の型 */
     using allocator_type = Allocator;
     /** @brief アロケータが扱う要素の型 */
     using value_type = typename Allocator::value_type;

     // --- ポインタ型 ---
     // Alloc::pointer があればそれを使用、なければ value_type*
     using pointer = detail::pointer_t<Allocator>; // typename と ::type を削除
     // Alloc::const_pointer があればそれを使用、なければ pointer_traits<pointer>::rebind<const value_type>
     using const_pointer = detail::const_pointer_t<Allocator, pointer>; // typename と ::type を削除
     // Alloc::void_pointer があればそれを使用、なければ void* (単純化)
     using void_pointer = detail::void_pointer_t<Allocator, void*>; // typename と ::type を削除
     // Alloc::const_void_pointer があればそれを使用、なければ const void* (単純化)
     using const_void_pointer = detail::const_void_pointer_t<Allocator, const void*>; // typename と ::type を削除

     // --- サイズ型 ---
     // Alloc::difference_type があればそれを使用、なければ pointer_traits<pointer>::difference_type
     using difference_type = detail::difference_type_t<Allocator>; // typename と ::type を削除
     // Alloc::size_type があればそれを使用、なければ std::make_unsigned_t<difference_type>
     using size_type = detail::size_type_t<Allocator>; // typename と ::type を削除

     /** @brief コンテナのコピー代入時にアロケータを伝播させるかを示す型。Allocator::propagate_on_container_copy_assignment がなければ false_type */
     using propagate_on_container_copy_assignment = detail::pocca_t<Allocator>; // typename と ::type を削除
     /** @brief コンテナのムーブ代入時にアロケータを伝播させるかを示す型。Allocator::propagate_on_container_move_assignment がなければ false_type */
     using propagate_on_container_move_assignment = detail::pocma_t<Allocator>;
     /** @brief コンテナのswap時にアロケータを交換するかを示す型。Allocator::propagate_on_container_swap がなければ false_type */
     using propagate_on_container_swap = detail::pocs_t<Allocator>;

     /** @brief アロケータの全てのインスタンスが常に等しいかを示す型。Allocator::is_always_equal がなければ is_empty<Allocator> */
     using is_always_equal = detail::iae_t<Allocator>;

     /**
      * @struct rebind
      * @brief 異なる型 `U` のためのアロケータ型を提供するメタ関数。
      * @tparam U 新しい要素型
      */
     template <typename U>
     struct rebind {
         /** @brief 型 `U` のためのアロケータ型。Allocator::rebind<U>::other があればそれ、なければ Allocator<U, Args...> のような形式を推測（注: このデフォルト推測は限定的） */
         using other = std::conditional_t<detail::has_rebind<Allocator, U>::value,
                                          typename Allocator::template rebind<U>::other,
                                          /* ここはAllocatorのテンプレート引数に依存するため、完全なデフォルト提供は難しい。
                                             標準では Alloc<U> を試みるが、Bluestlではアロケータが単純なテンプレートでない可能性も考慮。
                                             一旦、rebind<U>::other の存在を要求する形にするのが安全か。
                                             もしくは、bluestl::allocator のような特定のケースのみデフォルトを提供。 */
                                          typename Allocator::template rebind<U>::other // 仮: rebindの存在を要求
                                          >;
     };

     /** @brief 型 `U` のためのアロケータ型を取得するエイリアス */
     template <typename U>
     using rebind_alloc = typename rebind<U>::other;

     /** @brief 型 `U` のためのアロケータに対応する `allocator_traits` を取得するエイリアス */
     template <typename U>
     using rebind_traits = allocator_traits<rebind_alloc<U>>;

     /**
      * @brief メモリを確保する
      * @param a 使用するアロケータ
      * @param n 確保する要素数
      * @return 確保されたメモリへのポインタ
      * @details `a.allocate(n)` を呼び出します。
      */
     [[nodiscard]] static pointer allocate(Allocator& a, size_type n) {
         return a.allocate(n);
     }

     /**
      * @brief ヒント付きでメモリを確保する (オプション)
      * @param a 使用するアロケータ
      * @param n 確保する要素数
      * @param hint 確保位置のヒントとなるポインタ
      * @return 確保されたメモリへのポインタ
      * @details `Allocator` が `allocate(n, hint)` を提供する場合はそれを呼び出し、
      *          提供しない場合は `allocate(n)` を呼び出します。
      */
     [[nodiscard]] static pointer allocate(Allocator& a, size_type n, const_void_pointer hint) {
         if constexpr (detail::has_allocate_hint<Allocator, size_type, const_void_pointer>::value) {
             return a.allocate(n, hint);
         } else {
             return a.allocate(n);
         }
     }

     /**
      * @brief メモリを解放する
      * @param a 使用するアロケータ
      * @param p 解放するメモリへのポインタ (`allocate` で取得したもの)
      * @param n 解放する要素数 (`allocate` に渡したものと同じ値)
      * @details `a.deallocate(p, n)` を呼び出します。
      */
     static void deallocate(Allocator& a, pointer p, size_type n) {
         // Bluestlでは例外を投げないのでnoexceptを付与できる可能性があるが、
         // アロケータの実装に依存するため、ここでは付けない。
         a.deallocate(p, n);
     }

     /**
      * @brief 指定されたメモリ位置にオブジェクトを構築する
      * @tparam T 構築するオブジェクトの型 (通常は `value_type`)
      * @tparam Args 構築に使用する引数の型
      * @param a 使用するアロケータ
      * @param p 構築先のメモリアドレス (`allocate` で取得した領域内)
      * @param args 構築に使用する引数
      * @details `Allocator` が `construct(p, args...)` を提供する場合はそれを呼び出し、
      *          提供しない場合は placement new `::new (static_cast<void*>(p)) T(std::forward<Args>(args)...)` を使用します。
      */
     template <typename T, typename... Args>
     static void construct(Allocator& a, T* p, Args&&... args) {
         // Bluestlでは例外を投げない前提
         if constexpr (detail::has_construct<Allocator, T, Args...>::value) { // .value を追加
             a.construct(p, std::forward<Args>(args)...);
         } else {
             // placement new は例外を投げない (std::nothrow を指定しなくても、メモリ確保がないため)
             ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
         }
     }

     /**
      * @brief 指定されたメモリ位置のオブジェクトを破棄する
      * @tparam T 破棄するオブジェクトの型 (通常は `value_type`)
      * @param a 使用するアロケータ
      * @param p 破棄するオブジェクトのアドレス
      * @details `Allocator` が `destroy(p)` を提供する場合はそれを呼び出し、
      *          提供しない場合はデストラクタ `p->~T()` を直接呼び出します。
      */
     template <typename T>
     static void destroy(Allocator& a, T* p) {
         // Bluestlでは例外を投げない前提
         if constexpr (detail::has_destroy<Allocator, T>::value) { // .value を追加
             a.destroy(p);
         } else {
             // デストラクタが noexcept であれば、この操作も noexcept
             p->~T();
         }
     }

     /**
      * @brief アロケータが確保できる最大の要素数を返す
      * @param a 使用するアロケータ
      * @return 最大要素数
      * @details `Allocator` が `max_size()` を提供する場合はそれを呼び出し、
      *          提供しない場合は `std::numeric_limits<size_type>::max() / sizeof(value_type)` を返します。
      */
     static size_type max_size(const Allocator& a) noexcept {
         if constexpr (detail::has_max_size<Allocator>::value) {
             return a.max_size();
         } else {
             // この計算はオーバーフローする可能性があるが、std::allocator_traits のデフォルトに従う
             // size_type の最大値を返す方が安全かもしれない
             // return std::numeric_limits<size_type>::max();
             return std::numeric_limits<size_type>::max() / sizeof(value_type);
         }
     }

     /**
      * @brief コンテナのコピー構築時に使用するアロケータを選択する
      * @param a コピー元コンテナのアロケータ
      * @return 新しいコンテナが使用すべきアロケータ
      * @details `Allocator` が `select_on_container_copy_construction()` を提供する場合はそれを呼び出し、
      *          提供しない場合は `a` を返します (コピー元のアロケータをそのまま使う)。
      */
     static Allocator select_on_container_copy_construction(const Allocator& a) {
         if constexpr (detail::has_select_on_container_copy_construction<Allocator>::value) {
             return a.select_on_container_copy_construction();
         } else {
             return a;
         }
     }
 };

 } // namespace bluestl
