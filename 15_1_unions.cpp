#include <iostream>

union U {
    const int x;
    int xx;
    double y;
    std::string s;

    U() {}
    ~U() {}
};

int main() {
    U u;
    u.x = 1;
}
