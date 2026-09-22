#include <iostream>

struct A {
    A(int x) {
        std::cout << "A";
        if (x == 0) throw 1;
    }
    ~A() {
        std::cout << "~A";
    }
};

struct S {
    S(int) {
        std::cout << "S";
    }
    ~S() noexcept(false) {
        std::cout << "~S";
        std::cout << std::uncaught_exception();
        if (std::uncaught_exception()) {
            throw 1;
        }
    }
};

int main() {
    try {
        S s(0);
        throw 1;
    } catch (...) {

    }
}
