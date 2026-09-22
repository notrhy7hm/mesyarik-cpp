#include <iostream>
#include <vector>

template <typename T>
struct vector {
    template <typename Iter>
        vector(Iter, Iter) {}
};

template <typename Iter>
vector(Iter, Iter)
    -> vector<typename std::iterator_traits<Iter>::value_type>;

struct A {};

struct B {};

template <typename... Args>
struct S: Args... {};

template <typename... Args>
S(Args...) -> S<Args...>;

int main() {

    S s{A(), B()};

    // std::vector v = {1, 2, 3, 4, 5};

    // std::vector v2(v.begin(), v.end());
}
