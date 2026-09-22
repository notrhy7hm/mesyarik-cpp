#include <iostream>

class C {
private:
    void f(int) {
        std::cout << 1;
    }
public:
    void f(float) {
        std::cout << 2;
    }
};

int main() {
    C c;
    c.f(9);
    c.f(3.14);
}
