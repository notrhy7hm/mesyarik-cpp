#include <iostream>

struct A {
    virtual int f();
};

struct B : A {
    int f() override { new (this) A; return 1; }
};

int A::f() { new (this) B; return 2; }

int h() {
    A a;
    int n = a.f();
    int m = std::launder(&a)->f();
    return n + m;
}

int main() {
    std::cout << h() << '\n';
}
