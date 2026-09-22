#include <iostream>

template <typename T>
struct Base {
    int x = 0;
};

template <typename T>
struct Derived: Base<T> {
    void f() {
        ++Base<T>::x;
    }
};

int main() {}
