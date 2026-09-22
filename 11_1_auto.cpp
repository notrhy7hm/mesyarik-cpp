#include <iostream>

void f(auto... x) {
    
}

template <typename T>
auto g(T x) {
    if constexpr(std::is_same_v<T, int>)
        return x;
    else
        return 1u;
}

template <auto N>
struct Example {};

template <typename T>
void h(T) {}

int main() {
    h({1, 2, 3});
    auto lst = {1, 2, 3};

    Example<2> ei;
    Example<'a'> ec;
    
    f(2);
    // auto&& x = g(1);
    auto x = 5;
    g(x);
    g(1.0);
    {
        auto x = 5;
        auto& y = x;
        const auto& z = y;
        auto&& t = std::move(x);
        int* p = new auto(5);
    }
}
