#include <iostream>

class C;

struct S {
    int x = 1;
    double d = 3.14; 

    void f(int y);
    void ff(int x){
        this->x;
    }

    struct SS {
        char c;
    };
};

void S::f(int y) {
    std::cout << x + y;
    ff();
}

int main() {
    S s{2, 4.5}; // Aggregate initialization
    S* p = &s;
    p->x = 3;
    s.f(5);
}
