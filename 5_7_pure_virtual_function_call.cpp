#include <iostream>

struct Base {
    virtual void h() = 0;
    void f() {
        std::cout << "f";
        h();
    }
    Base() {
        std::cout << "Base";
        //h();
        f();
    }
    virtual ~Base() = default;
};

struct Derived: Base {
    void g() {
        f();
    }
    void h() override {
        std::cout << "h";
    }
    Derived() {
        std::cout << "Derived";
    }
};

int main() {
    Derived d;
}
