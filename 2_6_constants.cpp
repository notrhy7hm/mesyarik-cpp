#include <iostream>

int main() {
    const char x = 'a';
    char *p = nullptr;
    const char** q = &p;
    *q = &x;
    *p = 'b';
    std::cout << x;
}
