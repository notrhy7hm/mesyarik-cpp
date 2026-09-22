#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

// template <typename T, typename = std::enable_if_t<std::is_class_v<T>, bool> = true>

template <typename T>
requires std::is_class_v<T>
//requires std::is_vlass_v<T> && std::is_copy_constructible_v<T>
void f(const T& x) {
    std::cout << 1;
}


template <typename T>
requires std::is_integral_v<T>
void f(const T& x) {
    std::cout << 2;
}

template <typename T, size_t N>
requires (N <= 1'000'000)
struct array {
    T arr[N];
};

struct S {
   bool operator==(const S&) & = delete;
   bool operator==(const S&) && { return true; }
};


template <typename T>
requires requires(T a, T b) {
    std::move(a) == b;
}
void test() {}



int main() {

}
