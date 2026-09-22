#include <iostream>

class C;

struct A {
    int x = 1;
    double d = 3.14;

    struct AA {
        char c;
    };
};

int main() {
    struct S {
        int x = 1;
        int y = 2;
    };

    S s;
}
