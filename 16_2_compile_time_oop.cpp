#include <iostream>
#include <array>

struct S {
    int x = 5;
    constexpr S(int x): x(x) {}

    int getInt() {
        return x;
    }
    constexpr int getInt() const {
        return x + 1;
    }
    constexpr ~S() {}
};

constexpr int f() {
    S s = 10;
    return static_cast<const S&>(s).getInt();
}

template <S s>
struct MyClass {
    std::array<int, s.x> a;
    int x = s.getInt();
};

struct Base {
    int x = 0;
    virtual int f() {
        return x;
    }
};

struct Derived: Base {
    constexpr int f() override {
        return x + 1;
    }
};  

constexpr void g(int x) {
    Derived d;
    Base& b = d;
    Base& bb = d;

    Base& b2 = x % 2 ? bb : b;
}

int h() {
    return 1;
}

template <int N, std::invocable<int> auto F>
struct Checker {
    static_assert(F(N));
    std::array<int, N> a;
};


int main() {
    // constexpr int y = g(0);
    // constexpr int z = g(1);
    // static_assert(y == 0);
    // static_assert(z == 1);

    // static_assert(f() == 10);
    
    // constexpr S s = 5;
    // static_assert(sizeof(MyClass<s>) == 24);
    // constexpr MyClass<s> c{};
    // static_assert(c.x == 6);

   constexpr Checker<37, [](int n) {
        for (int i = 2; i * i <= n; ++i) {
            if (n % i == 0) return false;
        }
        return n != 1;
    }> c{};


}
