#include <iostream>

struct Base {
    int x;
    virtual void f() = 0;
};
struct Derived: Base {

};

int main() {
    Derived d;
}
