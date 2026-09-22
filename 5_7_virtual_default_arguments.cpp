#include <iostream>

struct Base {
    virtual void f(int x = 1) {
        std::cout << "Base" << x;
    }
};

struct Derived: Base {
    void f(int x = 2) override {
        std::cout << "Derived" << x;
    }
};

int main() {
    Derived d;
    Base& b = d;
    b.f();
}
