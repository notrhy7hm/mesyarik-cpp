#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>
#include <cstddef>
#include <compare>
#include <functional>

template <typename... Types>
struct tuple;


template <std::size_t N, typename... Types>
decltype(auto) get(tuple<Types...>& t);

template <std::size_t N, typename... Types>
decltype(auto) get(const tuple<Types...>& t);

template <std::size_t N, typename... Types>
decltype(auto) get(tuple<Types...>&& t);

template <std::size_t N, typename... Types>
decltype(auto) get(const tuple<Types...>&& t);


template <typename T>
struct tuple_size;

template <std::size_t N, typename T>
struct tuple_element;






template <>
struct tuple<> {
    tuple() = default;
    tuple(const tuple&) = default;
    tuple(tuple&&) = default;

    tuple& operator=(const tuple&) = default;
    tuple& operator=(tuple&&) = default;
    void swap(tuple&) noexcept {}
};






template <typename Head ,typename... Tail>
class tuple<Head, Tail...> {
private:
    Head head;
    tuple<Tail...> tail;
    
    template <typename...>
    friend struct tuple;

    template <std::size_t N, typename... Types>
    friend decltype(auto) get(tuple<Types...>&);
    
    template <std::size_t N, typename... Types>
    friend decltype(auto) get(const tuple<Types...>&);

    template <std::size_t N, typename... Types>
    friend decltype(auto) get (tuple<Types...>&&);
    
    template <std::size_t N, typename... Types>
    friend decltype(auto) get(const tuple<Types...>&&);

public:
    tuple() = default;

    template<typename H, typename... Ts>
    tuple(H&& h, Ts&&... ts_) 
        : head(std::forward<H>(h)),
          tail(std::forward<Ts>(ts_)...) {}

    tuple(const tuple&) = default;
    tuple(tuple&&) = default;


    template <typename OtherHead, typename... OtherTail>
    tuple(const tuple<OtherHead, OtherTail...>& other)
        : head(other.head),
          tail(other.tail) {}

    template <typename OtherHead, typename... OtherTail>
    tuple(tuple<OtherHead, OtherTail...>&& other)
        : head(std::forward<OtherHead>(other.head)),
          tail(std::move(other.tail)) {}

    tuple& operator=(const tuple&) = default;
    tuple& operator=(tuple&&) = default;

    template <typename OtherHead, typename... OtherTail>
    tuple& operator=(const tuple<OtherHead, OtherTail...>& other) {
        head = other.head;
        tail = other.tail;
        return *this;
    }
    
    template <typename OtherHead, typename... OtherTail>
    tuple& operator=(tuple<OtherHead, OtherTail...>&& other) {
        head = std::forward<OtherHead>(other.head);
        tail = std::move(other.tail);
        return *this;
    }

    void swap(tuple& other)
        noexcept(noexcept(std::swap(head, other.head)) &&
                noexcept(tail.swap(other.tail))) {
        
        using std::swap;
        
        swap(head, other.head);
        tail.swap(other.tail);
    }

};







template <size_t N, typename... Types>
decltype(auto) get(tuple<Types...>& t) {
    if constexpr (N == 0) {
        return (t.head);
    } else {
        return get<N-1>(t.tail);
    }
}

template <size_t N, typename... Types>
decltype(auto) get(const tuple<Types...>& t) {
    if constexpr (N == 0) {
        return (t.head);
    } else {
        return get<N-1>(t.tail);
    }
}

template <size_t N, typename... Types>
decltype(auto) get(tuple<Types...>&& t) {
    if constexpr (N == 0) {
        using H = decltype(t.head);
        return static_cast<std::add_rvalue_reference_t<H>>(t.head);
    } else {
        return get<N - 1>(std::move(t.tail));
    }
}

template <size_t N, typename... Types>
decltype(auto) get(const tuple<Types...>&& t) {
    if constexpr (N == 0) {
        using H = decltype(t.head);
        using ConstH = std::conditional_t<std::is_reference_v<H>, H, std::add_const_t<H>>;

        return static_cast<std::add_rvalue_reference_t<ConstH>>(t.head);
    } else {
        return get<N-1>(std::move(t.tail));
    }
}




template <>
struct tuple_size<tuple<>>
    : std::integral_constant<std::size_t, 0> {};

template <typename Head, typename... Tail>
struct tuple_size<tuple<Head, Tail...>>
    : std::integral_constant<std::size_t, 1+ tuple_size<tuple<Tail...>>::value> {};

template <typename T>
struct tuple_size<const T> : tuple_size<T> {};

template <typename T>
struct tuple_size<volatile T> : tuple_size<T> {};

template <typename T>
struct tuple_size<const volatile T> : tuple_size<T> {};

template <typename T>
inline constexpr std::size_t tuple_size_v = tuple_size<T>::value;






template <typename Head, typename... Tail>
struct tuple_element<0, tuple<Head, Tail...>> {
    using type = Head;
};

template <std::size_t N, typename Head, typename... Tail>
struct tuple_element<N, tuple<Head, Tail...>> {
    using type = typename tuple_element<N-1, tuple<Tail...>>::type;
};

template <std::size_t N, typename T>
struct tuple_element<N, const T> {
    using type = std::add_const_t<typename tuple_element<N, T>::type>;
};

template <std::size_t N, typename T>
struct tuple_element<N, volatile T> {
    using type = std::add_volatile_t<typename tuple_element<N, T>::type>;
};

