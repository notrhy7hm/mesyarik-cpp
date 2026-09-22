#include <iostream>

template <typename T>
auto call_f(const T& t) {
    t.f();
}

template<typename T>
auto call_f_if_available_impl(const T& t, int) -> decltype(call_f(t)) {
    call_f(t);
}

template<typename T>
auto call_f_if_available_impl(const T& t, ...) {

}

template <typename T>
auto call_f_if_available(const T& t) {
    call_f_if_available_impl(t, 0);
}

struct WithoutF {};
struct WithF { void f() const { std::cout << "f() called\n"; }};

int main() {
    std::cout << "WithoutF\n";
    call_f_if_available(WithoutF{});
    std::cout << "WithF\n";
    call_f_if_available(WithF{});
}
