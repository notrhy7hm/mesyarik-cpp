#include <iostream>
#include <string>

template <class ...T> struct my_common_type;

template <typename... T>
using my_common_type_t = typename my_common_type<T...>::type;

template <class T>
struct my_common_type<T> {
    using type = std::decay_t<T>;
};

template <class T, class U>
struct my_common_type<T, U> {
    using type = std::decay_t<decltype(true ? std::declval<T>() : std::declval<U>())>;
};

template <class T, class U, class... V>
struct my_common_type<T, U, V...> {
    using type = my_common_type_t<my_common_type_t<T, U>, V...>;
};

template <typename T>
void foo(typename my_common_type<T, int>::type y) {}

void foo(...) {
}
/*
template <typename T>
void bar(T x, typename std::common_type<T, int>::type y) {}

void bar(...) {}
*/
int main() {
    foo(std::string("world"));
}
