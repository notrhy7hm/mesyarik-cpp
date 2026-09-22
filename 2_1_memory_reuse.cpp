#include <iostream>

int main() {
    int a = 1;
    int *p = &a;
    {
        int b = 2;
        p = &b;
    }

    std::cout << p << '\n';
    std::cout << *p << '\n';

    int c = 3, d = 4, e = 5, f = 6;

    std::cout << &c << ' ' << &d << ' ' << &e << ' ' << &f << '\n';
    ++*p;

    std::cout << c << d << e << f << '\n';
    std::cout << *p << '\n';
}
