#include <iostream>

struct S {
    int x = 0;
    S(int x) : x(x) {
        std::cout << "Created" << std::endl;
    }
    S(const S& s) : x(s.x) {
        std::cout << "Copy" << std::endl;
    }
    S(S&& s) : x(s.x) {
        std::cout << "Move" << std::endl;
    }
    ~S() {
        std::cout << "Destroyed" << std::endl;
    }
};

struct T {
    T(S) {}
};

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
    BigInteger sum = a;
    sum += b;
    return sum;
}

S f(S&& a) {
    return std::move(a);
}

int main() {
    S t(0);
    S s = f(std::move(t));
}
