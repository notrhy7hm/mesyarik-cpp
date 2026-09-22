#include <iostream>

struct A {
    A() { std::cout << "A"; }
    A(const A&) { std::cout << "Copy" << std::endl; }
    ~A() { std::cout << "~A" << std::endl; }
};  

void f(int x) {
    A a;
    std::cout << &a << '\n';
    if (x == 0) {
        throw a;
    }
}

int main () {
    f(0);
}
