#include <iostream>

struct S {
    int x;
    double y;

    void f(int z) {
        std::cout << x + z;
    }
};

int main() {
    int S::* p = &S::x;
    S s{1, 3.14};
    s.*p = 2;

    S* ps = &s;
    std::cout << ps->*p;

    void (S::* pf)(int) = &S::f;
    (s.*pf)(3);
    (ps->*pf)(5);
}
