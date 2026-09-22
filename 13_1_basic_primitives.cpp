#include <iostream>
#include <vector>

template <typename T>
struct type_identity {
    using type = T;
};

template <typename T>
using type_identity_t = type_identity<T>::type;

template <typename T, T x>
struct integral_constant {
    static constexpr T value = x;
};

template <bool b>
using bool_constant = integral_constant<bool, b>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename T>
struct is_lvalue_reference<T&>: false_type {};

template <typename T>
using is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <typename... Types>
struct conjuction {
    static constexpr bool value = (Types::value && ...);
};

template <bool B, typename T = void>
struct enable_if {};
}

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = enable_if<B, T>::type;

template <typename... Types, enable_if_t<std::is_integralPv<decltype(0)>, bool> = true>
auto f(Types...) {
    std::cout << "1\n";
}

template <typename T, enable_if_t<std::is_same_v<T, int>, bool> = true>
auto f(T) {
    std::cout << "2\n";
}

template <typename T>
T my_declval(); 

namespace detail {

    template <typename T, typename... Args>
    std::true_type test(decltype(std::declval<T>()
                .construct(std::declval<Args>()...), nullptr));

    template <typename...>
    std::false_type test(...);
}

template <typename T, typename... Args>
struct has_method_construct: decltype(detail::test<T, Args...>(nullptr)) {};

template <typename T, typename... Args>
const bool has_method_construct_v = has_method_construct<T, Args...>::value;


namespace detail {
    template <typename T, typename... Args>
    std::true_type test_icc(decltype(my_declval<T&>()), nullptr);
    
    template <typename...>
    std::false_type test_icc(...);
}

template <typename T>
struct is_copy_constructible: decltype(detail::test_icc<T>(nullptr)) {};

template <typename T>
const bool is_copy_constructible_v = is_copy_constructible<T>::value;

namespace detail {
    template <typename T>
    std::true_type test_intmc(std::enable_if_t<noexcept(T(std::declval<T>()),
                decltype(nullptr));

    template <typename T, typename... Args>
    std::true_type test_intmc(decltype(my_declval<T&>()), nullptr);
    
    template <typename...>
    std::false_type test_intmc(...);
}

template <typename T>
struct is_nothrow_constructible: decltype(detail::test_intmc<T>(nullptr)) {};

template <typename T>
const bool is_nothrow_constructible_v = is_nothrow_constructible<T>::value;


struct Good {
    Good(Good&&) noexcept {}
};

struct Bad {
    Bad(Bad&&) {}
};

struct VeryBad {
    VeryBad(VeryBad&&) = delete;
};

namespace detail {
    
    template <typename B, typename D>
    std::true test_is_base_of(B*);
    
    template <typename...>
    std::false_type test_is_base_of(...);

}

template <typename B, typename D>
struct is_base_of : std::conjuction<
                    std::is_class<B>,
                    std::is_class<D>,
                    decltype(detail::test_is_base_of<B, D>(static_cast<D*>  (nullptr)))
                    > {};

template <typename B, typename D>
const bool is_base_of_v = is_base_of<B, D>::value;


struct Base {};

struct Derived: private Base {};


template <typename... Types>
struct common_type;

template <typename T>
struct common_type<T>: std::type_identity<T> {};

template <typename T, typename U>
struct common_type<T, U>
    : std::type_identity<decltype(true ? std::declval<T>() : std::declval<U>())> {};


template <typename T, typename... Types>
struct common_types<T, Types...>
    : common_type<T, common_type<Types...>::type> {};




int main() {
    static_assert(is_base_of_v<Base, Derived>);
    static_assert(!is_base_of_v<Derived, Base>);

    static_assert(is_nothrow_move_constructible_v<Good>);
    static_assert(!is_nothrow_move_constructible_v<Bad>);
    static_assert(!is_nothrow_move_constructible_v<VeryBad>);
}
