#include <iostream>

struct Granny {
    int x;
    void f() {}
};

struct Mom: private Granny {
    friend int main();
    int x;
};

struct Son: Mom {
    int x;
    void f(::Granny& g) {
        std::cout << g.x;
    }
};

int main() {
    Son s;
    s.Granny::x;
}
