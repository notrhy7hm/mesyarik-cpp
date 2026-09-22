#include <iostream>


struct Base {
    int x = 0;
    virtual void f() {}
    virtual ~Base() = default;
};

struct Derived: Base {
    int y = 0;
    void f() override {}
};

int main() {
    Derived d;
    Base& b = d;
    
    std::cout << typeid(b).name() << '\n';
    if(Derived* pd = dynamic_cast<Derived*>(&b); pd) {

    }
    if (int x = 1; x) {
    
    }
}
