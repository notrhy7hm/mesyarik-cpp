#include <iostream>

struct A {
    A(int x) { std::cout << "A" << x; }
    ~A() { std::cout << "~A"; }
};

struct Base { 
    A x;
    Base(int x): x(x) { std::cout << "Base"; }
    Base(const Base& other): x(other.x) { std::cout << "Copy"; }
};

struct Derived: Base {
    int y = 0;
    using Base::Base;
    Derived(int y): Base(0), y(y) {}

};

int main() {
    Derived d = 1;
    std::cout << '\n';

    Derived d2 = d;
    std::cout << d2.y;
}

