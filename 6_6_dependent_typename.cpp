#include <iostream>
#include <array>

template<typename T>
struct S {
    template<int N>
        void foo(int) {}
};

template<typename T>
void bar(int x, int y) {
    S<T> s;
    s.template foo<5>(x + y);
}

int main() {
    bar<int>(2, 3);
}