template <std::size_t N, typename T>
struct tuple_element<N, const volatile T> {
    using type = std::add_cv_t<typename tuple_element<N, T>::type>;
};

template <std::size_t N, typename T>
using tuple_element_t = typename tuple_element<N, T>::type;




template <typename... Ts>
auto make_tuple(Ts&&... args) {
    return tuple<std::unwrap_ref_decay_t<Ts>...>(std::forward<Ts>(args)...);
}





template <typename... Ts>
auto tie(Ts&... args) {
    return tuple<Ts&...>(args...);
}





template <typename... Ts>
auto forward_as_tuple(Ts&&... args) {
    return tuple<Ts&&...>(std::forward<Ts>(args)...);
}






struct ignore_t {
    template <typename T>
    constexpr const ignore_t& operator=(T&&) const noexcept {
        return *this;
    }
};

inline constexpr ignore_t ignore{};





template <typename... Ts, typename... Us, std::size_t... I>
bool tuple_equal_impl(
    const tuple<Ts...>& lhs,
    const tuple<Us...>& rhs,
    std::index_sequence<I...>) {
    
    return ((get<I>(lhs) == get<I>(rhs)) && ...);
}

template <typename... Ts, typename... Us>
bool operator==(
    const tuple<Ts...>& lhs,
    const tuple<Us...>& rhs
    ) {
    return tuple_equal_impl(lhs, rhs, std::index_sequence_for<Ts...>{});
}

template <typename... Ts, typename... Us>
bool operator!=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    return !(lhs == rhs);
}

template <std::size_t I = 0, typename... Ts, typename... Us>
bool tuple_less_impl(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    if constexpr (I == sizeof...(Ts)) {
        return false;
    } else {
        if (get<I>(lhs) < get<I>(rhs)) {
            return true;
        }

        if (get<I>(rhs) < get<I>(lhs)) {
            return false;
        }

        return tuple_less_impl<I+1>(lhs, rhs);
    }
}

template <typename... Ts, typename... Us>
bool operator<(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    return tuple_less_impl(lhs, rhs);
}

template <typename... Ts, typename... Us>
bool operator<=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    return !(rhs < lhs);
}

template <typename... Ts, typename... Us>
bool operator>(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    return rhs < lhs;
}

template <typename... Ts, typename... Us>
bool operator>=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
    return !(lhs < rhs);
}





template <typename... Ts>
void swap(tuple<Ts...>& lhs, tuple<Ts...>& rhs)
    noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}





struct tuple_cat_tag {};

template <typename... Ts, typename... Us, std::size_t... I, std::size_t... J>
auto tuple_cat_impl(tuple<Ts...>& t, tuple<Us...>& u,
        std::index_sequence<I...>, std::index_sequence<J...>) {
    return tuple<Ts..., Us...>(get<I>(t)..., get<J>(u)...);
}

template <typename... Us>
auto tuple_cat(tuple<>, tuple<Us...>& u) {
    return u;
}

template <typename... Ts, typename... Us>
auto tuple_cat(tuple<Ts...>& t, tuple<Us...>& u) {
    return tuple_cat_impl(t, u, std::index_sequence_for<Ts...>{},
                                std::index_sequence_for<Us...>{});
}






template <typename... Ts>
tuple(Ts&&...) -> tuple<std::decay_t<Ts>...>;




int main() {
    tuple<int, std::string, double> t(42, "hello", 3.14);

    std::cout << get<0>(t) << '\n';
    std::cout << get<1>(t) << '\n';
    std::cout << get<2>(t) << '\n';

    get<0>(t) = 100;
    std::cout << get<0>(t) << '\n';

    const auto& ct = t;
    std::cout << get<1>(ct) << '\n';


    static_assert(tuple_size_v<tuple<int, double, char>> == 3);

    static_assert(std::is_same_v<tuple_element_t<0, tuple<int, double>>, int>);

    static_assert(std::is_same_v<tuple_element_t<1, tuple<int, double>>, double>);


    int x = 10;

    auto mt = ::make_tuple(
        x,
        std::string("abc"),
        3.14
    );

    static_assert(
        std::is_same_v<
            decltype(mt),
            tuple<int, std::string, double>
        >
    );


    int a = 1;
    std::string b = "hello";

    auto tied = ::tie(a, b);

    get<0>(tied) = 100;
    std::cout << a << '\n'; // 100


    std::string s = "hello";

    tuple<std::string> copy(s);
    tuple<std::string> moved(std::move(s));

    std::cout << get<0>(copy) << '\n';
    std::cout << get<0>(moved) << '\n';


    tuple<int, int> c1(1, 2);
    tuple<int, int> c2(1, 3);

    std::cout << (c1 == c2) << '\n'; // 0
    std::cout << (c1 < c2)  << '\n'; // 1


    tuple<int, std::string> s1(1, "A");
    tuple<int, std::string> s2(2, "B");

    swap(s1, s2);

    std::cout << get<0>(s1) << ' ' << get<1>(s1) << '\n'; // 2 B
    std::cout << get<0>(s2) << ' ' << get<1>(s2) << '\n'; // 1 A


    tuple<int, std::string> p1(1, "abc");
    tuple<double, char> p2(3.14, 'x');

    auto cat = tuple_cat(p1, p2);

    static_assert(
        std::is_same_v<
            decltype(cat),
            tuple<int, std::string, double, char>
        >
    );

    std::cout
        << get<0>(cat) << ' '
        << get<1>(cat) << ' '
        << get<2>(cat) << ' '
        << get<3>(cat) << '\n';

    return 0;
}

