#include <iostream>

class C {
private:
    class Inner {
    public:
        int x = 1;
    private:
        int y = 2;
    };
public:
    Inner f() {
        return Inner();
    }
};

int main() {
    C c;
    std::cout << c.f().x;
}
