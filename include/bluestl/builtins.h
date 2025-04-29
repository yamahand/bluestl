#pragma once

#define bluestl_is_empty(T) __is_empty(T)

#define bluestl_is_union(T) __is_union(T)

#define bluestl_is_class(T) __is_class(T)

#define bluestl_is_enum(T) __is_enum(T)

#define bluestl_is_abstract(T) __is_abstract(T)

#define bluestl_is_polymorphic(T) __is_polymorphic(T)

#define bluestl_is_base_of(Base, Derived) __is_base_of(Base, Derived)

#define bluestl_has_unique_object_representations(T) __has_unique_object_representations(T)

#if defined(_MSC_VER)
#define bluestl_is_convertible(From, To) __is_convertible_to(From, To)
#else
#define bluestl_is_convertible(From, To) __is_convertible(From, To)
#endif

#if __has_builtin(__is_trivially_destructible) || defined(_MSC_VER)
#define bluestl_is_trivially_constructible(T) __is_trivially_constructible(T)
#define bluestl_is_trivially_destructible(T) __is_trivially_destructible(T)
#else
#define bluestl_is_trivially_constructible(T) __has_trivial_constructor(T)
#define bluestl_is_trivially_destructible(T) __has_trivial_destructor(T)
#endif

#define bluestl_is_trivially_copyable(T) __is_trivially_copyable(T)
#define bluestl_is_final(T) __is_final(T)

#define bluestl_is_signed(T) (T(-1) < T(0))
#define bluestl_is_unsigned(T) (T(-1) > T(0))