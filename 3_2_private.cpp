#include <iostream>


class C {
private: 
    int x{5};
public:
    void f(int y) {
        std::cout << x + y;
    }

    friend void g(C, int);
    friend class CC;
};

void g(C c, int y) {
    std::cout << c.x + y + 1;
}

int main() {
    C c;
    std::cout << (int&)c;
}
