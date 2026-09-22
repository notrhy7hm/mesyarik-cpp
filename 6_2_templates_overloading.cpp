#include <iostream>

template <typename U, typename T>
U f(T x) {
    std::cout << 1;
    return x;
}

void f(int x) {
    std::cout << 2;
}

int main() {
    int x = 0;
    int g = f<int>(x);
}
